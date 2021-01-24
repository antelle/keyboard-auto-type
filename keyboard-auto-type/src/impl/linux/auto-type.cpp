#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#undef None

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"
#include "x11-helpers.h"
#include "x11-keysym-map.h"

namespace keyboard_auto_type {

struct KeyCodeWithShiftLevel {
    uint8_t key_code = 0;
    uint8_t group = 0;
    uint8_t shift_level = 0;
};

constexpr std::array SHIFT_LEVELS_MODIFIERS = {
    std::make_pair(ShiftMask, Modifier::Shift),
    std::make_pair(LockMask, Modifier::Alt),
    std::make_pair(ControlMask, Modifier::Ctrl),
};

static constexpr auto MAX_SHIFT_LEVELS_COUNT =
    static_cast<uint8_t>((ShiftMask | LockMask | ControlMask) + 1);

class AutoType::AutoTypeImpl {
  private:
    Display *display_ = nullptr;
    std::optional<bool> is_supported_;
    std::optional<uint8_t> active_keyboard_group_;
    std::unordered_map<KeySym, KeyCodeWithShiftLevel> keyboard_layout_ = {};
    uint8_t empty_key_code_ = 0xcc; // for debugging! revert me
    KeySym empty_key_code_key_sym_ = 0;

  public:
    ~AutoTypeImpl() {
        if (display_) {
            remove_extra_key_mapping();
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
        if (is_supported_.has_value()) {
            return is_supported_.value();
        }
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

        read_keyboard_layout();
        if (!active_keyboard_group_.has_value()) {
            return throw_or_return(AutoTypeResult::OsError, "Keyboard layout was not read");
        }

        auto layout_entry = keyboard_layout_.find(code);
        KeyCodeWithShiftLevel key{};
        if (layout_entry == keyboard_layout_.end()) {
            if (XKeysymToString(code)) {
                key.key_code = add_extra_key_mapping(code);
                if (!key.key_code) {
                    return throw_or_return(AutoTypeResult::OsError, "Failed to add key mapping");
                }
            } else {
                return throw_or_return(AutoTypeResult::BadArg,
                                       std::string("Bad key code: ") + std::to_string(code));
            }
        } else {
            key = layout_entry->second;
        }

        auto down = direction == Direction::Down;

        if (key.group != active_keyboard_group_.value()) {
            if (!XkbLockGroup(display(), XkbUseCoreKbd, key.group)) {
                return throw_or_return(AutoTypeResult::OsError, "Failed to change keyboard layout");
            }
        }
        auto res = XTestFakeKeyEvent(display(), key.key_code, down, CurrentTime);
        if (key.group != active_keyboard_group_.value()) {
            if (!XkbLockGroup(display(), XkbUseCoreKbd, active_keyboard_group_.value())) {
                return throw_or_return(AutoTypeResult::OsError,
                                       "Failed to restore keyboard layout");
            }
        }

        if (!res) {
            return throw_or_return(AutoTypeResult::OsError, "Failed to send an event");
        }
        XSync(display(), False);
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
        auto active_group = kbd_state.group;
        if (active_group == active_keyboard_group_) {
            return;
        }

        keyboard_layout_.clear();

        std::ofstream layout_fs;
        layout_fs.open("tmp/layout.txt");

        auto kbd_components = XkbCompatMapMask | XkbGeometryMask;
        if (layout_fs.is_open()) {
            kbd_components |= XkbNamesMask;
        }
        auto kbd = XkbGetKeyboard(display(), kbd_components, XkbUseCoreKbd);
        if (!kbd) {
            return;
        }

        if (layout_fs.is_open()) {
            auto layout_name = XGetAtomName(display(), kbd->names->groups[kbd_state.group]);
            layout_fs << "Layout: " << layout_name << std::endl << std::endl;
            XFree(layout_name);
        }

        for (auto key_code = kbd->min_key_code; key_code < kbd->max_key_code; key_code++) {
            if (key_code == empty_key_code_) {
                continue;
            }
            auto key_groups_num = XkbKeyNumGroups(kbd, key_code);
            layout_fs << "Key 0x" << std::hex << static_cast<int>(key_code) << std::endl;
            layout_fs << "  key_groups_num=" << key_groups_num << std::endl;
            std::vector<uint8_t> key_groups;
            if (active_group < key_groups_num) {
                key_groups.push_back(active_group);
            }
            for (auto gr = 0; gr < key_groups_num; gr++) {
                if (gr != active_group) {
                    key_groups.push_back(gr);
                }
            }
            auto is_empty = true;
            for (auto group : key_groups) {
                auto shift_levels_count = XkbKeyGroupWidth(kbd, key_code, group);
                if (layout_fs.is_open()) {
                    auto kt = XkbKeyKeyType(kbd, key_code, group);
                    layout_fs << "  shift_levels_count=" << static_cast<int>(shift_levels_count)
                              << std::endl
                              << "  name=" << XGetAtomName(display(), kt->name) << std::endl;
                }
                shift_levels_count = std::min(shift_levels_count, MAX_SHIFT_LEVELS_COUNT);
                for (auto shift_level = 0; shift_level < shift_levels_count; shift_level++) {
                    auto sym = XkbKeySymEntry(kbd, key_code, shift_level, group);
                    if (sym && !keyboard_layout_.count(sym)) {
                        KeyCodeWithShiftLevel key_code_with_shift_level{};
                        key_code_with_shift_level.key_code = key_code;
                        key_code_with_shift_level.group = group;
                        key_code_with_shift_level.shift_level = shift_level;
                        keyboard_layout_.emplace(sym, key_code_with_shift_level);
                        layout_fs << "    shift_level=" << shift_level << ": 0x"
                                  << static_cast<int>(sym) << " (" << XKeysymToString(sym) << ")"
                                  << std::endl;
                    }
                    if (sym) {
                        is_empty = false;
                    }
                }
            }
            if (is_empty && !empty_key_code_) {
                empty_key_code_ = key_code;
            }
        }

        active_keyboard_group_ = active_group;

        XkbFreeKeyboard(kbd, kbd_components, True);
    }

    uint8_t add_extra_key_mapping(KeySym key_sym) {
        if (!empty_key_code_) {
            return 0;
        }
        if (empty_key_code_key_sym_ == key_sym) {
            return empty_key_code_;
        }
        if (empty_key_code_key_sym_) {
            wait_for_key_mapping_propagation();
        }
        if (XChangeKeyboardMapping(display(), empty_key_code_, 1, &key_sym, 1)) {
            return 0;
        }
        XSync(display(), False);
        wait_for_key_mapping_propagation();
        empty_key_code_key_sym_ = key_sym;
        return empty_key_code_;
    }

    void remove_extra_key_mapping() {
        if (!empty_key_code_ || !empty_key_code_key_sym_) {
            return;
        }
        wait_for_key_mapping_propagation();
        KeySym key_sym[] = {0};
        XChangeKeyboardMapping(display(), empty_key_code_, 1, key_sym, 1);
        XSync(display(), False);
    }

    void wait_for_key_mapping_propagation() { usleep(300'000); }

    std::optional<KeyCodeWithShiftLevel> key_code_from_layout(KeySym key_sym) {
        auto found = keyboard_layout_.find(key_sym);
        if (found == keyboard_layout_.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::optional<KeyCodeWithModifiers> os_key_code_from_char(char32_t character) {
        auto key_sym = char_to_keysym(character);
        if (!key_sym) {
            return std::nullopt;
        }
        KeyCodeWithModifiers kc{};
        kc.code = key_sym;
        auto layout_key = key_code_from_layout(key_sym);
        auto shift_level = layout_key.has_value() ? layout_key->shift_level : 0;
        if (shift_level) {
            for (auto [mask, modifier] : SHIFT_LEVELS_MODIFIERS) {
                if (shift_level & mask) {
                    kc.modifier = kc.modifier | modifier;
                }
            }
        }
        return kc;
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
    return impl_->os_key_code_from_char(character);
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    impl_->read_keyboard_layout();
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (size_t i = 0; i < length; i++) {
        result[i] = impl_->os_key_code_from_char(text[i]);
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
