#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <iostream>

#undef None

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"
#include "x11-helpers.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
    Display *display_;

  public:
    ~AutoTypeImpl() {
        if (display_) {
            XCloseDisplay(display_);
        }
    }

    Display *display() {
        if (!display_) {
            display_ = XOpenDisplay(0);
        }
        return display_;
    }

    AutoTypeResult key_move(Direction direction, os_key_code_t code) {
        if (!code) {
            return throw_or_return(AutoTypeResult::BadArg, "Empty key code");
        }
        if (!display_) {
            return throw_or_return(AutoTypeResult::OsError, "Cannot open display");
        }
        auto keycode = XKeysymToKeycode(display_, code);
        if (!keycode) {
            return throw_or_return(AutoTypeResult::BadArg,
                                   std::string("Bad key code: ") + std::to_string(code));
        }
        auto down = direction == Direction::Down;
        std::cout << "M" << (down ? '+' : '-') << " " << code << " => " << static_cast<int>(keycode)
                  << std::endl;
        // TODOL query if the extension is available
        auto res = XTestFakeKeyEvent(display_, keycode, down, CurrentTime);
        if (!res) {
            return throw_or_return(AutoTypeResult::OsError, "Failed to send an event");
        }
        return AutoTypeResult::Ok;
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t, std::optional<os_key_code_t> code,
                                  Modifier) {
    return impl_->key_move(direction, code.value_or(0));
}

Modifier AutoType::get_pressed_modifiers() {
    auto pressed_modifiers = Modifier::None;
    return pressed_modifiers;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Ctrl; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto native_key_code = map_key_code(code);
    return native_key_code;
}

std::optional<KeyCodeWithModifiers> AutoType::os_key_code_for_char(char32_t character) {
    if (character == U'e') {
        KeyCodeWithModifiers kc{};
        kc.code = map_key_code(KeyCode::E);
        return kc;
    }
    if (character == U'H') {
        KeyCodeWithModifiers kc{};
        kc.code = map_key_code(KeyCode::H);
        kc.modifier = Modifier::Shift;
        return kc;
    }
    if (character == U'y') {
        KeyCodeWithModifiers kc{};
        kc.code = map_key_code(KeyCode::Y);
        return kc;
    }
    return std::nullopt;
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (size_t i = 0; i < length; i++) {
        result[i] = os_key_code_for_char(text[i]);
    }
    return result;
}

pid_t AutoType::active_pid() {
    auto display = impl_->display();
    if (!display) {
        return {};
    }

    Window window = x11_get_active_window(display);
    if (!window) {
        return {};
    }

    return static_cast<pid_t>(x11_window_prop_ulong(display, window, "_NET_WM_PID"));
}

AppWindow AutoType::active_window(ActiveWindowArgs args) {
    auto display = impl_->display();
    if (!display) {
        return {};
    }

    Window window = x11_get_active_window(display);
    if (!window) {
        return {};
    }

    AppWindow result{};
    result.window_id = window;
    result.pid = static_cast<pid_t>(x11_window_prop_ulong(display, window, "_NET_WM_PID"));
    result.app_name = x11_window_prop_app_cls(display, window);

    if (args.get_window_title) {
        result.title = x11_window_prop_string(display, window, "_NET_WM_NAME");
        if (result.title.empty()) {
            result.title = x11_window_prop_string(display, window, "WM_NAME");
        }
    }

    return result;
}

bool AutoType::show_window(const AppWindow &window) {
    if (!window.pid) {
        return false;
    }
    return false;
}

} // namespace keyboard_auto_type
