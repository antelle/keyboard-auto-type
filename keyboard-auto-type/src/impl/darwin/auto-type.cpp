#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include "auto-release.h"
#include "key-map.h"
#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
    auto_release<CGEventSourceRef> event_source_ = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

  public:
    AutoTypeResult validate_system_state() {
        int total_wait_time = 10 * 1000 * 1000; // TODO: const
        int loop_wait_time = 10000;
        while (total_wait_time > 0) {
            auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
            if ((flags & (kCGEventFlagMaskCommand | kCGEventFlagMaskShift | kCGEventFlagMaskAlternate |
                          kCGEventFlagMaskControl)) == 0) {
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
            auto uniChar = static_cast<UniChar>(character);
            CGEventKeyboardSetUnicodeString(event, 1, &uniChar);
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

        Modifier modifiers[] = {Modifier::Command, Modifier::Option, Modifier::Control, Modifier::Shift};
        CGKeyCode codes[] = {kVK_Command, kVK_Option, kVK_Control, kVK_Shift};

        for (auto i = 0; i < 4; i++) {
            AutoTypeResult result;
            auto mod_check = modifiers[i];
            if ((modifier & mod_check) == mod_check) {
                result = key_up_down(0, codes[i], 0, down);
                if (result != AutoTypeResult::Ok) {
                    return result;
                }
            }
        }

        return AutoTypeResult::Ok;
    }

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
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, wchar_t character, KeyCode code, Modifier modifier) {
    auto flags = impl_->modifier_to_flags(modifier);
    return impl_->key_up_down(character, map_key_code(code), flags, direction == Direction::Down);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    return impl_->modifier_up_down(modifier, direction == Direction::Down);
}

AutoTypeResult AutoType::key_press(wchar_t character, KeyCode code, Modifier modifier) {
    if (!character && code == KeyCode::Undefined) {
        return AutoTypeResult::BadArg;
    }

    CGEventFlags flags = 0;
    AutoTypeResult result;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = impl_->modifier_to_flags(modifier);
    }

    result = impl_->key_up_and_down(character, map_key_code(code), flags);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, false);
    }

    return result;
}

AutoTypeResult AutoType::text(std::wstring text, Modifier modifier) {
    if (text.length() == 0) {
        return AutoTypeResult::Ok;
    }

    CGEventFlags flags = 0;
    AutoTypeResult result;

    if (modifier != Modifier::None) {
        result = impl_->modifier_up_down(modifier, true);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        flags = impl_->modifier_to_flags(modifier);
    }

    for (auto character : text) {
        result = impl_->key_up_and_down(character, 0, flags);
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
