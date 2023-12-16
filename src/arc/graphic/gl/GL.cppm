//
// Created by Matrix on 2023/12/3.
//
module;

export module GL;

import <unordered_set>;
import <glad/glad.h>;

namespace GL {
    GLuint lastProgram{0};

    std::unordered_set<GLenum> currentState;

    int maxTexSize = 4096;

    int viewport_x{0};
    int viewport_y{0};
    int viewport_w{0};
    int viewport_h{0};
}

export namespace GL {
    GLuint useProgram(const GLuint program) {
        if(program != lastProgram) {
            glUseProgram(program);
        }

        return lastProgram;
    }

    void enable(const GLenum cap) {
        if(!currentState.contains(cap)) {
            currentState.insert(cap);
            glEnable(cap);
        }
    }

    unsigned int getMaxTextureSize() {
        return maxTexSize;
    }

    void disable(const GLenum cap) {
        if(currentState.contains(cap)) {
            currentState.erase(cap);
            glDisable(cap);
        }
    }

    void init() {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glDepthFunc(GL_ALWAYS);

        enable(GL_DEPTH_TEST);
        enable(GL_STENCIL_TEST);
        disable(GL_STENCIL_TEST);
        disable(GL_DEPTH_TEST);
    }

    bool getState(const GLenum cap) {
        return currentState.contains(cap);
    }

    void setState(const GLenum cap, const bool enable) {
        if(enable) {
            GL::enable(cap);
        }else {
            GL::disable(cap);
        }
    }

    void viewport(const GLsizei x, const GLsizei y, const GLsizei width, const GLsizei height) {
        if(x == viewport_x && y == viewport_y && viewport_w == width && viewport_h == height)return;
        glViewport(x, y, width, height);
        viewport_x = x;
        viewport_y = y;
        viewport_w = width;
        viewport_h = height;
    }

    void viewport(const GLsizei width, const GLsizei height) {
        viewport(0, 0, width, height);
    }
}

