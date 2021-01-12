#include "key-map.h"

#include <array>

namespace keyboard_auto_type {

constexpr std::array<CGKeyCode, static_cast<size_t>(KeyCode::KeyCodeCount)> key_map{
    0,

    // numbers
    kVK_ANSI_0, kVK_ANSI_1, kVK_ANSI_2, kVK_ANSI_3, kVK_ANSI_4, kVK_ANSI_5, kVK_ANSI_6, kVK_ANSI_7,
    kVK_ANSI_8, kVK_ANSI_9,

    // letters
    kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_F, kVK_ANSI_G, kVK_ANSI_H,
    kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L, kVK_ANSI_M, kVK_ANSI_N, kVK_ANSI_O, kVK_ANSI_P,
    kVK_ANSI_Q, kVK_ANSI_R, kVK_ANSI_S, kVK_ANSI_T, kVK_ANSI_U, kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X,
    kVK_ANSI_Y, kVK_ANSI_Z,

    // function
    kVK_F1, kVK_F2, kVK_F3, kVK_F4, kVK_F5, kVK_F6, kVK_F7, kVK_F8, kVK_F9, kVK_F10, kVK_F11,
    kVK_F12, kVK_F13, kVK_F14, kVK_F15, kVK_F16, kVK_F17, kVK_F18, kVK_F19, kVK_F20,
    0, // F21
    0, // F22
    0, // F23
    0, // F24

    // keypad numbers
    kVK_ANSI_Keypad0, kVK_ANSI_Keypad1, kVK_ANSI_Keypad2, kVK_ANSI_Keypad3, kVK_ANSI_Keypad4,
    kVK_ANSI_Keypad5, kVK_ANSI_Keypad6, kVK_ANSI_Keypad7, kVK_ANSI_Keypad8, kVK_ANSI_Keypad9,

    // keypad other
    kVK_ANSI_KeypadClear, kVK_ANSI_KeypadDecimal, kVK_ANSI_KeypadDivide, kVK_ANSI_KeypadEnter,
    kVK_ANSI_KeypadEquals, kVK_ANSI_KeypadMinus, kVK_ANSI_KeypadMultiply, kVK_ANSI_KeypadPlus,

    // others
    kVK_ANSI_Backslash, kVK_ANSI_Comma, kVK_ANSI_Equal, kVK_ANSI_Grave, kVK_ANSI_LeftBracket,
    kVK_ANSI_Minus, kVK_ANSI_Period, kVK_ANSI_Quote, kVK_ANSI_RightBracket, kVK_ANSI_Semicolon,
    kVK_ANSI_Slash,

    // modifiers
    kVK_Command, kVK_Control, kVK_Function, kVK_Option, kVK_Shift, kVK_RightCommand,
    kVK_RightControl, kVK_RightOption, kVK_RightShift, kVK_CapsLock,
    0, // NumLock
    0, // ScrollLock

    // arrows
    kVK_DownArrow, kVK_LeftArrow, kVK_RightArrow, kVK_UpArrow,

    // navigation
    kVK_End, kVK_Home, kVK_PageDown, kVK_PageUp,

    // actions
    0, // ContextMenu
    kVK_Delete, kVK_Escape, kVK_ForwardDelete, kVK_Help, kVK_ISO_Section, kVK_Mute,
    0, // PrintScreen
    kVK_Return, kVK_Space, kVK_Tab, kVK_VolumeDown, kVK_VolumeUp,
    0, // ControlBreak
    0, // Sleep
};

CGKeyCode map_key_code(KeyCode code) {
    auto code_byte = static_cast<uint8_t>(code);
    if (code_byte >= key_map.size()) {
        return 0;
    }
    return key_map.at(code_byte);
}

} // namespace keyboard_auto_type
