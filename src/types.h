#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <GLFW/glfw3.h>
#include <iostream>
#include "glm/glm.hpp"

struct ImgInfo
{
        unsigned int width, height;
        int bit_depth, color_type, rowbytes, channels, interlance_type;
        GLint format;
};

struct TexCoords
{
        GLuint texId;
        glm::vec4 coords;
        TexCoords() : texId(0), coords(0) {};
        TexCoords(GLuint tid, glm::vec4 v) : texId(tid), coords(v) {};
};

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
{
    os << "(" << v[0] << ", " << v[1] << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
    os << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v)
{
    os << "(" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ")";
    return os;
}


#endif //TYPES_H_INCLUDED