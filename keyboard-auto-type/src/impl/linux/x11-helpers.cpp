#include "x11-helpers.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>

namespace keyboard_auto_type {

struct X11WindowProp {
    void *value = nullptr;
    unsigned long nitems = 0;      // NOLINT (google-runtime-int)
    unsigned long bytes_after = 0; // NOLINT (google-runtime-int)
    int format = 0;
    Atom prop_type = 0;
};

X11WindowProp x11_window_prop(Display *display, Window window, const char *prop,
                              Atom expected_type = AnyPropertyType) {
    auto atom_prop = XInternAtom(display, prop, False);
    if (!atom_prop) {
        return {};
    }

    X11WindowProp res{};

    auto err = XGetWindowProperty(display, window, atom_prop, 0, -1, False, expected_type,
                                  &res.prop_type, &res.format, &res.nitems, &res.bytes_after,
                                  reinterpret_cast<unsigned char **>(&res.value));

    if (err != Success || !res.nitems || !res.value) {
        return {};
    }

    return res;
}

std::string x11_window_prop_string(Display *display, Window window, const char *prop) {
    auto xprop = x11_window_prop(display, window, prop);

    std::string value;

    static constexpr std::array STRING_PROP_TYPES{"STRING", "UTF8_STRING"};

    if (xprop.value) {
        auto *type = XGetAtomName(display, xprop.prop_type);
        if (std::find(STRING_PROP_TYPES.begin(), STRING_PROP_TYPES.end(), std::string_view(type)) !=
            STRING_PROP_TYPES.end()) {
            value = reinterpret_cast<char *>(xprop.value);
        }
        XFree(xprop.value);
        XFree(type);
    }

    return value;
}

uint64_t x11_window_prop_ulong(Display *display, Window window, const char *prop) {
    auto prop_type_cardinal = XInternAtom(display, "CARDINAL", False);
    auto xprop = x11_window_prop(display, window, prop, prop_type_cardinal);

    uint64_t value = 0;
    if (xprop.value) {
        value = *reinterpret_cast<unsigned long *>(xprop.value); // NOLINT (google-runtime-int)
        XFree(xprop.value);
    }

    return value;
}

std::string x11_window_prop_app_cls(Display *display, Window window) {
    XClassHint cls{};
    if (!XGetClassHint(display, window, &cls)) {
        return "";
    }
    if (cls.res_name) {
        XFree(cls.res_name);
    }
    std::string name;
    if (cls.res_class) {
        name = cls.res_class;
        XFree(cls.res_class);
    }
    return name;
}

Window x11_get_active_window(Display *display) {
    auto prop_type_window = XInternAtom(display, "WINDOW", False);
    if (!prop_type_window) {
        return 0;
    }

    auto root = XDefaultRootWindow(display);
    auto xprop = x11_window_prop(display, root, "_NET_ACTIVE_WINDOW", prop_type_window);

    Window window = 0;
    if (xprop.value) {
        window = *reinterpret_cast<Window *>(xprop.value);
        XFree(xprop.value);
    }

    return window;
}

} // namespace keyboard_auto_type
