# Keyboard Auto Type

Cross-platform library for simulating keyboard events.

## Status

WIP, don't use it yet.

## Installation

TODO

## Usage

Include the library headers and create an `AutoType` object:
```cpp
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

kbd::AutoType typer;
```

Perform some auto-typing using a high-level API:
```cpp
typer.text(L"Hello, world!");
```

Alternatively, you can simulate one key stroke:
```cpp
typer.key_press(L'a');
```

You can also use [modifiers](Modifiers) to perform different actions, for example, this will send <kbd>⌘</kbd><kbd>A</kbd> to select all text:
```cpp
typer.key_press(L'a', kbd::KeyCode::ANSI_A, typer.shortcut_modifier());
```

Another example of a word deletion using <kbd>⌥</kbd><kbd>⌫</kbd>, in this case you pass `code`, but not `character` (also see more about [shortcuts](#shortcuts)):
```cpp
typer.key_press(0, kbd::KeyCode::BackwardDelete, kbd::Modifier::Option);
```

There's also a method to run combinations like <kbd>⌘</kbd><kbd>A</kbd> / <kbd>⌃</kbd><kbd>A</kbd>:
```cpp
typer.shortcut(kbd::KeyCode::ANSI_C); // copy
typer.shortcut(kbd::KeyCode::ANSI_V); // paste
```

If you need access to more low-level API, there's a function `key_move` that can trigger specific individual key events, for example, only `keyUp`.

## Modifiers

The library supports keyboard modifiers in most of methods. The modifiers are:
```cpp
Modifiers::Ctrl
Modifiers::Alt
Modifiers::Shift
Modifiers::Meta
```

If you like these names more, there are aliases:
```cpp
Modifiers::Control // alias for Ctrl
Modifiers::Option  // alias for Alt
Modifiers::Command // alias for Meta
Modifiers::Win     // alias for Meta
```

Since the "command" shortcut is a very common one, the library provides a convenience method that returns <kbd>⌘</kbd> on macOS and <kbd>Ctrl</kbd> on other OS:
```cpp
AutoType::shortcut_modifier()
```

## Key codes

Where applicable, you can pass different key codes, for example:
```cpp
KeyCode::ANSI_A
```

Key codes are mapped depending on operating system. If the key doesn't exist, an error is returned.

## Shortcuts

There are a lot of keyboard shortcuts available in operating systems and applications. While you can trigger them using this library, please keep in mind that the library doesn't provide a standard way of triggering them. For example, <kbd>⌘</kbd><kbd>A</kbd> will select text on macOS, but not on Windows. It's not a goal of this library to standartize this.

## Errors

All functions return `AutoTypeResult`. If it's not `AutoTypeResult::Ok`, ther are following errors possible:

- `AutoTypeResult::BadArg`: bad argument
- `AutoTypeResult::ModifierNotReleased`: the user is holding a modifier key, simulating keystrokes in this state can have unexpected consequences

## License

[MIT](LICENSE.md)
