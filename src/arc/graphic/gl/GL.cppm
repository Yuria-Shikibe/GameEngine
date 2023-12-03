//
// Created by Matrix on 2023/12/3.
//
module;

export module GL;

import <unordered_set>;
import <glad/glad.h>;

namespace GL {

std::unordered_set<GLenum> currentState;

}



export namespace GL {
    void enable(const GLenum cap) {
        if(!currentState.contains(cap)) {
            currentState.insert(cap);
            glEnable(cap);
        }
    }

    void disable(const GLenum cap) {
        if(currentState.contains(cap)) {
            currentState.erase(cap);
            glDisable(cap);
        }
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
}

