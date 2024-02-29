//
// Created by Matrix on 2023/12/3.
//
module;

export module GL;

import <unordered_map>;
import <unordered_set>;
import <glad/glad.h>;

namespace GL {
    GLuint lastProgram{0};

    std::unordered_set<GLenum> currentState;
    std::unordered_map<GLuint, std::unordered_set<GLenum>> bufferState;

    int maxTexSize = 4096;

    int viewport_x{0};
    int viewport_y{0};
    int viewport_w{0};
    int viewport_h{0};

    GLuint currentDrawFrameBufferID = 0;
    GLuint currentReadFrameBufferID = 0;

    GLint scissor_x{0};
    GLint scissor_y{0};
    GLint scissor_w{0};
    GLint scissor_h{0};

    GLenum
        globalBlend_src{}, globalBlend_dst{}, globalBlend_srcAlpha{}, globalBlend_dstAlpha;
}

export namespace GL {
    namespace Test {
        constexpr GLenum DEPTH = 0x0B71;
        constexpr GLenum STENCIL = 0x0B90;
        constexpr GLenum SIMULTANEOUS_TEXTURE_AND_DEPTH = 0x82AC;
        constexpr GLenum SIMULTANEOUS_TEXTURE_AND_STENCIL = 0x82AD;
        constexpr GLenum FASTEST = 0x1101;
        constexpr GLenum SCISSOR = 0x0C11;
    }


    unsigned int getMaxTextureSize() {
        return maxTexSize;
    }
    
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

    void disable(const GLenum cap) {
        if(currentState.contains(cap)) {
            currentState.erase(cap);
            glDisable(cap);
        }
    }

    void enablei(const GLenum cap, const GLuint id) {
        if(auto& state = bufferState[cap]; !state.contains(cap)) {
            state.insert(cap);
            glEnablei(cap, id);
        }
    }

    void disablei(const GLenum cap, const GLuint id) {
        if(auto& state = bufferState[cap]; state.contains(cap)) {
            state.erase(cap);
            glDisablei(cap, id);
        }
    }

    bool cleari(const GLuint id) {
        if(const auto itr = bufferState.find(id); itr != bufferState.end()) {
            itr->second.clear();
            return true;
        }

        return false;
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
        }else{
            GL::disable(cap);
        }
    }

    void bindFrameBuffer(const GLenum flag, const GLuint id = 0) {
        if(flag == GL_FRAMEBUFFER) {
            if(id != currentDrawFrameBufferID || id != currentReadFrameBufferID) {
                currentDrawFrameBufferID = currentReadFrameBufferID = id;
                glBindFramebuffer(flag, id);
            }
        }else if(flag == GL_READ_FRAMEBUFFER) {
            if(id != currentReadFrameBufferID) {
                currentReadFrameBufferID = id;
                glBindFramebuffer(flag, id);
            }
        }else {
            if(id != currentDrawFrameBufferID) {
                currentDrawFrameBufferID = id;
                glBindFramebuffer(flag, id);
            }
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

    void scissor(const GLint x, const GLint y, const GLint width, const GLint height) {
        if(x == scissor_x && y == scissor_y && scissor_w == width && scissor_h == height)return;
        glScissor(x, y, width, height);
        scissor_x = x;
        scissor_y = y;
        scissor_w = width;
        scissor_h = height;
    }

    void blendFunc(const GLenum src, const GLenum dst) {
        if(src == globalBlend_src && dst == globalBlend_dst && src == globalBlend_srcAlpha && dst == globalBlend_dstAlpha)return;
        glBlendFunc(src, dst);
        globalBlend_src = globalBlend_srcAlpha = src;
        globalBlend_dst = globalBlend_dstAlpha = dst;
    }

    void blendFunc(const GLenum src, const GLenum dst, const GLenum srcAlpha, const GLenum dstAlpha) {
        if(src == globalBlend_src && dst == globalBlend_dst && src == globalBlend_srcAlpha && dst == globalBlend_dstAlpha)return;
        glBlendFuncSeparate(src, dst, srcAlpha, dstAlpha);
        globalBlend_src = src;
        globalBlend_srcAlpha = srcAlpha;
        globalBlend_dst = dst;
        globalBlend_dstAlpha = dstAlpha;
    }

    void blendFunci(const GLuint buf, const GLenum src, const GLenum dst) {
        glBlendFunci(buf, src, dst);
    }

    void blendFunci(const GLuint buf, const GLenum src, const GLenum dst, const GLenum srcAlpha, const GLenum dstAlpha) {
        glBlendFuncSeparatei(buf, src, dst, srcAlpha, dstAlpha);
    }
}

