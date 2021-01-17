#include <Carbon/Carbon.h>

#include <array>
#include <codecvt>
#include <locale>

#include "auto-release.h"
#include "carbon-helpers.h"
#include "key-map.h"
#include "keyboard-auto-type.h"
#include "native-methods.h"

namespace keyboard_auto_type {

constexpr std::array BROWSER_APP_BUNDLE_IDS{
    "com.google.Chrome",
    "org.chromium.Chromium",
    "com.google.Chrome.canary",
    "com.apple.Safari",
    "com.apple.SafariTechnologyPreview",
};

class AutoType::AutoTypeImpl {
  private:
    auto_release<CGEventSourceRef> event_source_ =
        CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    static constexpr int MAX_KEYBOARD_LAYOUT_CHAR_CODE = 128;
    static constexpr int KEYBOARD_LAYOUT_LENGTH = 128;
    std::array<CGKeyCode, KEYBOARD_LAYOUT_LENGTH> keyboard_layout_ = {{}};
    CFDataRef keyboard_layout_data_ = nullptr;

  public:
    AutoTypeResult key_move(char32_t character, CGKeyCode code, CGEventFlags flags,
                            Direction direction) {
        auto down = direction == Direction::Down;
        auto_release event = CGEventCreateKeyboardEvent(event_source_, code, down);
        if (character) {
            set_event_char(event, character);
        }
        if (flags) {
            CGEventSetFlags(event, flags);
        }
        CGEventPost(kCGHIDEventTap, event);

        return AutoTypeResult::Ok;
    }

    static void set_event_char(CGEventRef event, char32_t character) {
        std::u32string char_str(1, character);
        std::wstring_convert<std::codecvt_utf16<char32_t, UINT32_MAX, std::little_endian>, char32_t>
            converter;
        auto utf16 = converter.to_bytes(char_str.c_str());
        const auto *utf16_cstr = reinterpret_cast<const UniChar *>(utf16.c_str());
        auto utf16_length = utf16.length() / sizeof(UniChar);
        CGEventKeyboardSetUnicodeString(event, utf16_length, utf16_cstr);
    }

    void read_keyboard_layout() {
        // https://stackoverflow.com/questions/1918841/how-to-convert-ascii-character-to-cgkeycode
        // https://stackoverflow.com/questions/8263618/convert-virtual-key-code-to-unicode-string

        auto_release keyboard = TISCopyCurrentKeyboardInputSource();
        const auto *layout_data = static_cast<CFDataRef>(
            TISGetInputSourceProperty(keyboard, kTISPropertyUnicodeKeyLayoutData));

        if (layout_data == keyboard_layout_data_) {
            return;
        }

        const auto *layout =
            reinterpret_cast<const UCKeyboardLayout *>(CFDataGetBytePtr(layout_data));
        auto kbd_type = LMGetKbdType();

        uint32_t keys_down = 0;
        std::array<UniChar, 4> chars{};
        UniCharCount length = 0;

        for (int i = 0; i < KEYBOARD_LAYOUT_LENGTH; i++) {
            keyboard_layout_.at(i) = 0;
        }
        static constexpr std::array MOD_STATES{0U, static_cast<UInt32>(NX_DEVICELSHIFTKEYMASK)};
        for (int code = 0; code < MAX_KEYBOARD_LAYOUT_CHAR_CODE; code++) {
            for (auto mod_state : MOD_STATES) {
                UCKeyTranslate(layout, code, kUCKeyActionDown, mod_state, kbd_type,
                               kUCKeyTranslateNoDeadKeysBit, &keys_down, 4, &length, chars.data());
                auto ch = chars[0];

                if (length && (ch >= ' ' && ch <= '~' || ch == '\t' || ch == ' ') &&
                    !keyboard_layout_.at(ch)) {
                    keyboard_layout_.at(ch) = code;
                }
            }
        }

        keyboard_layout_data_ = layout_data;
    }

    std::optional<KeyCodeWithModifiers> char_to_key_code(char32_t character) {
        if (character >= keyboard_layout_.size()) {
            return std::nullopt;
        }
        auto code = keyboard_layout_.at(character);
        KeyCodeWithModifiers code_with_modifiers{};
        code_with_modifiers.code = code;
        // TODO: code_with_modifiers.modifier = ...
        return code_with_modifiers;
    }

