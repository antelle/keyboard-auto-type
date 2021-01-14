#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include <array>
#include <codecvt>
#include <exception>
#include <iostream>
#include <string>

#include "auto-release.h"
#include "key-map.h"
#include "keyboard-auto-type.h"
#include "native-methods.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
    auto_release<CGEventSourceRef> event_source_ =
        CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    static constexpr int KEYBOARD_LAYOUT_LENGTH = 128;
    std::array<CGKeyCode, KEYBOARD_LAYOUT_LENGTH> keyboard_layout_ = {{}};
    CFDataRef keyboard_layout_data_ = nullptr;

    static constexpr int KEY_HOLD_TOTAL_WAIT_TIME = 10 * 1000 * 1000;
    static constexpr int KEY_HOLD_LOOP_WAIT_TIME = 10000;
    static constexpr int MAX_KEYBOARD_LAYOUT_CHAR_CODE = 128;

  public:
    AutoTypeResult validate_system_state() {
        auto total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME;
        auto loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME;
        constexpr auto all_flags_mask = (kCGEventFlagMaskCommand | kCGEventFlagMaskShift |
                                         kCGEventFlagMaskAlternate | kCGEventFlagMaskControl);
        while (total_wait_time > 0) {
            auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
            if ((flags & all_flags_mask) == 0) {
                return AutoTypeResult::Ok;
            }
            if (flags & kCGEventFlagMaskCommand) {
                modifier_up_down(Modifier::Command, false);
            }
            if (flags & kCGEventFlagMaskShift) {
                modifier_up_down(Modifier::Shift, false);
            }
            if (flags & kCGEventFlagMaskAlternate) {
                modifier_up_down(Modifier::Alt, false);
            }
            if (flags & kCGEventFlagMaskControl) {
                modifier_up_down(Modifier::Control, false);
            }
            usleep(loop_wait_time);
            total_wait_time -= loop_wait_time;
        }
        if (throw_exceptions_) {
#if __cpp_exceptions
            throw std::runtime_error("Modifier key not released");
#endif
        }
        return AutoTypeResult::ModifierNotReleased;
    }

    AutoTypeResult key_up_down(char32_t character, CGKeyCode code, CGEventFlags flags, bool down) {
        if (down) {
            auto result = validate_system_state();
            if (result != AutoTypeResult::Ok) {
                return result;
            }
        }

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

        constexpr std::array modifiers{Modifier::Command, Modifier::Option, Modifier::Control,
                                       Modifier::Shift};
        constexpr std::array codes{kVK_Command, kVK_Option, kVK_Control, kVK_Shift};

        for (auto i = 0; i < modifiers.size(); i++) {
            AutoTypeResult result;
            auto mod_check = modifiers.at(i);
            if ((modifier & mod_check) == mod_check) {
                result = key_up_down(0, codes.at(i), 0, down);
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
        constexpr std::array mod_states{0U, static_cast<UInt32>(NX_DEVICELSHIFTKEYMASK)};
        for (int code = 0; code < MAX_KEYBOARD_LAYOUT_CHAR_CODE; code++) {
            for (auto mod_state : mod_states) {
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
};

CGEventFlags modifier_to_flags(Modifier modifier) {
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

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, KeyCode code,
                                  Modifier modifier) {
    auto flags = modifier_to_flags(modifier);
    return impl_->key_up_down(character, map_key_code(code), flags, direction == Direction::Down);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    return impl_->modifier_up_down(modifier, direction == Direction::Down);
}

AutoTypeResult AutoType::key_press(char32_t character, KeyCode code, Modifier modifier) {
    if (!character && code == KeyCode::Undefined) {
        if (throw_exceptions_) {
#if __cpp_exceptions
            throw std::invalid_argument("Either character or code must be set");
#endif
        }
        return AutoTypeResult::BadArg;
    }

    CGKeyCode native_key_code = 0;
    if (code == KeyCode::Undefined) {
        impl_->read_keyboard_layout();
        native_key_code = impl_->char_to_key_code(character);
    } else {
        native_key_code = map_key_code(code);
    }

    CGEventFlags flags = 0;
    AutoTypeResult result;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = modifier_to_flags(modifier);
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

    CGEventFlags flags = 0;
    AutoTypeResult result = AutoTypeResult::Ok;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = modifier_to_flags(modifier);
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

AppWindowInfo AutoType::active_window() {
    auto pid = native_frontmost_app_pid();
    if (!pid) {
        return {};
    }

    auto_release windows = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    if (!windows) {
        return {};
    }

    AppWindowInfo result = {.pid = pid};

    auto count = CFArrayGetCount(windows);
    for (auto i = 0; i < count; i++) {
        auto window = reinterpret_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows, i));

        // auto desc = CFCopyDescription(window); // for debugging
        // std::cout << CFStringGetCStringPtr(desc, kCFStringEncodingUTF8) << std::endl;

        if (!CFDictionaryContainsKey(window, kCGWindowOwnerPID)) {
            continue;
        }

        auto window_layer_number =
            reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(window, kCGWindowLayer));
        int window_layer;
        if (!CFNumberGetValue(window_layer_number, kCFNumberSInt32Type, &window_layer)) {
            continue;
        }
        if (window_layer > 0) {
            continue;
        }

        auto window_pid_number =
            reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(window, kCGWindowOwnerPID));
        int window_pid;
        if (!CFNumberGetValue(window_pid_number, kCFNumberSInt32Type, &window_pid)) {
            continue;
        }
        if (window_pid != pid) {
            continue;
        }

        if (CFDictionaryContainsKey(window, kCGWindowOwnerName)) {
            auto window_owner_name_str =
                reinterpret_cast<CFStringRef>(CFDictionaryGetValue(window, kCGWindowOwnerName));
            auto length = CFStringGetLength(window_owner_name_str);
            auto max_length = length * 4 + 1;
            char window_owner_name[max_length];
            if (CFStringGetCString(window_owner_name_str, window_owner_name, max_length,
                                   kCFStringEncodingUTF8)) {
                result.app_name = window_owner_name;
            }
        }

        if (CFDictionaryContainsKey(window, kCGWindowNumber)) {
            auto window_number =
                reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(window, kCGWindowNumber));
            int window_id;
            if (CFNumberGetValue(window_number, kCFNumberSInt32Type, &window_id)) {
                result.window_id = window_id;
            }
        }
    }

    auto_release<CFArrayRef> ax_windows = nullptr;
    auto accessibility_app = AXUIElementCreateApplication(pid);
    auto ax_res =
        AXUIElementCopyAttributeValues(accessibility_app, kAXWindowsAttribute, 0, 10, &ax_windows);
    if (ax_res != kAXErrorSuccess) {
        return result;
    }

    auto ax_windows_num = CFArrayGetCount(ax_windows);
    for (int i = 0; i < ax_windows_num; i++) {
        auto ax_window = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(ax_windows, i));
        auto_release<CFTypeRef> ax_main = nullptr;
        ax_res = AXUIElementCopyAttributeValue(ax_window, kAXMainAttribute, &ax_main);
        if (ax_res != kAXErrorSuccess) {
            continue;
        }
        auto is_main =
            CFBooleanGetValue(reinterpret_cast<CFBooleanRef>(static_cast<CFTypeRef>(ax_main)));
        if (!is_main) {
            continue;
        }
        auto_release<CFTypeRef> ax_title = nullptr;
        ax_res = AXUIElementCopyAttributeValue(ax_window, kAXTitleAttribute, &ax_title);
        if (ax_res != kAXErrorSuccess) {
            continue;
        }
        char buf[1024];
        CFStringGetCString(reinterpret_cast<CFStringRef>(static_cast<CFTypeRef>(ax_title)), buf,
                           sizeof(buf), kCFStringEncodingUTF8);
        result.title = buf;
    }

    return result;
}

} // namespace keyboard_auto_type
