module;

#include <glad/glad.h>

export module GL;

import std;
import ext.RuntimeException;
import Geom.Rect_Orthogonal;

namespace GL {
    GLuint lastProgram{0};

    std::unordered_set<GLenum> currentState;
    std::unordered_map<GLuint, std::unordered_set<GLenum>> states;

    std::unordered_map<GLenum, GLuint> bufferBindState;

    int maxTexSize = 4096;

    int viewport_x{0};
    int viewport_y{0};
    int viewport_w{0};
    int viewport_h{0};

    GLuint currentDrawFrameBufferID = 0;
    GLuint currentReadFrameBufferID = 0;

    Geom::OrthoRectInt scissorRect{};

    unsigned scissorShrinkActivatedCount{0};

    GLenum
        globalBlend_src{}, globalBlend_dst{}, globalBlend_srcAlpha{}, globalBlend_dstAlpha;
}

export namespace GL {
    void defGlDebugCallback(GLenum source, GLenum type, const GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param){
#ifndef _DEBUG
        return;
#endif

        const std::string_view src_str = [source]{
            switch (source)
            {
                case GL_DEBUG_SOURCE_API: return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
                case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
                case GL_DEBUG_SOURCE_OTHER: return "OTHER";
                default: return "UNKNOWN";
            }
        }();

        const std::string_view type_str = [type]() {
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR: return "ERROR";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
                case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
                case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
                case GL_DEBUG_TYPE_MARKER: return "MARKER";
                case GL_DEBUG_TYPE_OTHER: return "OTHER";
                default: return "UNKNOWN";
            }
        }();

        const std::string_view severity_str = [severity]{
            switch (severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
                case GL_DEBUG_SEVERITY_LOW: return "LOW";
                case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
                case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
                default: return "UNKNOWN";
            }
        }();

