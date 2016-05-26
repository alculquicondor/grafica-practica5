#include <iostream>
#include <fstream>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "scene.h"
#include "perspective_camera.h"
#include "data_buffer_3.h"
#include "data_buffer_2.h"
#include "uniform_color.h"


using namespace std;


PerspectiveCamera *camera;
float cam_dist = 15;
MvpProgram *program = nullptr;

class Center : public Drawable {
public:
    Center() : Drawable(nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr), position(0, 0, 0), angle_z(0) {}

    glm::vec3 position;
    float angle_z;
    virtual glm::mat4 get_model(float time) {
        return Drawable::get_model(time) *
                glm::rotate(-angle_z, glm::vec3(0, 1, 0)) * glm::translate(position);
    }

    void move_horizontal(float d) {
        position += glm::rotateY(glm::vec3(d, 0, 0), angle_z);
    }

    void move_vertical(float d) {
        position -= glm::rotateY(glm::vec3(0, 0, d), angle_z);
    }
};


Center *center;


void scroll(GLFWwindow* window, double x_offset, double y_offset) {
    center->angle_z -= x_offset * .02;
    cam_dist -= y_offset * .2;
    camera->look_at(glm::vec3(0, cam_dist *.6f, cam_dist), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

class Terrain : public Drawable {
public:
    Terrain(vector<float> heights, int size, Drawable *parent = nullptr) :
            Drawable(::program, parent), size(size), vertices(size * size), normals(size * size) {
        int p, q;
        std::vector<glm::vec2> uv(size * size);
        std::vector<glm::vec3> triangle_normal((std::size_t)((size - 1) * (size - 1) * 2));
        float duv = 1.f / (size - 1);
        p = 0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                uv[p] = {j * duv, (size - i - 1) * duv};
                vertices[p] = {j - size / 2, heights[p] / 25., i - size / 2};
                ++p;
            }
        }
        this->texture = new Texture("textures/fontvieille.tga", GL_BGR);
        this->geometry = new DataBuffer3(vertices, 0);
        this->color = new UniformColor({0, 0, 0, 1}, (GLsizei)(size * size));
        this->uv = new DataBuffer2(uv, 2);

        p = q = 0;
        std::vector<unsigned int> index((size - 1) * (size - 1) * 6u);
        for (int i = 0; i < size - 1; ++i) {
            for (int j = 0; j < size - 1; ++j) {
                index[p++] = (unsigned int)((i * size) + j + 1);
                index[p++] = (unsigned int)((i * size) + j);
                index[p++] = (unsigned int)(((i + 1) * size) + j);
                triangle_normal[q++] = glm::normalize(
                        glm::cross((vertices[index[p - 2]] - vertices[index[p - 3]]),
                                   (vertices[index[p - 1]] - vertices[index[p - 3]])));
                index[p++] = (unsigned int)((i * size) + j + 1);
                index[p++] = (unsigned int)(((i + 1) * size) + j);
                index[p++] = (unsigned int)(((i + 1) * size) + j + 1);
                triangle_normal[q++] = glm::normalize(
                        glm::cross((vertices[index[p - 2]] - vertices[index[p - 3]]),
                                   (vertices[index[p - 1]] - vertices[index[p - 3]])));
            }
        }
        this->index_buffer = new IndexBuffer(index);

        p = 0;
        for (int i = 0; i < size; ++i) {
            int row_first = i * (size - 1) * 2, prev_row_first = (i - 1) * (size - 1) * 2;
            if (i < size - 1)
                normals[p] += triangle_normal[row_first];
            if (i > 0)
                normals[p] += triangle_normal[prev_row_first] +
                        triangle_normal[prev_row_first + 1];
            normals[p] = glm::normalize(normals[p]);
            ++p;
            for (int j = 1; j < size - 1; ++j) {
                if (i < size - 1)
                    for (int k = 2 * j - 2; k <= 2 * j; ++k)
                        normals[p] += triangle_normal[row_first + k];
                if (i > 0)
                    for (int k = 2 * j - 1; k <= 2 * j + 1; ++k)
                        normals[p] += triangle_normal[prev_row_first + k];
                normals[p] = glm::normalize(normals[p]);
                ++p;
            }
            if (i < size - 1)
                normals[p] += triangle_normal[row_first + 2 * size - 4] +
                        triangle_normal[row_first + 2 * size - 3];
            if (i > 0)
                normals[p] += triangle_normal[prev_row_first + 2 * size - 3];
            normals[p] = glm::normalize(normals[p]);
            ++p;
        }
        this->normal = new DataBuffer3(normals, 3);
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    int size;
};


class Normals : public Drawable {
public:
    Normals(Terrain *terrain, Drawable *parent = nullptr) : Drawable(new MvpProgram(), parent) {
        this->draw_mode = GL_LINES;
        size = terrain->size * terrain->size * 2;
        std::vector<glm::vec3> v(size);
        this->color = new UniformColor(glm::vec4(1, 0, 0, 1), size);
        int p = 0;
        for (int i = 0; i < terrain->size; ++i)
            for (int j = 0; j < terrain->size; ++j) {
                v[p * 2] = terrain->vertices[p];
                v[p * 2 + 1] = terrain->vertices[p] + terrain->normals[p];
                ++p;
            }
        this->geometry = new DataBuffer3(v);
    }
    virtual void draw_geometry() {
        for (int i = 0; i < size; i += 2)
            glDrawArrays(draw_mode, i, 2);
    }
private:
    int size;
};

const int orig_size = 101;

vector<float> getData() {
    ifstream is("textures/fontvieille.mnt.txt");
    std::string buffer;
    getline(is, buffer);
    vector<int> orig_heights(orig_size * orig_size);
    for (int &h : orig_heights)
        is >> h;

    int size = orig_size * 2 - 1;
    vector<float> heights(size * size);
    int p = 0;
    for (int i = 0; i < orig_size; ++i) {
        int q = i * orig_size;
        for (int j = 0; j < orig_size - 1; ++j) {
            heights[p++] = orig_heights[q];
            heights[p++] = (orig_heights[q] + orig_heights[q + 1] ) / 2.f;
            ++q;
        }
        heights[p++] = orig_heights[q];
        ++q;

        if (i < orig_size - 1) {
            int q = i * orig_size;
            for (int j = 0; j < orig_size - 1; ++j) {
                heights[p++] = (orig_heights[q] + orig_heights[q + orig_size]) / 2.f;
                heights[p] = (orig_heights[q + orig_size] + orig_heights[q + orig_size + 1]) / 4.f + heights[p - size] / 2.f;
                ++p;
                ++q;
            }
            heights[p++] = (orig_heights[q] + orig_heights[q + orig_size]) / 2.f;
        }
    }
    return heights;
    //return vector<float>(orig_heights.begin(), orig_heights.end());
}


class HouseScene : public Scene {
private:
    Terrain *terrain;
public:
    HouseScene(Camera *camera) : Scene("House", 1000, 700, camera) { }

    void init() {
        terrain = new Terrain(getData(), orig_size * 2 - 1, center);
        /*
        Drawable *normals = new Normals(terrain, center);
        add(normals);
         */
        add(terrain);
        set_wheel_callback(scroll);
    }

    virtual ~HouseScene() {
    }

    virtual void process_events() {
        if (key_state(GLFW_KEY_LEFT))
            center->move_horizontal(.2f);
        if (key_state(GLFW_KEY_RIGHT))
            center->move_horizontal(-.2f);
        if (key_state(GLFW_KEY_UP))
            center->move_vertical(-.2f);
        if (key_state(GLFW_KEY_DOWN))
            center->move_vertical(.2f);
    }

    void before_iteration(float time) {
        ::program->set_uniform("lightPosition",
                               //glm::vec3(center->get_model(time) * glm::rotateX(glm::vec4(0, 60, 0, 1), time * .5f)));
                               glm::vec3(center->get_model(time) * glm::vec4(0, 60, 0, 1)));
        ::program->set_uniform("lightColor", {.9, .9, .9});
        ::program->set_uniform("ambientLight", {.2, .2, .2});
    }

};

int main() {
    camera = new PerspectiveCamera(1000, 700, 1.5, 1, 200);
    HouseScene *scene = new HouseScene(camera);
    center = new Center();
    program = new MvpProgram("shaders/mvp_vertex.glsl", "shaders/texture_fragment.glsl");
    camera->look_at(glm::vec3(0, cam_dist * .6f, cam_dist), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    scene->init();

    scene->run();

    delete center;
    delete scene;
    delete camera;
    delete program;
    return 0;
}