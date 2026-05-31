#pragma once

#include "core/render/render_types.h"
#include "core/platform/window_backend.h"

#if !defined(EUI_WINDOW_BACKEND_SDL2)
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#endif

#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

namespace core {

enum class CursorShape {
    Arrow,
    Hand
};

struct PointerEvent {
    double x = 0.0;
    double y = 0.0;
    double deltaX = 0.0;
    double deltaY = 0.0;
    bool down = false;
    bool pressedThisFrame = false;
    bool releasedThisFrame = false;
    bool rightDown = false;
    bool rightPressedThisFrame = false;
    bool rightReleasedThisFrame = false;
};

struct KeyboardEvent {
    std::string text;
    std::string pasteText;
    bool backspace = false;
    bool del = false;
    bool enter = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool home = false;
    bool end = false;
    bool selectAll = false;
    bool copy = false;
    bool cut = false;
    bool undo = false;
    bool redo = false;
    bool shift = false;
    bool escape = false;

    bool hasInput() const {
        return !text.empty() || !pasteText.empty() || backspace || del || enter ||
               left || right || up || down || home || end || selectAll || copy || cut ||
               undo || redo || escape;
    }
};

struct ScrollEvent {
    double x = 0.0;
    double y = 0.0;

    bool active() const {
        return x != 0.0 || y != 0.0;
    }
};

enum class InputKey {
    Backspace,
    Delete,
    Enter,
    Left,
    Right,
    Up,
    Down,
    Home,
    End,
    Escape,
    A,
    C,
    V,
    X,
    Y,
    Z
};

namespace detail {

struct InputQueue {
    std::string text;
    std::string pasteText;
    double scrollX = 0.0;
    double scrollY = 0.0;
    bool backspace = false;
    bool del = false;
    bool enter = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool home = false;
    bool end = false;
    bool selectAll = false;
    bool copy = false;
    bool cut = false;
    bool undo = false;
    bool redo = false;
    bool shift = false;
    bool escape = false;
};

struct PointerState {
    double lastX = 0.0;
    double lastY = 0.0;
    bool lastDown = false;
    bool lastRightDown = false;
};

inline std::unordered_map<window::Handle, InputQueue>& inputQueues() {
    static std::unordered_map<window::Handle, InputQueue> queues;
    return queues;
}

inline std::unordered_map<window::Handle, PointerState>& pointerStates() {
    static std::unordered_map<window::Handle, PointerState> states;
    return states;
}

inline InputQueue& inputQueue(window::Handle window) {
    return inputQueues()[window];
}

inline PointerState& pointerState(window::Handle window) {
    return pointerStates()[window];
}

inline void appendUtf8(std::string& output, unsigned int codepoint) {
    if (codepoint < 0x20) {
        return;
    }
    if (codepoint <= 0x7F) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

} // namespace detail

inline void queueTextInput(window::Handle window, const std::string& text) {
    detail::inputQueue(window).text += text;
}

inline void queueScrollInput(window::Handle window, double x, double y) {
    detail::InputQueue& queue = detail::inputQueue(window);
    queue.scrollX += x;
    queue.scrollY += y;
}

inline void queueKeyInput(window::Handle window, InputKey key, bool ctrl = false, bool shift = false) {
    detail::InputQueue& queue = detail::inputQueue(window);
    queue.shift = shift;
    if (ctrl && key == InputKey::V) {
        queue.pasteText += core::window::clipboardText(window);
        return;
    }
    if (ctrl && key == InputKey::C) {
        queue.copy = true;
        return;
    }
    if (ctrl && key == InputKey::X) {
        queue.cut = true;
        return;
    }
    if (ctrl && key == InputKey::Z) {
        shift ? queue.redo = true : queue.undo = true;
        return;
    }
    if (ctrl && key == InputKey::Y) {
        queue.redo = true;
        return;
    }
    if (ctrl && key == InputKey::A) {
        queue.selectAll = true;
        return;
    }

    switch (key) {
    case InputKey::Backspace: queue.backspace = true; break;
    case InputKey::Delete: queue.del = true; break;
    case InputKey::Enter: queue.enter = true; break;
    case InputKey::Left: queue.left = true; break;
    case InputKey::Right: queue.right = true; break;
    case InputKey::Up: queue.up = true; break;
    case InputKey::Down: queue.down = true; break;
    case InputKey::Home: queue.home = true; break;
    case InputKey::End: queue.end = true; break;
    case InputKey::Escape: queue.escape = true; break;
    default: break;
    }
}

inline void installInputCallbacks(window::Handle window) {
    if (!window) {
        return;
    }

#if !defined(EUI_WINDOW_BACKEND_SDL2)
    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window);
    glfwSetCharCallback(glfwWindow, [](GLFWwindow* currentWindow, unsigned int codepoint) {
        detail::appendUtf8(detail::inputQueue(currentWindow).text, codepoint);
    });

