#include "x11-helpers.h"

#include <X11/Xlib.h>

#include <iostream>

namespace keyboard_auto_type {

std::string x11_window_string_prop(Display *display, Window window, std::string_view prop) {
    auto atom_prop = XInternAtom(display, prop.data(), False);
    if (!atom_prop) {
        return "";
    }
    Atom prop_type = 0;
    int format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *value_chars;
    auto ret = XGetWindowProperty(display, window, atom_prop, 0, 0, False, AnyPropertyType,
                                  &prop_type, &format, &nitems, &bytes_after, &value_chars);

    if (ret != Success || !value_chars) {
        return "";
    }

    // TODO: check format

    std::string value_str(reinterpret_cast<char *>(value_chars));
    XFree(value_chars);

    return value_str;
}

} // namespace keyboard_auto_type
