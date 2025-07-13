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
    gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress));
    glfwSwapInterval(1);

    GLuint array_texture = 0;
    GLuint texture_views[4] = { 0, 0, 0, 0 };
    GLuint framebuffers[4] = { 0, 0, 0, 0 };
    int texture_width = 32;
    int texture_height = 32;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &array_texture);
    printf("array_texture = %u\n", array_texture);
    glTextureStorage3D(array_texture, 1, GL_RGBA8, texture_width, texture_height, 4);
    glGenTextures(4, &texture_views[0]);
    glCreateFramebuffers(4, &framebuffers[0]);
    for (int i = 0; i < 4; ++i)
    {
        glTextureView(texture_views[i], GL_TEXTURE_2D, array_texture, GL_RGBA8, 0, 1, i, 1);
        glNamedFramebufferTexture(framebuffers[i], GL_COLOR_ATTACHMENT0, texture_views[i], 0);
        printf("texture_views[%d] = %u\n", i, texture_views[i]);
        printf("framebuffers[%d] = %u\n", i, framebuffers[i]);
    }
    {
        GLenum error_code = glGetError();
        if (error_code != GL_NO_ERROR)
        {
            printf("error");
            abort();
        }
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

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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

        unsigned char pink [4] = { 0xff, 0x44, 0x88, 0xff };
        unsigned char red  [4] = { 0xff, 0x00, 0x00, 0xff };
        unsigned char green[4] = { 0x00, 0xff, 0x00, 0xff };
        unsigned char blue [4] = { 0x00, 0x00, 0xff, 0xff };
        unsigned char white[4] = { 0xff, 0xff, 0xff, 0xff };
        glClearTexImage(array_texture,    0, GL_RGBA, GL_UNSIGNED_BYTE, &pink [0]);
        glClearTexImage(texture_views[0], 0, GL_RGBA, GL_UNSIGNED_BYTE, &red  [0]);
        glClearTexImage(texture_views[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, &green[0]);
        glClearTexImage(texture_views[2], 0, GL_RGBA, GL_UNSIGNED_BYTE, &blue [0]);
        glClearTexImage(texture_views[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, &white[0]);

        for (int i = 0; i < 4; ++i) {
            glBlitNamedFramebuffer(
                framebuffers[i],          // read framebuffer
                0,                        // draw framebuffer

                0,                        // srcX0
                0,                        // srcY0
                texture_width,            // srcX1
                texture_height,           // srcY1

                (i) * texture_width,      // dstX0
                0,                        // dstY0
                (i + 1) * texture_width,  // dstX1
                texture_height,           // dstY1

                GL_COLOR_BUFFER_BIT,      // mask
                GL_NEAREST                // filter
            );
        }
        {
            GLenum error_code = glGetError();
            if (error_code != GL_NO_ERROR)
            {
                printf("error");
                abort();
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