        std::println(std::cout, "[GL {}-{}-{} | {}]: {}", src_str, type_str, severity_str, id, message);
        std::cout.flush();
    }

    void setupGLDebug(){
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(defGlDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }

    namespace Test {
        constexpr GLenum DEPTH = 0x0B71;
        constexpr GLenum STENCIL = 0x0B90;
        constexpr GLenum SIMULTANEOUS_TEXTURE_AND_DEPTH = 0x82AC;
        constexpr GLenum SIMULTANEOUS_TEXTURE_AND_STENCIL = 0x82AD;
        constexpr GLenum FASTEST = 0x1101;
        constexpr GLenum SCISSOR = 0x0C11;
    }

    namespace State{
        constexpr GLenum BLEND = GL_BLEND;
    }

    enum class Func : GLenum {
        NEVER = GL_NEVER,
        LESS = GL_LESS,
        EQUAL = GL_EQUAL,
        LEQUAL = GL_LEQUAL,
        GREATER = GL_GREATER,
        NOTEQUAL = GL_NOTEQUAL,
        GEQUAL = GL_GEQUAL,
        ALWAYS = GL_ALWAYS,
    };

    enum class Operation : GLenum {
        KEEP = GL_KEEP,
        ZERO = GL_ZERO,
        REPLACE = GL_REPLACE,
        INCR = GL_INCR,
        INCR_WRAP = GL_INCR_WRAP,
        DECR = GL_DECR,
        DECR_WRAP = GL_DECR_WRAP,
        INVERT = GL_INVERT,
    };

    int getMaxTextureSize() {
        return maxTexSize;
    }
    
    GLuint useProgram(const GLuint program) {
        if(program != lastProgram) {
            glUseProgram(program);
        }

        return lastProgram;
    }

    void bindBuffer(const GLenum flag, const GLuint id) {
        // auto& current = bufferBindState[flag];

        // if(current == id)return;

        glBindBuffer(flag, id);
        // current = id;
    }

    void unbindBuffer(const GLenum flag) {
        bindBuffer(flag, 0);
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
        if(auto& state = states[cap]; !state.contains(cap)) {
            state.insert(cap);
            glEnablei(cap, id);
        }
    }

    void disablei(const GLenum cap, const GLuint id) {
        if(auto& state = states[cap]; state.contains(cap)) {
            state.erase(cap);
            glDisablei(cap, id);
        }
    }

    bool cleari(const GLuint id) {
        if(const auto itr = states.find(id); itr != states.end()) {
            itr->second.clear();
            return true;
        }

        return false;
    }

    void init() {
        setupGLDebug();
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glDepthFunc(GL_ALWAYS);

        enable(GL_DEPTH_TEST);
        enable(GL_STENCIL_TEST);
        disable(GL_STENCIL_TEST);
        disable(GL_DEPTH_TEST);

        viewport_h = viewport_w = viewport_x = viewport_y = 0;
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

    void blit(const GLint srcX0, const GLint srcY0, const GLint srcX1, const GLint srcY1, const GLint dstX0, const GLint dstY0, const GLint dstX1, const GLint dstY1, const GLbitfield mask, const GLenum filter){
        glBlitFramebuffer(
                srcX0, srcY0, srcX1, srcY1,
                dstX0, dstY0, dstX1, dstY1,
            mask, filter);
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

    [[nodiscard]] Geom::OrthoRectInt getViewPort(){
        return {viewport_x, viewport_y, viewport_w, viewport_h};
    }

    void scissorShrinkBegin(){
        scissorShrinkActivatedCount++;
    }

    void scissorShrinkEnd(){
        if(scissorShrinkActivatedCount){
            scissorShrinkActivatedCount--;
        }else{
            throw ext::IllegalArguments{"Scissor Shrink Deactivated After Count Is Zero!"};
        }
    }

    unsigned getSrhinkCount(){
        return scissorShrinkActivatedCount;
    }

    Geom::OrthoRectInt getScissorRect(){
        return scissorRect;
    }

    void forceSetScissor(const Geom::OrthoRectInt rect){
        scissorRect = rect;
        glScissor(
            rect.getSrcX(),
            rect.getSrcY(),
            rect.getWidth(),
            rect.getHeight());
    }

    void setScissor(Geom::OrthoRectInt cur) {
        if(cur == scissorRect)return;
        if(scissorShrinkActivatedCount){
            const Geom::OrthoRectInt rect = scissorRect.getOverlap(cur);

            glScissor(
                rect.getSrcX(),
                rect.getSrcY(),
                rect.getWidth(),
                rect.getHeight());

            scissorRect = rect;
        }else{
            glScissor(cur.getSrcX(), cur.getSrcY(), cur.getWidth(), cur.getHeight());
            scissorRect = cur;
        }
    }

    void blendFunc(const GLenum src, const GLenum dst) {
        // if(src == globalBlend_src && dst == globalBlend_dst && src == globalBlend_srcAlpha && dst == globalBlend_dstAlpha)return;
        glBlendFunc(src, dst);
        globalBlend_src = globalBlend_srcAlpha = src;
        globalBlend_dst = globalBlend_dstAlpha = dst;
    }

    void blendFunc(const GLenum src, const GLenum dst, const GLenum srcAlpha, const GLenum dstAlpha) {
        // if(src == globalBlend_src && dst == globalBlend_dst && src == globalBlend_srcAlpha && dst == globalBlend_dstAlpha)return;
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

    void setDepthFunc(const Func func){
        glDepthFunc(static_cast<const GLenum>(func));
    }

    void setDepthMask(const bool enable){
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void setStencilFunc(const Func func, const GLint ref, const GLuint mask){
        glStencilFunc(static_cast<const GLenum>(func), ref, mask);
    }

    void setStencilMask(const GLuint mask){
        glStencilMask(mask);
    }

    void setStencilOperation(Operation stencilFail, Operation depthFail, Operation pass){
        glStencilOp(static_cast<GLenum>(stencilFail), static_cast<GLenum>(depthFail), static_cast<GLenum>(pass));
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
}

