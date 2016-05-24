#ifndef HELLO_OPENGL_DRAWABLE_H
#define HELLO_OPENGL_DRAWABLE_H

#include "buffer.h"
#include "camera.h"
#include "mvp_program.h"
#include "texture.h"
#include "index_buffer.h"


class Drawable {
public:
    Drawable(MvpProgram *program, Buffer *geometry, GLenum draw_mode = GL_TRIANGLES,
             Buffer *color = nullptr, Texture *texture = nullptr, Buffer *uv = nullptr, Buffer *normal = nullptr,
             IndexBuffer *index_buffer = nullptr, Drawable *parent = nullptr);
    Drawable(MvpProgram *program, Drawable *parent = nullptr);

    void set_parent(Drawable *parent);

    virtual ~Drawable();
    virtual void draw(Camera *camera, float time=0);
    virtual void draw_geometry();
    virtual glm::mat4 get_model(float time);

protected:
    GLenum draw_mode;
    Buffer *geometry;
    MvpProgram *program;
    Buffer *color;
    Drawable *parent;
    Texture *texture;
    Buffer *uv;
    Buffer *normal;
    IndexBuffer *index_buffer;
};


#endif //HELLO_OPENGL_DRAWABLE_H
