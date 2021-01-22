#pragma once

#include <X11/Xlib.h>

#include <string>
#include <string_view>

#include "x11-helpers.h"

namespace keyboard_auto_type {

std::string x11_window_string_prop(Display *display, Window window, std::string_view prop);

} // namespace keyboard_auto_type
