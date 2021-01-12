#ifndef __KEYBOARD_AUTO_TYPE__KEY_CODE_H__
#define __KEYBOARD_AUTO_TYPE__KEY_CODE_H__

#include <cstdint>

namespace keyboard_auto_type {

enum class KeyCode : uint8_t {
    Undefined,

    // numbers
    ANSI_0,
    ANSI_1,
    ANSI_2,
    ANSI_3,
    ANSI_4,
    ANSI_5,
    ANSI_6,
    ANSI_7,
    ANSI_8,
    ANSI_9,

    // letters
    ANSI_A,
    ANSI_B,
    ANSI_C,
    ANSI_D,
    ANSI_E,
    ANSI_F,
    ANSI_G,
    ANSI_H,
    ANSI_I,
    ANSI_J,
    ANSI_K,
    ANSI_L,
    ANSI_M,
    ANSI_N,
    ANSI_O,
    ANSI_P,
    ANSI_Q,
    ANSI_R,
    ANSI_T,
    ANSI_U,
    ANSI_V,
    ANSI_W,
    ANSI_X,
    ANSI_Y,
    ANSI_Z,

    // function
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    // keypad numbers
    ANSI_Keypad0,
    ANSI_Keypad1,
    ANSI_Keypad2,
    ANSI_Keypad3,
    ANSI_Keypad4,
    ANSI_Keypad5,
    ANSI_Keypad6,
    ANSI_Keypad7,
    ANSI_Keypad8,
    ANSI_Keypad9,

    // keypad other
    ANSI_KeypadClear,
    ANSI_KeypadDecimal,
    ANSI_KeypadDivide,
    ANSI_KeypadEnter,
    ANSI_KeypadEquals,
    ANSI_KeypadMinus,
    ANSI_KeypadMultiply,
    ANSI_KeypadPlus,

    // others
    ANSI_Backslash,
    ANSI_Comma,
    ANSI_Equal,
    ANSI_Grave,
    ANSI_LeftBracket,
    ANSI_Minus,
    ANSI_Period,
    ANSI_Quote,
    ANSI_RightBracket,
    ANSI_Semicolon,
    ANSI_Slash,

    // modifiers
    Meta,
    Command = Meta,
    Win = Meta,
    Control,
    Function,
    Option,
    Alt = Option,
    Shift,
    RightMeta,
    RightCommand = RightMeta,
    RightWin = RightMeta,
    RightControl,
    RightOption,
    RightAlt = RightOption,
    RightShift,
    CapsLock,
    NumLock,
    ScrollLock,

    // arrows
    DownArrow,
    LeftArrow,
    RightArrow,
    UpArrow,

    // navigation
    End,
    Home,
    PageDown,
    PageUp,

    // actions
    ContextMenu,
    BackwardDelete,
    Backspace = BackwardDelete,
    Escape,
    ForwardDelete,
    Help,
    ISO_Section,
    Mute,
    PrintScreen,
    Snapshot = PrintScreen,
    Return,
    Enter = Return,
    Space,
    Tab,
    VolumeDown,
    VolumeUp,
    ControlBreak,
    Sleep,

    // number of key codes
    KeyCodeCount
};

}

#endif
