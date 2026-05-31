#include "core/platform/window_backend.h"
#include "core/platform/native_bridge.h"

#if defined(EUI_WINDOW_BACKEND_SDL2)

#include <SDL.h>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <imm.h>
#include <SDL_syswm.h>
#endif

namespace core::window {

#if defined(_WIN32)
namespace {

LONG roundLong(float value) {
    return static_cast<LONG>(value >= 0.0f ? value + 0.5f : value - 0.5f);
}

HWND hwndForWindow(Handle window) {
    if (window == nullptr) {
        return nullptr;
    }
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(static_cast<SDL_Window*>(window), &info) != SDL_TRUE ||
        info.subsystem != SDL_SYSWM_WINDOWS) {
        return nullptr;
    }
    return info.info.win.window;
}

} // namespace
#endif

ContextKey currentContextKey() {
    return SDL_GL_GetCurrentContext();
}

double timeSeconds() {
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    return frequency > 0
        ? static_cast<double>(SDL_GetPerformanceCounter()) / static_cast<double>(frequency)
        : 0.0;
}

void postEmptyEvent() {
    SDL_Event event{};
    event.type = SDL_USEREVENT;
    SDL_PushEvent(&event);
}

void getCursorPosition(Handle, double& x, double& y) {
    int cursorX = 0;
    int cursorY = 0;
    SDL_GetMouseState(&cursorX, &cursorY);
    x = static_cast<double>(cursorX);
    y = static_cast<double>(cursorY);
}

bool isMouseButtonDown(Handle, int button) {
    const Uint32 state = SDL_GetMouseState(nullptr, nullptr);
    const Uint32 mask = button == 1 ? SDL_BUTTON_RMASK : SDL_BUTTON_LMASK;
    return (state & mask) != 0;
}

std::string clipboardText(Handle) {
    char* text = SDL_GetClipboardText();
    if (text == nullptr) {
        return {};
    }
    std::string result(text);
    SDL_free(text);
    return result;
}

void setClipboardText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

CursorHandle createStandardCursor(CursorType type) {
    return SDL_CreateSystemCursor(type == CursorType::Hand ? SDL_SYSTEM_CURSOR_HAND : SDL_SYSTEM_CURSOR_ARROW);
}

void setCursor(Handle, CursorHandle cursor) {
    SDL_SetCursor(static_cast<SDL_Cursor*>(cursor));
}

void destroyCursor(CursorHandle cursor) {
    SDL_FreeCursor(static_cast<SDL_Cursor*>(cursor));
}

void setWindowIcon(Handle window, int width, int height, unsigned char* pixels) {
    if (window == nullptr || pixels == nullptr || width <= 0 || height <= 0) {
        return;
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
    if (surface != nullptr) {
        SDL_SetWindowIcon(static_cast<SDL_Window*>(window), surface);
        SDL_FreeSurface(surface);
    }
    eui_set_application_icon_rgba(width, height, pixels);
}

void setImeCursorRect(Handle window, float x, float y, float width, float height) {
#if defined(_WIN32)
    HWND hwnd = hwndForWindow(window);
    if (hwnd != nullptr) {
        HIMC context = ImmGetContext(hwnd);
        if (context != nullptr) {
            const LONG caretX = roundLong(x);
            const LONG caretY = roundLong(y + height);

            COMPOSITIONFORM composition{};
            composition.dwStyle = CFS_FORCE_POSITION;
            composition.ptCurrentPos.x = caretX;
            composition.ptCurrentPos.y = caretY;
            composition.rcArea.left = roundLong(x);
            composition.rcArea.top = roundLong(y);
            composition.rcArea.right = roundLong(x + width);
            composition.rcArea.bottom = roundLong(y + height);
            ImmSetCompositionWindow(context, &composition);

            CANDIDATEFORM candidate{};
            candidate.dwIndex = 0;
            candidate.dwStyle = CFS_CANDIDATEPOS;
            candidate.ptCurrentPos.x = caretX;
            candidate.ptCurrentPos.y = caretY;
            candidate.rcArea = composition.rcArea;
            ImmSetCandidateWindow(context, &candidate);

            ImmReleaseContext(hwnd, context);
        }
    }

    SDL_Rect rect{
        roundLong(x),
        roundLong(y),
        roundLong(width),
        roundLong(height)
    };
    SDL_SetTextInputRect(&rect);
#else
    int windowWidth = 0;
    int windowHeight = 0;
    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GetWindowSize(static_cast<SDL_Window*>(window), &windowWidth, &windowHeight);
    SDL_GL_GetDrawableSize(static_cast<SDL_Window*>(window), &drawableWidth, &drawableHeight);
    const float scaleX = windowWidth > 0 && drawableWidth > 0
        ? static_cast<float>(drawableWidth) / static_cast<float>(windowWidth)
        : 1.0f;
    const float scaleY = windowHeight > 0 && drawableHeight > 0
        ? static_cast<float>(drawableHeight) / static_cast<float>(windowHeight)
        : 1.0f;
    SDL_Rect rect{
        static_cast<int>(x / scaleX),
        static_cast<int>(y / scaleY),
        static_cast<int>(width / scaleX),
        static_cast<int>(height / scaleY)
    };
    SDL_SetTextInputRect(&rect);
#endif
}

} // namespace core::window

#else

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#include "core/platform/ime_bridge.h"

namespace core::window {

ContextKey currentContextKey() {
    return glfwGetCurrentContext();
}

double timeSeconds() {
    return glfwGetTime();
}

void postEmptyEvent() {
    glfwPostEmptyEvent();
}

void getCursorPosition(Handle window, double& x, double& y) {
    glfwGetCursorPos(static_cast<GLFWwindow*>(window), &x, &y);
}

bool isMouseButtonDown(Handle window, int button) {
    const int glfwButton = button == 1 ? GLFW_MOUSE_BUTTON_RIGHT : GLFW_MOUSE_BUTTON_LEFT;
    return glfwGetMouseButton(static_cast<GLFWwindow*>(window), glfwButton) == GLFW_PRESS;
}

std::string clipboardText(Handle window) {
    const char* text = glfwGetClipboardString(static_cast<GLFWwindow*>(window));
    return text != nullptr ? text : "";
}

void setClipboardText(const std::string& text) {
    glfwSetClipboardString(glfwGetCurrentContext(), text.c_str());
}

CursorHandle createStandardCursor(CursorType type) {
    return glfwCreateStandardCursor(type == CursorType::Hand ? GLFW_HAND_CURSOR : GLFW_ARROW_CURSOR);
}

void setCursor(Handle window, CursorHandle cursor) {
    glfwSetCursor(static_cast<GLFWwindow*>(window), static_cast<GLFWcursor*>(cursor));
}

void destroyCursor(CursorHandle cursor) {
    glfwDestroyCursor(static_cast<GLFWcursor*>(cursor));
}

void setWindowIcon(Handle window, int width, int height, unsigned char* pixels) {
    if (window == nullptr || pixels == nullptr || width <= 0 || height <= 0) {
        return;
    }
    GLFWimage image{};
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    glfwSetWindowIcon(static_cast<GLFWwindow*>(window), 1, &image);
    eui_set_application_icon_rgba(width, height, pixels);
}

void setImeCursorRect(Handle window, float x, float y, float width, float height) {
    eui_ime_set_cursor_rect(static_cast<GLFWwindow*>(window), x, y, width, height);
}

} // namespace core::window

#endif
