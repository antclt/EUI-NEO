#pragma once

struct GLFWwindow;

#ifdef __cplusplus
extern "C" {
#endif

void eui_ime_set_cursor_rect(struct GLFWwindow* window, double x, double y, double width, double height);
int eui_ime_is_composing(struct GLFWwindow* window);

#ifdef __cplusplus
}
#endif
