#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include <array>
#include <string>

#include "auto-release.h"
#include "key-map.h"
#include "keyboard-auto-type.h"

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
    static AutoTypeResult validate_system_state() {
        int total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME;
        int loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME;
        while (total_wait_time > 0) {
            auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
            if ((flags & (kCGEventFlagMaskCommand | kCGEventFlagMaskShift |
                          kCGEventFlagMaskAlternate | kCGEventFlagMaskControl)) == 0) {
                return AutoTypeResult::Ok;
            }
            usleep(loop_wait_time);
            total_wait_time -= loop_wait_time;
        }
        return AutoTypeResult::ModifierNotReleased;
    }

    AutoTypeResult key_up_down(wchar_t character, CGKeyCode code, CGEventFlags flags, bool down) {
        if (!character && !code) {
            return AutoTypeResult::BadArg;
        }

        if (down) {
            auto result = validate_system_state();
            if (result != AutoTypeResult::Ok) {
                return result;
            }
        }

        auto_release event = CGEventCreateKeyboardEvent(event_source_, code, down);
        if (character) {
            auto uni_char = static_cast<UniChar>(static_cast<uint16_t>(character));
            CGEventKeyboardSetUnicodeString(event, 1, &uni_char);
        }
        if (flags) {
            CGEventSetFlags(event, flags);
        }
        CGEventPost(kCGHIDEventTap, event);

        return AutoTypeResult::Ok;
    }

    AutoTypeResult key_up_and_down(wchar_t character, CGKeyCode code, CGEventFlags flags) {
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
        for (int code = 0; code < MAX_KEYBOARD_LAYOUT_CHAR_CODE; code++) {
            UCKeyTranslate(layout, code, kUCKeyActionDisplay, 0, kbd_type,
                           kUCKeyTranslateNoDeadKeysBit, &keys_down, 4, &length, chars.data());
            auto ch = chars[0];

            if (length && (ch >= ' ' && ch <= '~' || ch == '\t' || ch == ' ') &&
                !keyboard_layout_.at(ch)) {
                keyboard_layout_.at(ch) = code;
            }
        }

        keyboard_layout_data_ = layout_data;
    }

    CGKeyCode char_to_key_code(wchar_t character) {
        if (character < 0 || character >= keyboard_layout_.size()) {
            return 0;
        }
        auto index = std::tolower(character);
        return keyboard_layout_.at(index);
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

AutoTypeResult AutoType::key_move(Direction direction, wchar_t character, KeyCode code,
                                  Modifier modifier) {
    auto flags = modifier_to_flags(modifier);
    return impl_->key_up_down(character, map_key_code(code), flags, direction == Direction::Down);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    return impl_->modifier_up_down(modifier, direction == Direction::Down);
}

AutoTypeResult AutoType::key_press(wchar_t character, KeyCode code, Modifier modifier) {
    if (!character && code == KeyCode::Undefined) {
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

AutoTypeResult AutoType::text(std::wstring_view text, Modifier modifier) {
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

} // namespace keyboard_auto_type
