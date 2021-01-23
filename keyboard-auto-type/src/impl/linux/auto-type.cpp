#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <algorithm>
#include <iostream>

#undef None

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"
#include "x11-helpers.h"
#include "x11-keysym-map.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
    struct KeyCodeWithShiftLevel {
        int key_code;
        int shift_level;
    };

    Display *display_ = nullptr;
    std::optional<bool> is_supported_;
    std::optional<uint8_t> active_layout_;
    std::unordered_map<KeySym, KeyCodeWithShiftLevel> keyboard_layout_ = {};

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

    bool is_supported() {
        if (display()) {
            int i = 0;
            is_supported_ = XTestQueryExtension(display(), &i, &i, &i, &i) &&
                            XkbQueryExtension(display(), &i, &i, &i, &i, &i);
        } else {
            is_supported_ = false;
        }
        return is_supported_.value();
    }

    AutoTypeResult key_move(Direction direction, os_key_code_t code) {
        if (!code) {
            return throw_or_return(AutoTypeResult::BadArg, "Empty key code");
        }
        if (!is_supported()) {
            return throw_or_return(AutoTypeResult::NotSupported, "Not supported");
        }
        if (!display()) {
            return throw_or_return(AutoTypeResult::OsError, "Cannot open display");
        }

        auto layout_entry = keyboard_layout_.find(code);
        if (layout_entry == keyboard_layout_.end()) {
            return throw_or_return(AutoTypeResult::BadArg,
                                   std::string("Bad key code: ") + std::to_string(code));
        }
        auto key_code_with_shift_level = layout_entry->second;
        auto key_code = key_code_with_shift_level.key_code;

        // TODO: shift level

        auto down = direction == Direction::Down;
        auto res = XTestFakeKeyEvent(display(), key_code, down, CurrentTime);
        if (!res) {
            return throw_or_return(AutoTypeResult::OsError, "Failed to send an event");
        }
        return AutoTypeResult::Ok;
    }

    void read_keyboard_layout() {
        if (!display()) {
            return;
        }

        XkbStateRec kbd_state;
        if (XkbGetState(display(), XkbUseCoreKbd, &kbd_state)) {
            return;
        }
        auto active_layout = kbd_state.group;
        if (active_layout == active_layout_) {
            return;
        }

        keyboard_layout_.clear();

        auto kbd_components = XkbCompatMapMask | XkbGeometryMask; // XkbAllComponentsMask
        auto kbd = XkbGetKeyboard(display(), kbd_components, XkbUseCoreKbd);
        if (!kbd) {
            return;
        }

        for (auto key_code = kbd->min_key_code; key_code < kbd->max_key_code; key_code++) {
            auto shift_levels_count = XkbKeyGroupWidth(kbd, key_code, kbd_state.group);
            auto max_shift_level = std::min(static_cast<uint8_t>(1U), shift_levels_count);
            for (auto shift_level = 0; shift_level <= max_shift_level; shift_level++) {
                auto sym = XkbKeySymEntry(kbd, key_code, shift_level, kbd_state.group);
                if (sym && !keyboard_layout_.count(sym)) {
                    KeyCodeWithShiftLevel key_code_with_shift_level{};
                    key_code_with_shift_level.key_code = key_code;
                    key_code_with_shift_level.shift_level = shift_level;
                    keyboard_layout_.emplace(sym, key_code_with_shift_level);
                }
            }
        }

        active_layout_ = active_layout;

        XkbFreeKeyboard(kbd, kbd_components, True);
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier) {
    if (!code.has_value()) {
        auto msg = std::string("Character ") + std::to_string(static_cast<uint32_t>(character)) +
                   " not supported";
        return throw_or_return(AutoTypeResult::BadArg, msg);
    }
    impl_->read_keyboard_layout(); // TODO: optimize for batch input
    return impl_->key_move(direction, code.value());
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
    impl_->read_keyboard_layout();
    auto keysym = char_to_keysym(character);
    if (!keysym) {
        return std::nullopt;
    }
    KeyCodeWithModifiers kc{};
    kc.code = keysym;
    return kc;
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    impl_->read_keyboard_layout();
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (size_t i = 0; i < length; i++) {
        auto keysym = char_to_keysym(text[i]);
        if (keysym) {
            KeyCodeWithModifiers kc{};
            kc.code = keysym;
            result[i] = kc;
        }
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
