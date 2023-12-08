//
// Created by Matrix on 2023/12/3.
//
module;

export module GL;

import <unordered_set>;
import <glad/glad.h>;

namespace GL {
    std::unordered_set<GLenum> currentState;

    int maxTexSize = 4096;
}

export namespace GL {
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
        glViewport(x, y, width, height);
    }

    void viewport(const GLsizei width, const GLsizei height) {
        viewport(0, 0, width, height);
    }
}

