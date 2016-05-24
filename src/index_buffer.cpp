#include "index_buffer.h"

IndexBuffer::IndexBuffer(const std::vector<unsigned int> &data) : data(data) {
    glGenBuffers(1, &buffer_id);
}

IndexBuffer::~IndexBuffer() { }

void IndexBuffer::load_data() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
    data_loaded = true;
}


void IndexBuffer::activate() {
    if (not data_loaded)
        load_data();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
}


GLsizei IndexBuffer::size() const {
    return (GLsizei) data.size();
}
