#pragma once

#include <glad/glad.h>

#include <functional>
#include <utility>

namespace core::render {

class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual void makeCurrent() = 0;
    virtual void beginFrame(int framebufferWidth, int framebufferHeight, float dpiScale) = 0;
    virtual void present() = 0;
};

class OpenGLRenderBackend final : public RenderBackend {
public:
    using Callback = std::function<void()>;

    OpenGLRenderBackend(Callback makeCurrent, Callback present)
        : makeCurrent_(std::move(makeCurrent)), present_(std::move(present)) {}

    void makeCurrent() override {
        if (makeCurrent_) {
            makeCurrent_();
        }
    }

    void beginFrame(int framebufferWidth, int framebufferHeight, float) override {
        makeCurrent();
        glViewport(0, 0, framebufferWidth, framebufferHeight);
    }

    void present() override {
        if (present_) {
            present_();
        }
    }

private:
    Callback makeCurrent_;
    Callback present_;
};

} // namespace core::render
