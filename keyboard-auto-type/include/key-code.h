#ifndef KEYBOARD_AUTO_TYPE_KEY_CODE_H
#define KEYBOARD_AUTO_TYPE_KEY_CODE_H

namespace keyboard_auto_type {

enum class KeyCode {
    Undefined,

    // numbers
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,

    // letters
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

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
    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,

    // keypad other
    KeypadClear,
    KeypadDecimal,
    KeypadDivide,
    KeypadEnter,
    KeypadEquals,
    KeypadMinus,
    KeypadMultiply,
    KeypadPlus,

    // others
    Backslash,
    Comma,
    Equal,
    Grave,
    LeftBracket,
    Minus,
    Period,
    Quote,
    RightBracket,
    Semicolon,
    Slash,

    // modifiers
    Meta,
    Command = Meta,
    Win = Meta,
    Ctrl,
    Control = Ctrl,
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
    Insert,
    Sleep,

    // number of key codes
    KeyCodeCount
};

} // namespace keyboard_auto_type

#endif
