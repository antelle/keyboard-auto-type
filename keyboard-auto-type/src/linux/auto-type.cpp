#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <algorithm>
#include <chrono>
#include <climits>
#include <iostream>
#include <thread>
#include <vector>

#undef None

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"
#include "x11-helpers.h"
#include "x11-keysym-map.h"

namespace keyboard_auto_type {

struct KeyCodeWithMask {
    uint8_t key_code = 0;
    uint8_t group = 0;
    uint8_t mod_mask = 0;
};

constexpr std::array OS_KEY_CODE_SUPPORTED_MODIFIERS_MASKS{
    std::make_pair(ShiftMask, Modifier::Shift),
};

constexpr std::array KEY_CODES_MODIFIERS{
    std::make_pair(XK_Shift_L, Modifier::LeftShift),
    std::make_pair(XK_Shift_R, Modifier::RightShift),
    std::make_pair(XK_Control_L, Modifier::LeftCtrl),
    std::make_pair(XK_Control_R, Modifier::RightCtrl),
    std::make_pair(XK_Alt_L, Modifier::LeftAlt),
    std::make_pair(XK_Alt_R, Modifier::RightAlt),
    std::make_pair(XK_Super_L, Modifier::LeftMeta),
    std::make_pair(XK_Super_R, Modifier::RightMeta),
};

static constexpr auto KEY_MAPPING_PROPAGATION_DELAY = std::chrono::milliseconds(200);

static constexpr auto MAX_KEYSYM = 0x0110FFFFU;

static constexpr uint8_t EMPTY_KEY_CODE_FOR_DEBUGGING = 0xcc;

class AutoType::AutoTypeImpl {
  private:
    Display *display_ = nullptr;
    std::optional<bool> is_supported_;
    std::optional<uint8_t> active_keyboard_group_; // aka "layout" or "input language"
    std::unordered_map<KeySym, KeyCodeWithMask> keyboard_layout_ = {};
    uint8_t empty_key_code_ = EMPTY_KEY_CODE_FOR_DEBUGGING;
    KeySym empty_key_code_key_sym_ = 0;
    bool in_batch_text_entry_ = false;

  public:
    AutoTypeImpl() = default;
    AutoTypeImpl(const AutoTypeImpl &) = delete;
    AutoTypeImpl &operator=(const AutoTypeImpl &) = delete;
    AutoTypeImpl(AutoTypeImpl &&) = delete;
    AutoTypeImpl &operator=(AutoTypeImpl &&) = delete;

    ~AutoTypeImpl() {
        if (display_) {
            remove_extra_key_mapping();
            XCloseDisplay(display_);
        }
    }

