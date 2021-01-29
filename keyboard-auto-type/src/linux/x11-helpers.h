#pragma once

#include <X11/Xlib.h>

#include <cstdint>
#include <string>
#include <string_view>

#include "x11-helpers.h"

namespace keyboard_auto_type {

int x11_error_handler(Display *display, XErrorEvent *event);
std::string x11_window_prop_string(Display *display, Window window, const char *prop);
uint64_t x11_window_prop_ulong(Display *display, Window window, const char *prop);
std::string x11_window_prop_app_cls(Display *display, Window window);
Window x11_get_active_window(Display *display);
bool x11_send_client_message(Display *display, Window window, Window send_to_window,
                             const char *type, uint64_t lparam);

} // namespace keyboard_auto_type
