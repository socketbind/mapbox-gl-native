#include <mbgl/shader/line_shader.hpp>
#include <mbgl/shader/line.vertex.hpp>
#include <mbgl/shader/line.fragment.hpp>
#include <mbgl/gl/gl.hpp>

#include <cstdio>

using namespace mbgl;

LineShader::LineShader() : Shader("line", shaders::line::vertex, shaders::line::fragment) {
    a_data = MBGL_CHECK_ERROR(glGetAttribLocation(program, "a_data"));
}

void LineShader::bind(GLbyte* offset) {
    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, 8, offset + 0));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_data));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_data, 4, GL_BYTE, false, 8, offset + 4));
}
