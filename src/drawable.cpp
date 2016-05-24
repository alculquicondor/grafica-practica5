#include "drawable.h"


Drawable::Drawable(MvpProgram *program, Buffer *geometry, GLenum draw_mode, Buffer *color, Texture *texture, Buffer *uv,
                   Buffer *normal, IndexBuffer *index_buffer, Drawable *parent) :
        program(program), geometry(geometry), color(color), draw_mode(draw_mode), parent(parent),
        texture(texture), uv(uv), normal(normal), index_buffer(index_buffer) { }


Drawable::Drawable(MvpProgram *program, Drawable *parent) :
        program(program), geometry(nullptr), color(nullptr), draw_mode(GL_TRIANGLES), texture(nullptr), uv(nullptr),
        normal(nullptr), index_buffer(nullptr), parent(parent) { }


Drawable::~Drawable() {
}


void Drawable::set_parent(Drawable *parent) {
    this->parent = parent;
}


void Drawable::draw(Camera *camera, float time) {
    if (geometry == nullptr)
        return;
    program->activate();
    geometry->activate();
    if (color != nullptr)
        color->activate();
    if (texture != nullptr)
        texture->load();
    if (uv != nullptr)
        uv->activate();
    if (normal != nullptr)
        normal->activate();
    if (index_buffer != nullptr)
        index_buffer->activate();

    glm::mat4 m = get_model(time), v(1), p(1);
    if (camera != nullptr) {
        v = camera->get_v();
        p = camera->get_p();
    }
    program->set_m(m);
    program->set_v(v);
    program->set_mvp(p * v * m);
    draw_geometry();

    geometry->deactivate();
    if (color != nullptr)
        color->deactivate();
    if (uv != nullptr)
        uv->deactivate();
    if (normal != nullptr)
        normal->deactivate();
}


void Drawable::draw_geometry() {
    if (index_buffer != nullptr)
        glDrawElements(draw_mode, index_buffer->size(), GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(draw_mode, 0, geometry->size());
}


glm::mat4 Drawable::get_model(float time) {
    return parent != nullptr ? parent->get_model(time) : glm::mat4(1.0f);
}
