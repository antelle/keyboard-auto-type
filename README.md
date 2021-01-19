# Keyboard Auto Type

Cross-platform library for simulating keyboard input events.

<kbd>K</kbd><kbd>E</kbd><kbd>Y</kbd><kbd>B</kbd><kbd>O</kbd><kbd>A</kbd><kbd>R</kbd><kbd>D</kbd><kbd>-</kbd><kbd>A</kbd><kbd>U</kbd><kbd>T</kbd><kbd>O</kbd><kbd>-</kbd><kbd>T</kbd><kbd>Y</kbd><kbd>P</kbd><kbd>E</kbd>

## TL;DR

This library allows you to send keystrokes to different applications as if they were typed by the user. It provides both simple and low-level API consistent across platforms. It can be useful in password managers, automation, testing software, and everywhere else where you need to simulate user input.

A minimalistic example:

```cpp
#include "keyboard-auto-type.h"

// create an instance of AutoType
keyboard_auto_type::AutoType typer;

// get active / frontmost window information
auto win = typer.active_window({ .get_window_title = true });
do_something_with(win.pid, win.app_name, win.title);

// type something
typer.text(U"Hello, world!");
// press Enter
typer.key_press(kbd::KeyCode::Enter);
// select all
typer.shortcut(keyboard_auto_type::KeyCode::A);
```