    glfwSetScrollCallback(glfwWindow, [](GLFWwindow* currentWindow, double xoffset, double yoffset) {
        queueScrollInput(currentWindow, xoffset, yoffset);
    });

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* currentWindow, int key, int, int action, int mods) {
        if (action != GLFW_PRESS && action != GLFW_REPEAT) {
            return;
        }

        const bool ctrl = (mods & GLFW_MOD_CONTROL) != 0 || (mods & GLFW_MOD_SUPER) != 0;
        const bool shift = (mods & GLFW_MOD_SHIFT) != 0;
        switch (key) {
        case GLFW_KEY_BACKSPACE: queueKeyInput(currentWindow, InputKey::Backspace, ctrl, shift); break;
        case GLFW_KEY_DELETE: queueKeyInput(currentWindow, InputKey::Delete, ctrl, shift); break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER: queueKeyInput(currentWindow, InputKey::Enter, ctrl, shift); break;
        case GLFW_KEY_LEFT: queueKeyInput(currentWindow, InputKey::Left, ctrl, shift); break;
        case GLFW_KEY_RIGHT: queueKeyInput(currentWindow, InputKey::Right, ctrl, shift); break;
        case GLFW_KEY_UP: queueKeyInput(currentWindow, InputKey::Up, ctrl, shift); break;
        case GLFW_KEY_DOWN: queueKeyInput(currentWindow, InputKey::Down, ctrl, shift); break;
        case GLFW_KEY_HOME: queueKeyInput(currentWindow, InputKey::Home, ctrl, shift); break;
        case GLFW_KEY_END: queueKeyInput(currentWindow, InputKey::End, ctrl, shift); break;
        case GLFW_KEY_ESCAPE: queueKeyInput(currentWindow, InputKey::Escape, ctrl, shift); break;
        case GLFW_KEY_A: queueKeyInput(currentWindow, InputKey::A, ctrl, shift); break;
        case GLFW_KEY_C: queueKeyInput(currentWindow, InputKey::C, ctrl, shift); break;
        case GLFW_KEY_V: queueKeyInput(currentWindow, InputKey::V, ctrl, shift); break;
        case GLFW_KEY_X: queueKeyInput(currentWindow, InputKey::X, ctrl, shift); break;
        case GLFW_KEY_Y: queueKeyInput(currentWindow, InputKey::Y, ctrl, shift); break;
        case GLFW_KEY_Z: queueKeyInput(currentWindow, InputKey::Z, ctrl, shift); break;
        default:
            break;
        }
    });
#endif
}

inline std::pair<KeyboardEvent, ScrollEvent> consumeInputEvents(window::Handle window) {
    detail::InputQueue& queue = detail::inputQueue(window);
    KeyboardEvent keyboard;
    keyboard.text = std::move(queue.text);
    keyboard.pasteText = std::move(queue.pasteText);
    keyboard.backspace = queue.backspace;
    keyboard.del = queue.del;
    keyboard.enter = queue.enter;
    keyboard.left = queue.left;
    keyboard.right = queue.right;
    keyboard.up = queue.up;
    keyboard.down = queue.down;
    keyboard.home = queue.home;
    keyboard.end = queue.end;
    keyboard.selectAll = queue.selectAll;
    keyboard.copy = queue.copy;
    keyboard.cut = queue.cut;
    keyboard.undo = queue.undo;
    keyboard.redo = queue.redo;
    keyboard.shift = queue.shift;
    keyboard.escape = queue.escape;

    ScrollEvent scroll{queue.scrollX, queue.scrollY};
    queue = {};
    return {std::move(keyboard), scroll};
}

inline void releaseInputQueue(window::Handle window) {
    detail::inputQueues().erase(window);
    detail::pointerStates().erase(window);
}

struct InteractionState {
    bool hover = false;
    bool pressed = false;
    bool clicked = false;
    bool pressStarted = false;
    bool released = false;
    bool drag = false;
    bool active = false;
    bool changed = false;
    double dragStartX = 0.0;
    double dragStartY = 0.0;
    double dragDeltaX = 0.0;
    double dragDeltaY = 0.0;

    void update(const Rect& bounds, const PointerEvent& event, bool topmostHover, bool enabled = true) {
        const bool oldHover = hover;
        const bool oldPressed = pressed;
        const bool oldDrag = drag;
        const bool oldActive = active;

        clicked = false;
        pressStarted = false;
        released = false;

        if (!enabled) {
            hover = false;
            pressed = false;
            drag = false;
            active = false;
            dragDeltaX = 0.0;
            dragDeltaY = 0.0;
            changed = oldHover != hover || oldPressed != pressed || oldDrag != drag || oldActive != active;
            return;
        }

        hover = topmostHover && bounds.contains(event.x, event.y);

        if (hover && event.pressedThisFrame) {
            active = true;
            pressStarted = true;
            dragStartX = event.x;
            dragStartY = event.y;
        }

        pressed = active && event.down;

        dragDeltaX = event.x - dragStartX;
        dragDeltaY = event.y - dragStartY;
        drag = pressed && (std::fabs(dragDeltaX) > 2.0 || std::fabs(dragDeltaY) > 2.0);

        if (event.releasedThisFrame) {
            released = active;
            clicked = active && hover;
            active = false;
            pressed = false;
            drag = false;
        }

        changed = oldHover != hover ||
                  oldPressed != pressed ||
                  oldDrag != drag ||
                  oldActive != active ||
                  pressStarted ||
                  released ||
                  clicked;
    }
};

inline PointerEvent readPointerEvent(window::Handle window, float dpiScale = 1.0f) {
    detail::PointerState& state = detail::pointerState(window);

    double x = 0.0;
    double y = 0.0;
    core::window::getCursorPosition(window, x, y);
    x *= dpiScale;
    y *= dpiScale;

    PointerEvent event;
    event.x = x;
    event.y = y;
    event.deltaX = x - state.lastX;
    event.deltaY = y - state.lastY;
    event.down = core::window::isMouseButtonDown(window, 0);
    event.rightDown = core::window::isMouseButtonDown(window, 1);
    event.pressedThisFrame = event.down && !state.lastDown;
    event.releasedThisFrame = !event.down && state.lastDown;
    event.rightPressedThisFrame = event.rightDown && !state.lastRightDown;
    event.rightReleasedThisFrame = !event.rightDown && state.lastRightDown;

    state.lastX = x;
    state.lastY = y;
    state.lastDown = event.down;
    state.lastRightDown = event.rightDown;
    return event;
}

} // namespace core
