#ifndef HELLO_OPENGL_INDEX_BUFFER_H
#define HELLO_OPENGL_INDEX_BUFFER_H

#include <vector>

#include "GL/glew.h"


class IndexBuffer {
private:
    GLuint buffer_id;
    bool data_loaded;
    std::vector<unsigned int> data;

    void load_data();

public:
    IndexBuffer(const std::vector<unsigned int> &data);
    virtual ~IndexBuffer();
    void activate();

    GLsizei size() const;
};

#endif //HELLO_OPENGL_INDEX_BUFFER_H
