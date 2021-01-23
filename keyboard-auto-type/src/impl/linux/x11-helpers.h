#pragma once

#include <X11/Xlib.h>

#include <string>
#include <string_view>

#include "x11-helpers.h"

namespace keyboard_auto_type {

std::string x11_window_prop_string(Display *display, Window window, const char *prop);
unsigned long x11_window_prop_ulong(Display *display, Window window, const char *prop);
std::string x11_window_prop_app_cls(Display *display, Window window);
Window x11_get_active_window(Display *display);

} // namespace keyboard_auto_type
