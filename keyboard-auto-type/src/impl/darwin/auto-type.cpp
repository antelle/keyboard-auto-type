#include <Carbon/Carbon.h>

#include <array>
#include <codecvt>
#include <exception>
#include <iostream>

#include "auto-release.h"
#include "carbon-helpers.h"
#include "key-map.h"
#include "keyboard-auto-type.h"
#include "native-methods.h"

namespace keyboard_auto_type {

static constexpr int KEY_HOLD_TOTAL_WAIT_TIME = 10'000'000;
static constexpr int KEY_HOLD_LOOP_WAIT_TIME = 100'000;
static constexpr int MAX_KEYBOARD_LAYOUT_CHAR_CODE = 128;

class AutoType::AutoTypeImpl {
  private:
    auto_release<CGEventSourceRef> event_source_ =
        CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    static constexpr int KEYBOARD_LAYOUT_LENGTH = 128;
    std::array<CGKeyCode, KEYBOARD_LAYOUT_LENGTH> keyboard_layout_ = {{}};
    CFDataRef keyboard_layout_data_ = nullptr;

  public:
    AutoTypeResult key_up_down(char32_t character, CGKeyCode code, CGEventFlags flags, bool down) {
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

    AutoTypeResult key_up_and_down(char32_t character, CGKeyCode code, CGEventFlags flags) {
        auto result = key_up_down(character, code, flags, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        return key_up_down(character, code, flags, false);
    }

    AutoTypeResult modifier_up_down(Modifier modifier, bool down) {
        if (modifier == Modifier::None) {
            return AutoTypeResult::Ok;
        }

        constexpr std::array MODIFIERS{Modifier::Command, Modifier::Option, Modifier::Control,
                                       Modifier::Shift};
        constexpr std::array CODES{kVK_Command, kVK_Option, kVK_Control, kVK_Shift};

        for (auto i = 0; i < MODIFIERS.size(); i++) {
            AutoTypeResult result;
            auto mod_check = MODIFIERS.at(i);
            if ((modifier & mod_check) == mod_check) {
                result = key_up_down(0, CODES.at(i), 0, down);
                if (result != AutoTypeResult::Ok) {
                    return result;
                }
            }
        }

        return AutoTypeResult::Ok;
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
        constexpr std::array MOD_STATES{0U, static_cast<UInt32>(NX_DEVICELSHIFTKEYMASK)};
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

    CGKeyCode char_to_key_code(char32_t character) {
        if (character >= keyboard_layout_.size()) {
            return 0;
        }
        return keyboard_layout_.at(character);
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

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, KeyCode code,
                                  Modifier modifier) {
    auto flags = AutoTypeImpl::modifier_to_flags(modifier);
    return impl_->key_up_down(character, map_key_code(code), flags, direction == Direction::Down);
}

AutoTypeResult AutoType::ensure_modifier_not_pressed() {
    auto total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME;
    auto loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME;
    constexpr auto ALL_FLAGS_MASK = (kCGEventFlagMaskCommand | kCGEventFlagMaskShift |
                                     kCGEventFlagMaskAlternate | kCGEventFlagMaskControl);
    while (total_wait_time > 0) {
        auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
        if ((flags & ALL_FLAGS_MASK) == 0) {
            return AutoTypeResult::Ok;
        }
        if (flags & kCGEventFlagMaskCommand) {
            impl_->modifier_up_down(Modifier::Command, false);
        }
        if (flags & kCGEventFlagMaskShift) {
            impl_->modifier_up_down(Modifier::Shift, false);
        }
        if (flags & kCGEventFlagMaskAlternate) {
            impl_->modifier_up_down(Modifier::Alt, false);
        }
        if (flags & kCGEventFlagMaskControl) {
            impl_->modifier_up_down(Modifier::Control, false);
        }
        usleep(loop_wait_time);
        total_wait_time -= loop_wait_time;
    }
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
    throw std::runtime_error("Modifier key not released");
#endif
    return AutoTypeResult::ModifierNotReleased;
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    return impl_->modifier_up_down(modifier, direction == Direction::Down);
}

AutoTypeResult AutoType::key_press(char32_t character, KeyCode code, Modifier modifier) {
    if (!character && code == KeyCode::Undefined) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::invalid_argument("Either character or code must be set");
#endif
        return AutoTypeResult::BadArg;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    CGKeyCode native_key_code = 0;
    if (code == KeyCode::Undefined) {
        impl_->read_keyboard_layout();
        native_key_code = impl_->char_to_key_code(character);
    } else {
        native_key_code = map_key_code(code);
    }

    CGEventFlags flags = 0;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = AutoTypeImpl::modifier_to_flags(modifier);
    }

    result = impl_->key_up_and_down(character, native_key_code, flags);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, false);
    }

    return result;
}

AutoTypeResult AutoType::text(std::u32string_view text, Modifier modifier) {
    if (text.length() == 0) {
        return AutoTypeResult::Ok;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    CGEventFlags flags = 0;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = AutoTypeImpl::modifier_to_flags(modifier);
    }

    impl_->read_keyboard_layout();

    for (auto character : text) {
        auto native_key_code = impl_->char_to_key_code(character);
        result = impl_->key_up_and_down(character, native_key_code, flags);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, false);
    }

    return result;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Command; }

pid_t AutoType::active_pid() { return native_frontmost_app_pid(); }

constexpr std::array BROWSER_APP_BUNDLE_IDS{
    "com.google.Chrome",
    "org.chromium.Chromium",
    "com.google.Chrome.canary",
    "com.apple.Safari",
    "com.apple.SafariTechnologyPreview",
};

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

    AppWindowInfo result = {
        .pid = app.pid,
        .app_name = app.name,
    };

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
