#include <array>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

constexpr int KEY_HOLD_TOTAL_WAIT_TIME_MS = 10'000;
constexpr int KEY_HOLD_LOOP_WAIT_TIME_MS = 100;

constexpr std::array MODIFIERS{Modifier::Meta, Modifier::Shift, Modifier::Alt, Modifier::Ctrl};
constexpr std::array MODIFIERS_KEY_CODES{
    std::make_pair(Modifier::Meta, KeyCode::Meta),
    std::make_pair(Modifier::Shift, KeyCode::Shift),
    std::make_pair(Modifier::Alt, KeyCode::Alt),
    std::make_pair(Modifier::Ctrl, KeyCode::Ctrl),
};

AutoTypeResult AutoType::text(std::u32string_view text) {
    if (text.length() == 0) {
        return AutoTypeResult::Ok;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    auto native_keys = os_key_codes_for_chars(text);
    auto length = text.length();

    auto pressed_modifiers = Modifier::None;

    for (auto i = 0; i < length; i++) {
        auto character = text[i];
        if (!character) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
            throw std::invalid_argument("Typing a null character is not possible");
#endif
            return AutoTypeResult::BadArg;
        }

        auto native_key_with_modifiers = native_keys[i];

        std::optional<os_key_code_t> code;
        auto modifier = Modifier::None;

        if (native_key_with_modifiers.has_value()) {
            code = native_key_with_modifiers->code;
            modifier = native_key_with_modifiers->modifier;

            for (auto mod_check : MODIFIERS) {
                auto is_pressed = (modifier & mod_check) == mod_check;
                auto was_pressed = (pressed_modifiers & mod_check) == mod_check;
                if (is_pressed && !was_pressed) {
                    result = key_move(Direction::Down, mod_check);
                } else if (!is_pressed && was_pressed) {
                    result = key_move(Direction::Up, mod_check);
                }
                if (result != AutoTypeResult::Ok) {
                    return result;
                }
            }

            pressed_modifiers = modifier;
        }

        result = key_move(Direction::Down, character, code, modifier);
        if (result != AutoTypeResult::Ok) {
            return result;
        }

        result = key_move(Direction::Up, character, code, modifier);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    if (pressed_modifiers != Modifier::None) {
        result = key_move(Direction::Up, pressed_modifiers);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::key_press(KeyCode code, Modifier modifier) {
    auto key_code = os_key_code(code);
    if (!key_code.has_value()) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::invalid_argument(std::string("Key code ") +
                                    std::to_string(static_cast<int>(code)) + " not supported");
#endif
        return AutoTypeResult::BadArg;
    }

    auto result = key_move(Direction::Down, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    result = key_move(Direction::Down, 0, key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }
    result = key_move(Direction::Up, 0, key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    result = key_move(Direction::Up, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::shortcut(KeyCode code) { return key_press(code, shortcut_modifier()); }

AutoTypeResult AutoType::ensure_modifier_not_pressed() {
    auto total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME_MS;
    auto loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME_MS;

    while (total_wait_time > 0) {
        auto pressed_modifiers = get_pressed_modifiers();
        if (pressed_modifiers == Modifier::None) {
            return AutoTypeResult::Ok;
        }
        if (can_unpress_modifier()) {
            for (auto modifier : MODIFIERS) {
                if ((pressed_modifiers & modifier) == modifier) {
                    key_move(Direction::Up, modifier);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(loop_wait_time));
        total_wait_time -= loop_wait_time;
    }
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
    throw std::runtime_error("Modifier key not released");
#endif
    return AutoTypeResult::ModifierNotReleased;
}

AutoTypeResult AutoType::key_move(Direction direction, KeyCode code, Modifier modifier) {
    auto key_code_opt = os_key_code(code);
    if (!key_code_opt.has_value()) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::invalid_argument(std::string("Key code ") +
                                    std::to_string(static_cast<int>(code)) + " not supported");
#endif
        return AutoTypeResult::BadArg;
    }
    return key_move(direction, 0, key_code_opt, modifier);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    if (modifier == Modifier::None) {
        return AutoTypeResult::Ok;
    }

    for (auto [mod_check, key_code] : MODIFIERS_KEY_CODES) {
        AutoTypeResult result;
        if ((modifier & mod_check) == mod_check) {
            result = key_move(direction, key_code);
            if (result != AutoTypeResult::Ok) {
                return result;
            }
        }
    }

    return AutoTypeResult::Ok;
}

} // namespace keyboard_auto_type