See more in the [Usage](#usage) section.

## Status

The library is WIP, so the API is not stable now. Detailed feature status:

|                            | macOS                      | Windows                 | Linux |
|----------------------------|----------------------------|-------------------------|-------|
| Sending key codes          | :white_check_mark:         | :white_check_mark:      | :x:   |
| Typing text                | :white_check_mark:         | :white_check_mark:      | :x:   |
| Layout-aware text entry    | :white_check_mark:         | :white_check_mark:      | :x:   |
| Emoji and CJK characters   | :white_check_mark:         | :white_check_mark:      | :x:   |
| Getting window information | :white_check_mark:         | :white_check_mark:      | :x:   |
| Getting browser URL        | :eight_spoked_asterisk:    | :eight_spoked_asterisk: | :x:   |
| Switching to an app        | :white_check_mark:         | :x:                     | :x:   |
| Switching a given window   | :x:                        | :white_check_mark:      | :x:   |
| Virtual desktops (spaces)  | :eight_pointed_black_star: | :white_check_mark:      | :x:   |

## Installation

TODO

## Usage

Include the library header and create an `AutoType` object:
```cpp
#include "keyboard-auto-type.h"

// for convenience, to type less
namespace kbd = keyboard_auto_type;

kbd::AutoType typer;
```

Perform some auto-typing using the high-level API:
```cpp
typer.text(U"Hello, world!");
```

Alternatively, you can simulate one key stroke:
```cpp
typer.key_press(kbd::KeyCode::A);
```

You can also use [modifiers](#Modifiers) to perform different actions, for example, this will send <kbd>⌘</kbd><kbd>A</kbd> to select all text:
```cpp
typer.key_press(kbd::KeyCode::A, typer.shortcut_modifier());
```

There's also a method to run combinations like <kbd>⌘</kbd><kbd>A</kbd> / <kbd>⌃</kbd><kbd>A</kbd>:
```cpp
typer.shortcut(kbd::KeyCode::C); // copy
typer.shortcut(kbd::KeyCode::V); // paste
```

Another example of word deletion using <kbd>⌥</kbd><kbd>⌫</kbd>, in this case you pass `code`, but not `character` (also see more about [shortcuts](#shortcuts)):
```cpp
typer.key_press(kbd::KeyCode::BackwardDelete, kbd::Modifier::Option);
```

## Low-level API

If you need access to a low-level API, there's a method `key_move` that can trigger specific individual key events, for example, using this your can trigger only `keyUp` or simulate a keypress with right Ctrl.

Other methods (`key_press`, `text`) will also press the modifier key for you, while `key_move` won't do it. However it accepts `modifier` parameter because you may need to pass it to the key event. For example, an event emitted when <kbd>A</kbd> is moved down in <kbd>⌘</kbd><kbd>A</kbd> combination, contains a flag that allows to understand that Command is now pressed.

First of all, there are three things that you can do using a keyboard:
- enter text, which sends unicode values of typed _characters_, this usually depends on keyboard layout
- press a key and send a _key code_, for example <kbd>⌘</kbd><kbd>A</kbd> works in the same way no matter which keyboard layout is selected
- manipulate the modifier key itself (Shift, Ctrl, ...)

Therefore, there are three methods:

```cpp
// press the A key as text
typer.key_move(kbd::Direction::Down,
               U'a',
               kbd::Modifier::Alt);
// send the key code A
typer.key_move(kbd::Direction::Down,
               kbd::KeyCode::A,
               kbd::Modifier::Alt);
// use a modifier: Shift, Ctrl, ...
typer.key_move(kbd::Direction::Down,
               kbd::Modifier::Alt);
```

Additionally, this method checks if any of modifiers is currently pressed. It makes sense to call it before starting text input. This method will throw an exception if the user keeps holding a key unnaturally long (more than 10s, to be precise):
```cpp
typer.ensure_modifier_not_pressed();
```

And finally, if you want to mess with underlying OS key codes, there's a way to do so:
```cpp
// get a system key code, type varies per OS
auto key_code = typer.os_key_code(kbd::KeyCode::A)

// using the current locale, find its key by character
auto key = typer.os_key_code_for_char(character)
// the result is not guaranteed to exist, so it's optional
// e.g. there's no key combination for "★"
key->code     // key code that represents the character
key->modifier // modifier required to get the desired result

// same as above, but optimized for long strings
typer.os_key_codes_for_chars(long_string)

// and here we go, pass what you get to:
typer.key_move(kbd::Direction::Down, U'★', key.code, key.modifier)

// but be careful, it's a low-level API, all checks are on you
```

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

All functions return `AutoTypeResult`. If it's not `AutoTypeResult::Ok`, there are following errors possible:

- `AutoTypeResult::BadArg`: bad argument
- `AutoTypeResult::ModifierNotReleased`: the user is holding a modifier key, simulating keystrokes in this state can have unexpected consequences
- `AutoTypeResult::KeyPressFailed`: we have sent a keypress event, however it didn't have any effect
- `AutoTypeResult::NotSupported`: the given key code is not supported on this operating system
- `AutoTypeResult::OsError`: opereating system reported an error during simulating keyboard input

## Window management

The library also contains window management functions that you may need for simulating keyboard input.

Get information about the active / frontmost window:

```cpp
kbd::AutoType::active_window()
```

There's an option to get window title and url from browsers, this will display prompts about managing other apps. You can pass these options to get window title and url respectively:
```cpp
window_info = kbd::AutoType::active_window({
    .get_window_title = true,
    .get_browser_url = true,
})
```

This function returns just a pid of the active / frontmost process:
```cpp
kbd::AutoType::active_pid()
```

To activate a window found using `active_window`:
```cpp
kbd::AutoType::show_window(window_info)
```

## Strings

The library accepts 32-bit platform-independent wide characters in form of `std::u32string` or `char32_t`, the conversion is up to you. If you prefer, you can also pass `std::wstring`. In some places, such as window information, it will return `std::string`, these strings are in UTF-8.

## Thread safety

The library is not thread safe. Moreover, it's not a good idea to manipulate the keyboard from different threads at the same time, don't do it.

## C++ standard

The library requires C++17 or above. Tests and examples are using C++20 features.

## Development

You will need CMake and the standard development toolchain to build the project.

`Makefile` is provided as a convenience measure to launch `cmake` commands. There's nothing important there, however using it is easier than typing commands. If you're familiar with CMake, you can build without `make` if you prefer.

Build the library:
```sh
make
```

On Windows you can use `nmake` instead:
```sh
nmake
```

IDE projects can be generated on macOS using
```sh
make xcode-project
```

and on Windows with
```sh
nmake vs-project
```

## Tests

Tests currently run only in English language (for Copy-Paste menu) and QWERTY keyboard layout. To run tests:
```sh
make tests
```

or, on Windows:
```sh
nmake tests
```

## License

[MIT](LICENSE.md)
