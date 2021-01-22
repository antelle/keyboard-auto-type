#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>

#include <iostream>

#undef None

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
    Display *display = XOpenDisplay(0);

  public:
    ~AutoTypeImpl() {
        if (display) {
            XCloseDisplay(display);
        }
    }

    AutoTypeResult key_move(Direction direction, os_key_code_t code) {
        if (!code) {
            return throw_or_return(AutoTypeResult::BadArg, "Empty key code");
        }
        if (!display) {
            return throw_or_return(AutoTypeResult::OsError, "Cannot open display");
        }
        auto keycode = XKeysymToKeycode(display, code);
        if (!keycode) {
            return throw_or_return(AutoTypeResult::BadArg,
                                   std::string("Bad key code: ") + std::to_string(code));
        }
        auto down = direction == Direction::Down;
        std::cout << "M" << (down ? '+' : '-') << " " << code << " => " << static_cast<int>(keycode)
                  << std::endl;
        // TODOL query if the extension is available
        auto res = XTestFakeKeyEvent(display, keycode, down, CurrentTime);
        if (!res) {
            return throw_or_return(AutoTypeResult::OsError, "Failed to send an event");
        }
        return AutoTypeResult::Ok;
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier modifier) {
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

pid_t AutoType::active_pid() { return 0; }

AppWindow AutoType::active_window(const ActiveWindowArgs &args) {
    return {};
    // XGetInputFocus(display, &window, &revert);
}

bool AutoType::show_window(const AppWindow &window) {
    if (!window.pid) {
        return false;
    }
    return false;
}

} // namespace keyboard_auto_type