    Display *display() {
        if (!display_) {
            display_ = XOpenDisplay(nullptr);
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
        if (!display()) {
            return throw_or_return(AutoTypeResult::OsError, "Cannot open display");
        }
        if (!is_supported()) {
            return throw_or_return(AutoTypeResult::NotSupported, "Not supported");
        }

        if (!in_batch_text_entry_ || !active_keyboard_group_.has_value()) {
            read_keyboard_layout();
        }
        if (!active_keyboard_group_.has_value()) {
            return throw_or_return(AutoTypeResult::OsError, "Keyboard layout was not read");
        }

        auto layout_entry = keyboard_layout_.find(code);
        KeyCodeWithMask key{};
        KeySym key_sym_lower = 0;
        KeySym key_sym_upper = 0;
        XConvertCase(code, &key_sym_lower, &key_sym_upper);
        if (layout_entry == keyboard_layout_.end()) {
            if (is_valid_key_sym(code)) {
                key = add_extra_key_mapping(code);
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
        XkbStateRec kbd_state{};
        if (key.mod_mask) {
            if (XkbGetState(display(), XkbUseCoreKbd, &kbd_state)) {
                return throw_or_return(AutoTypeResult::OsError, "Failed to get modifiers state");
            }
            if (kbd_state.locked_mods != key.mod_mask) {
                if (!XkbLockModifiers(display(), XkbUseCoreKbd, key.mod_mask, key.mod_mask)) {
                    return throw_or_return(AutoTypeResult::OsError, "Failed to lock modifiers");
                }
            }
        }

        if (!XTestFakeKeyEvent(display(), key.key_code, down, CurrentTime)) {
            return throw_or_return(AutoTypeResult::OsError, "Failed to send key event");
        }

        if (key.mod_mask && kbd_state.locked_mods != key.mod_mask) {
            if (!XkbLockModifiers(display(), XkbUseCoreKbd, key.mod_mask, kbd_state.locked_mods)) {
                return throw_or_return(AutoTypeResult::OsError, "Failed to unlock modifiers");
            }
        }
        if (key.group != active_keyboard_group_.value()) {
            if (!XkbLockGroup(display(), XkbUseCoreKbd, active_keyboard_group_.value())) {
                return throw_or_return(AutoTypeResult::OsError,
                                       "Failed to restore keyboard layout");
            }
        }

        XSync(display(), False);
        return AutoTypeResult::Ok;
    }

    void read_keyboard_layout() {
        if (!is_supported()) {
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

        auto kbd_components = XkbCompatMapMask | XkbGeometryMask;
        auto *kbd = XkbGetKeyboard(display(), kbd_components, XkbUseCoreKbd);
        if (!kbd) {
            return;
        }

        for (uint16_t key_code = kbd->min_key_code; key_code <= kbd->max_key_code; key_code++) {
            if (key_code == empty_key_code_) {
                continue;
            }
            auto key_groups_num = XkbKeyNumGroups(kbd, key_code);
            auto is_empty = true;
            for (auto group = 0; group < key_groups_num; group++) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                auto shift_levels_count = XkbKeyGroupWidth(kbd, key_code, group);
                if (shift_levels_count > 1) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    auto *key_type = XkbKeyKeyType(kbd, key_code, group);
                    if (key_type->map_count > 0 && key_type->map[0].mods.mask == ShiftMask) {
                        // for most of keys we consider two states
                        shift_levels_count = 2;
                    } else {
                        // all other modifiers are ignored for now
                        shift_levels_count = 1;
                    }
                }
                for (auto shift_level = 0; shift_level < shift_levels_count; shift_level++) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    auto sym = XkbKeySymEntry(kbd, key_code, shift_level, group);
                    if (sym) {
                        auto existing_mapping = keyboard_layout_.find(sym);
                        if (existing_mapping != keyboard_layout_.end()) {
                            // active group always has priority
                            // so that we press "heZ" in German layout to get "heY"
                            if (group != active_group) {
                                continue;
                            }
                            // inside the active group, the first key has priority
                            if (existing_mapping->second.group == active_group) {
                                continue;
                            }
                        }
                        KeyCodeWithMask kc{};
                        kc.key_code = key_code;
                        kc.group = group;
                        if (shift_level > 0) {
                            kc.mod_mask = ShiftMask;
                        }
                        keyboard_layout_.insert_or_assign(sym, kc);
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

    KeyCodeWithMask add_extra_key_mapping(KeySym key_sym) {
        if (!empty_key_code_) {
            return {};
        }
        if (empty_key_code_key_sym_ != key_sym) {
            if (empty_key_code_key_sym_) {
                wait_for_key_mapping_propagation();
            }
            if (XChangeKeyboardMapping(display(), empty_key_code_, 1, &key_sym, 1)) {
                return {};
            }
            XSync(display(), False);
            wait_for_key_mapping_propagation();
        }

        KeyCodeWithMask key{};
        key.key_code = empty_key_code_;

        KeySym key_sym_lower = 0;
        KeySym key_sym_upper = 0;
        XConvertCase(key_sym, &key_sym_lower, &key_sym_upper);

        if (key_sym_lower != key_sym && key_sym_upper == key_sym) {
            key.mod_mask = ShiftMask;
        }

        empty_key_code_key_sym_ = key_sym;

        return key;
    }

    void remove_extra_key_mapping() {
        if (!empty_key_code_ || !empty_key_code_key_sym_) {
            return;
        }

        // The key has been just used, let the app process it
        wait_for_key_mapping_propagation();

        KeySym key_sym = 0;
        XChangeKeyboardMapping(display(), empty_key_code_, 1, &key_sym, 1);
        XSync(display(), False);

        empty_key_code_key_sym_ = 0;
    }

    void wait_for_key_mapping_propagation() {
        // This is called:
        // 1. between adding a new key mapping and its usage
        // 2. after using and before removing a key mapping
        // We need this delay to make sure the target app has processesed the remapping event
        std::this_thread::sleep_for(KEY_MAPPING_PROPAGATION_DELAY);
    }

    std::optional<KeyCodeWithMask> key_code_from_layout(KeySym key_sym) {
        auto found = keyboard_layout_.find(key_sym);
        if (found == keyboard_layout_.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::optional<KeyCodeWithModifiers> os_key_code_from_char(char32_t character) {
        auto key_sym = char_to_keysym(character);
        if (!key_sym || !is_valid_key_sym(key_sym)) {
            return std::nullopt;
        }
        KeyCodeWithModifiers kc{};
        kc.code = key_sym;
        auto layout_key = key_code_from_layout(key_sym);
        auto mod_mask = layout_key.has_value() ? layout_key->mod_mask : 0;
        if (mod_mask) {
            for (auto [mask, modifier] : OS_KEY_CODE_SUPPORTED_MODIFIERS_MASKS) {
                if (mod_mask & mask) {
                    kc.modifier = kc.modifier | modifier;
                }
            }
        }
        return kc;
    }

    bool is_valid_key_sym(KeySym key_sym) {
        // this can be better checked with XKeysymToString but the leak detector says
        // it still leaks if it's called with valid Unocide code points not defined as KeySym
        // see https://bugs.freedesktop.org/show_bug.cgi?id=7100
        return key_sym > 0 && key_sym <= MAX_KEYSYM;
    }

    [[nodiscard]] AutoTypeTextTransaction begin_batch_text_entry() {
        if (in_batch_text_entry_) {
            // for convenience, allow nested transactions, but don't do anything
            return AutoTypeTextTransaction();
        }
        in_batch_text_entry_ = true;
        return AutoTypeTextTransaction([this] {
            in_batch_text_entry_ = false;
            remove_extra_key_mapping();
        });
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier /*unused*/) {
    if (!code.has_value()) {
        auto msg = std::string("Character ") + std::to_string(static_cast<uint32_t>(character)) +
                   " not supported";
        return throw_or_return(AutoTypeResult::BadArg, msg);
    }
    return impl_->key_move(direction, code.value());
}

Modifier AutoType::get_pressed_modifiers() {
    auto *display = impl_->display();
    if (!display) {
        return Modifier::None;
    }

    static constexpr auto KEYMAP_SIZE = 32;

    std::array<char, KEYMAP_SIZE> keymap{};
    if (!XQueryKeymap(display, keymap.data())) {
        return Modifier::None;
    }

    auto pressed_modifiers = Modifier::None;

    for (auto [key_sym, modifier] : KEY_CODES_MODIFIERS) {
        auto key_code = XKeysymToKeycode(display, key_sym);
        if (!key_code) {
            continue;
        }

        auto key_offsets = std::div(key_code, CHAR_BIT);
        auto key_ix_in_keymap = key_offsets.quot;
        auto key_bit_offset = key_offsets.rem;
        auto key_bit_mask = 1 << key_bit_offset;
        if (keymap.at(key_ix_in_keymap) & key_bit_mask) {
            pressed_modifiers = pressed_modifiers | modifier;
        }
    }

    return pressed_modifiers;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Ctrl; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto key_sym = map_key_code(code);
    if (!key_sym) {
        return std::nullopt;
    }
    return key_sym;
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
    auto *display = impl_->display();
    if (!display) {
        return 0;
    }

    auto prev_error_handler = XSetErrorHandler(x11_error_handler);

    pid_t pid = 0;
    Window window = x11_get_active_window(display);
    if (window) {
        pid = static_cast<pid_t>(x11_window_prop_ulong(display, window, "_NET_WM_PID"));
    }

    XSetErrorHandler(prev_error_handler);

    return pid;
}

AppWindow AutoType::active_window(ActiveWindowArgs args) {
    auto *display = impl_->display();
    if (!display) {
        return {};
    }

    auto prev_error_handler = XSetErrorHandler(x11_error_handler);

    Window window = x11_get_active_window(display);
    if (!window) {
        XSetErrorHandler(prev_error_handler);
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

    XSetErrorHandler(prev_error_handler);

    return result;
}

bool AutoType::show_window(const AppWindow &window) {
    if (!window.pid || !window.window_id) {
        return false;
    }
    auto *display = impl_->display();
    if (!display) {
        return false;
    }

    auto prev_error_handler = XSetErrorHandler(x11_error_handler);

    XWindowAttributes window_attr{};
    if (!XGetWindowAttributes(display, window.window_id, &window_attr)) {
        XSetErrorHandler(prev_error_handler);
        return false;
    }
    auto root = window_attr.root;

    auto window_desktop = x11_window_prop_ulong(display, window.window_id, "_NET_WM_DESKTOP");
    auto current_desktop = x11_window_prop_ulong(display, root, "_NET_CURRENT_DESKTOP");
    if (window_desktop != current_desktop) {
        if (!x11_send_client_message(display, root, root, "_NET_CURRENT_DESKTOP", window_desktop)) {
            XSetErrorHandler(prev_error_handler);
            return false;
        }
    }

    constexpr auto WINDOW_MESSAGE_FROM_WINDOW_PAGER = 2;
    if (!x11_send_client_message(display, window.window_id, root, "_NET_ACTIVE_WINDOW",
                                 WINDOW_MESSAGE_FROM_WINDOW_PAGER)) {
        XSetErrorHandler(prev_error_handler);
        return false;
    }

    XSync(display, False);

    XMapRaised(display, window.window_id);
    XSetInputFocus(display, window.window_id, RevertToParent, CurrentTime);

    XSync(display, False);

    XSetErrorHandler(prev_error_handler);

    return true;
}

AutoTypeTextTransaction AutoType::begin_batch_text_entry() {
    return impl_->begin_batch_text_entry();
}

} // namespace keyboard_auto_type
