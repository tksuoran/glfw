//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;

static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        exit(EXIT_FAILURE);
    }
    // Prepare staging buffer
    GLuint staging_buffer = 0;
    size_t float_count = 16384;
    size_t byte_count = sizeof(float) * float_count;
    void* data = malloc(byte_count);
    memset(data, 0, byte_count);
    glCreateBuffers(1, &staging_buffer);
    glNamedBufferStorage(staging_buffer, byte_count, data, GL_CLIENT_STORAGE_BIT);

    // Prepare 3 buffers for vertex attributes
    GLuint buffers[3] = { 0, 0, 0 };
    glCreateBuffers(3, &buffers[0]);
    for (size_t i = 0; i < 3; ++i) {
        glNamedBufferStorage(buffers[i], byte_count, NULL, 0);
        glCopyNamedBufferSubData(staging_buffer, buffers[i], 0, 0, byte_count);
    }

    // Prepare VAO
    GLuint vaobj = 0;
    glCreateVertexArrays(1, &vaobj);
    glVertexArrayAttribBinding(vaobj, 0, 0);
    glVertexArrayAttribBinding(vaobj, 1, 1);
    glVertexArrayAttribBinding(vaobj, 2, 2);
    glVertexArrayVertexBuffer(vaobj, 0, buffers[0], 0, 16);
    glVertexArrayVertexBuffer(vaobj, 1, buffers[1], 0, 16);
    glVertexArrayVertexBuffer(vaobj, 2, buffers[2], 0, 16);

    gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        printf("Warning: GL error before query: %04x.", gl_error);
    }

    // Query VAO
    GLuint query_vertex_attrib_vertex_array_buffer_bindings[3] = { 0, 0, 0 };
    GLuint query_vertex_attrib_bindings[3] = { 0, 0, 0 };
    GLuint query_vertex_binding_buffers[3] = { 0, 0, 0 };
    int error_count = 0;
    int mismatch_count = 0;
    for (GLuint index = 0; index < 3; ++index) {
        // Query attribute binding
        glGetVertexArrayIndexediv(vaobj, index, GL_VERTEX_ATTRIB_BINDING, &query_vertex_attrib_bindings[index]);
        gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            printf(
                "glGetVertexArrayIndexediv(vaobj = %u, index = %u, pname = GL_VERTEX_ATTRIB_BINDING, "
                "param = %p) failed with error %04x.",
                vaobj, index, &query_vertex_attrib_bindings[index], gl_error
            );
            ++error_count;
        } else {
            printf("Attribute %u expected binding %u found binding %u\n", index, index, query_vertex_attrib_bindings[index]);
            if (query_vertex_attrib_bindings[index] != index) {
                ++mismatch_count;
            }
        }

        // Query binding buffer
        glGetVertexArrayIndexediv(vaobj, index, GL_VERTEX_BINDING_BUFFER, &query_vertex_binding_buffers[index]);
        gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            printf(
                "glGetVertexArrayIndexediv(vaobj = %u, index = %u, pname = GL_VERTEX_ATTRIB_BINDING, "
                "param = %p) failed with error %04x.",
                vaobj, index, &query_vertex_binding_buffers[index], gl_error
            );
            ++error_count;
        } else {
            printf("Binding %u expected buffer %u found buffer %u\n", index, buffers[index], query_vertex_binding_buffers[index]);
            if (query_vertex_binding_buffers[index] != buffers[index]) {
                ++mismatch_count;
            }
        }

        // Query attribute buffer (legacy API)
        glGetVertexArrayIndexediv(vaobj, index, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &query_vertex_attrib_vertex_array_buffer_bindings[index]);
        gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            printf(
                "glGetVertexArrayIndexediv(vaobj = %u, index = %u, pname = GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, "
                "param = %p) failed with error %04x.",
                vaobj, index, &query_vertex_attrib_vertex_array_buffer_bindings[index], gl_error
            );
            ++error_count;
        } else {
            printf(
                "Attribute %u expected buffer %u found buffer %u\n",
                index, buffers[index], query_vertex_attrib_vertex_array_buffer_bindings[index]
            );
            if (query_vertex_attrib_vertex_array_buffer_bindings[index] != buffers[index]) {
                ++mismatch_count;
            }
        }
    }

    if ((error_count > 0) || (mismatch_count > 0)) {
        exit(EXIT_FAILURE);
    }

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
