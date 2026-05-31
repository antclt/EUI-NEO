#pragma once

#include "eui/app.h"
#include "core/platform/async.h"
#include "core/platform/network.h"
#include "core/platform/platform.h"

#include <algorithm>
#include <cstdio>

namespace app {

struct AppRunner {
    bool needsRender = true;
    bool trayAvailable = false;
    bool hiddenToTray = false;
    int renderedFrames = 0;
    double lastTitleUpdate = 0.0;
    double nextFrameTime = 0.0;
    double frameInterval = 1.0 / 60.0;
    double lastFrameTime = 0.0;
    double lastFrameRateLimit = 0.0;
    double lastRefreshRateUpdate = 0.0;

    void resetTiming(double now) {
        lastTitleUpdate = now;
        nextFrameTime = now;
        lastFrameTime = now;
    }

    float consumeFrameDelta(double now) {
        const float deltaSeconds = static_cast<float>(now - lastFrameTime);
        lastFrameTime = now;
        return deltaSeconds;
    }

    bool initializeTray() {
        if (!trayEnabled()) {
            trayAvailable = false;
            return false;
        }
        trayAvailable = core::platform::initializeTray({
            trayTitle(),
            trayIconPath()
        });
        return trayAvailable;
    }

    void pollTray(bool wait = false) {
        core::platform::pollTray(wait);
    }

    bool consumeTrayExitRequested() {
        return core::platform::consumeTrayExitRequested();
    }

    bool consumeTrayShowRequested() {
        return core::platform::consumeTrayShowRequested();
    }

    bool consumeExternalReady() {
        const bool asyncReady = core::async::dispatchReady();
        return core::network::consumeAnyTextReady() || asyncReady;
    }

    bool anyAnimating(bool childAnimating) const {
        return isAnimating() || childAnimating;
    }

    void updateFrameInterval(double refreshRate, double now, bool force = false) {
        const double limit = frameRateLimit();
        if (!force && limit == lastFrameRateLimit && now - lastRefreshRateUpdate < 0.5) {
            return;
        }

        refreshRate = std::clamp(refreshRate, 30.0, 500.0);
        if (limit > 0.0) {
            refreshRate = std::min(refreshRate, limit);
        }
        frameInterval = 1.0 / std::max(1.0, refreshRate);
        lastFrameRateLimit = limit;
        lastRefreshRateUpdate = now;
    }

    void markRendered() {
        needsRender = false;
        ++renderedFrames;
    }

    template <typename SetTitleFn>
    void updateFrameTitle(double now, SetTitleFn&& setTitle) {
        if (!showFrameCountInTitle()) {
            return;
        }
        const double elapsed = now - lastTitleUpdate;
        if (elapsed < 1.0) {
            return;
        }

        char title[128];
        std::snprintf(title, sizeof(title), "%s - %.0f FPS", windowTitle(), renderedFrames / elapsed);
        setTitle(title);
        renderedFrames = 0;
        lastTitleUpdate = now;
    }

    void advanceFrameClock(double now, bool animating) {
        if (animating) {
            nextFrameTime += frameInterval;
            if (nextFrameTime <= now || nextFrameTime > now + frameInterval * 2.0) {
                nextFrameTime = now + frameInterval;
            }
        } else {
            nextFrameTime = now;
        }
    }
};

} // namespace app