    static CGEventFlags modifier_to_flags(Modifier modifier) {
        if (modifier == Modifier::None) {
            return 0;
        }

        CGEventFlags flags = 0;
        if ((modifier & Modifier::Command) == Modifier::Command) {
            flags |= kCGEventFlagMaskCommand;
        }
        if ((modifier & Modifier::Option) == Modifier::Option) {
            flags |= kCGEventFlagMaskAlternate;
        }
        if ((modifier & Modifier::Control) == Modifier::Control) {
            flags |= kCGEventFlagMaskControl;
        }
        if ((modifier & Modifier::Shift) == Modifier::Shift) {
            flags |= kCGEventFlagMaskShift;
        }
        return flags;
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier modifier) {
    auto flags = AutoTypeImpl::modifier_to_flags(modifier);
    return impl_->key_move(character, code.value_or(0), flags, direction);
}

Modifier AutoType::get_pressed_modifiers() {
    static constexpr std::array FLAGS_MODIFIERS{
        std::make_pair(kCGEventFlagMaskCommand, Modifier::Command),
        std::make_pair(kCGEventFlagMaskShift, Modifier::Shift),
        std::make_pair(kCGEventFlagMaskAlternate, Modifier::Option),
        std::make_pair(kCGEventFlagMaskControl, Modifier::Control),
    };
    auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
    auto pressed_modifiers = Modifier::None;
    for (auto [flag, modifier] : FLAGS_MODIFIERS) {
        if (flags & flag) {
            pressed_modifiers = pressed_modifiers | modifier;
        }
    }
    return pressed_modifiers;
}

bool AutoType::can_unpress_modifier() { return true; }

Modifier AutoType::shortcut_modifier() { return Modifier::Command; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto native_key_code = map_key_code(code);
    if (!native_key_code && code != KeyCode::A) { // A maps to kVK_ANSI_A = 0
        return std::nullopt;
    }
    return native_key_code;
}

std::optional<KeyCodeWithModifiers> AutoType::os_key_code_for_char(char32_t character) {
    impl_->read_keyboard_layout();
    return impl_->char_to_key_code(character);
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    impl_->read_keyboard_layout();
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (auto i = 0; i < length; i++) {
        result[i] = impl_->char_to_key_code(text[i]);
    }
    return result;
}

pid_t AutoType::active_pid() { return native_frontmost_app_pid(); }

AppWindowInfo AutoType::active_window(const ActiveWindowArgs &args) {
    auto app = native_frontmost_app();
    if (!app.pid) {
        return {};
    }

    auto_release windows = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    if (!windows) {
        return {};
    }

    AppWindowInfo result = {};
    result.pid = app.pid;
    result.app_name = app.name;

    auto count = CFArrayGetCount(windows);
    for (auto i = 0; i < count; i++) {
        // std::cout << cfstring_to_string(CFCopyDescription(window)) << std::endl;
        const auto *window = reinterpret_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows, i));
        auto window_pid = get_number_from_dictionary(window, kCGWindowOwnerPID);
        if (window_pid != app.pid) {
            continue;
        }
        auto window_layer = get_number_from_dictionary(window, kCGWindowLayer);
        if (window_layer != 0) {
            continue;
        }
        result.window_id = get_number_from_dictionary(window, kCGWindowNumber);
        result.title = get_string_from_dictionary(window, kCGWindowName);
        // result.app_name = get_strng_from_dictionary(window, kCGWindowOwnerName);
    }

    if (args.get_window_title) {
        result.title = ax_get_focused_window_title(app.pid);
    }

    if (args.get_browser_url && !app.bundle_id.empty()) {
        bool is_browser =
            std::find(BROWSER_APP_BUNDLE_IDS.begin(), BROWSER_APP_BUNDLE_IDS.end(), app.bundle_id);
        if (is_browser) {
            auto native_win_info = native_window_info(app.pid);
            if (args.get_window_title && result.title.empty() && !native_win_info.title.empty()) {
                result.title = native_win_info.title;
            }
            if (!native_win_info.url.empty()) {
                result.url = native_win_info.url;
            }
        }
    }

    return result;
}

bool AutoType::show_window(const AppWindowInfo &window) {
    return window.pid && native_show_app(window.pid);
}

} // namespace keyboard_auto_type
