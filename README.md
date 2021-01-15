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
typer.key_press(L'a', kbd::KeyCode::A, typer.shortcut_modifier());
```

Another example of a word deletion using <kbd>⌥</kbd><kbd>⌫</kbd>, in this case you pass `code`, but not `character` (also see more about [shortcuts](#shortcuts)):
```cpp
typer.key_press(0, kbd::KeyCode::BackwardDelete, kbd::Modifier::Option);
```

There's also a method to run combinations like <kbd>⌘</kbd><kbd>A</kbd> / <kbd>⌃</kbd><kbd>A</kbd>:
```cpp
typer.shortcut(kbd::KeyCode::C); // copy
typer.shortcut(kbd::KeyCode::V); // paste
```

If you need access to more low-level API, there's a function `key_move` that can trigger specific individual key events, for example, only `keyUp`.

## Modifiers

The library supports keyboard modifiers in most of methods. The modifiers are:
```cpp
kbd::Modifiers::Ctrl
kbd::Modifiers::Alt
kbd::Modifiers::Shift
kbd::Modifiers::Meta
```

If you like these names more, there are aliases:
```cpp
kbd::Modifiers::Control // alias for Ctrl
kbd::Modifiers::Option  // alias for Alt
kbd::Modifiers::Command // alias for Meta
kbd::Modifiers::Win     // alias for Meta
```

Since the "command" shortcut is a very common one, the library provides a convenience method that returns <kbd>⌘</kbd> on macOS and <kbd>Ctrl</kbd> on other OS:
```cpp
kbd::AutoType::shortcut_modifier()
```

which is also exposed as `shortcut` method:
```cpp
typer.shortcut(kbd::KeyCode::A); // select all
```

## Key codes

Where applicable, you can pass different key codes, for example:
```cpp
kbd::KeyCode::A
```

Key codes are mapped depending on operating system. If the key doesn't exist, an error is returned.

## Shortcuts

There are a lot of keyboard shortcuts available in operating systems and applications. While you can trigger them using this library, please keep in mind that the library doesn't provide a standard way of triggering them. For example, <kbd>⌘</kbd><kbd>A</kbd> will select text on macOS, but not on Windows. It's not a goal of this library to standartize this.

## Errors

By default, if exception support is enabled, auto-type methods can throw an exception. If exception support is disabled, they will just return an error code described below. You can also disable exceptions using `KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS` flag.

All functions return `AutoTypeResult`. If it's not `AutoTypeResult::Ok`, ther are following errors possible:

- `AutoTypeResult::BadArg`: bad argument
- `AutoTypeResult::ModifierNotReleased`: the user is holding a modifier key, simulating keystrokes in this state can have unexpected consequences

## Window management

The library also contains window management functions that you may need for simulating keyboard input.

Get information about the active / frontmost window:

```cpp
kbd::AutoType::active_window()
```

By default it will try to get window title and url (from browsers), which will display prompts about managing other apps. You can pass these options to disable this behavior:
```cpp
window_info = kbd::AutoType::active_window({
    .get_window_title = false,
    .get_browser_url = false,
})
```

This function returns just a pid of the active / frontmost process:
```cpp
kbd::AutoType::active_pid()
```

To activate the window found using `active_window`:
```cpp
kbd::AutoType::show_window(window_info)
```

## License

[MIT](LICENSE.md)
