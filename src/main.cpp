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
    Terrain(Drawable *parent = nullptr) : Drawable(::program, parent) {
        int p, q;
        ifstream is("textures/fontvieille.mnt.txt");
        std::string buffer;
        getline(is, buffer);
        std::vector<glm::vec3> vertex(size * size);
        std::vector<glm::vec2> uv(size * size);
        std::vector<glm::vec3> normal(size * size);
        std::vector<glm::vec3> triangle_normal((std::size_t)((size - 1) * (size - 1) * 2));
        float duv = 1.f / (size - 1), h;
        p = 0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                is >> h;
                uv[p] = {j * duv, (size - i - 1) * duv};
                vertex[p] = {j - size / 2, h / 50., i - size / 2};
                normal[p] = {0, 0, 0};
                ++p;
            }
        }
        this->texture = new Texture("textures/fontvieille.tga", GL_BGR);
        this->geometry = new DataBuffer3(vertex, 0);
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
                        glm::cross((vertex[index[p - 2]] - vertex[index[p - 3]]),
                                   (vertex[index[p - 1]] - vertex[index[p - 3]])));
                index[p++] = (unsigned int)((i * size) + j + 1);
                index[p++] = (unsigned int)(((i + 1) * size) + j);
                index[p++] = (unsigned int)(((i + 1) * size) + j + 1);
                triangle_normal[q++] = glm::normalize(
                        glm::cross((vertex[index[p - 2]] - vertex[index[p - 3]]),
                                   (vertex[index[p - 1]] - vertex[index[p - 3]])));
            }
        }
        this->index_buffer = new IndexBuffer(index);

        p = 0;
        for (int i = 0; i < size; ++i) {
            int row_first = i * (size - 1) * 2, prev_row_first = (i - 1) * (size - 1) * 2;
            if (i < size - 1)
                normal[p] += triangle_normal[row_first];
            if (i > 0)
                normal[p] += triangle_normal[prev_row_first] +
                        triangle_normal[prev_row_first + 1];
            normal[p] = glm::normalize(normal[p]);
            ++p;
            for (int j = 1; j < size - 1; ++j) {
                if (i < size - 1)
                    for (int k = 2 * j - 2; k <= 2 * j; ++k)
                        normal[p] += triangle_normal[row_first + k];
                if (i > 0)
                    for (int k = 2 * j - 1; k <= 2 * j + 1; ++k)
                        normal[p] += triangle_normal[prev_row_first + k];
                normal[p] = glm::normalize(normal[p]);
                ++p;
            }
            if (i < size - 1)
                normal[p] += triangle_normal[row_first + 2 * size - 4] +
                        triangle_normal[row_first + 2 * size - 3];
            if (i > 0)
                normal[p] += triangle_normal[prev_row_first + 2 * size - 3];
            normal[p] = glm::normalize(normal[p]);
            ++p;
        }
        this->normal = new DataBuffer3(normal, 3);
    }
private:
    int size = 101;
};


class HouseScene : public Scene {
private:
    Terrain *terrain;
public:
    HouseScene(Camera *camera) : Scene("House", 1000, 700, camera) { }

    void init() {
        terrain = new Terrain(center);
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
                               glm::vec3(center->get_model(time) * glm::vec4(0, 40, 0, 1)));
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