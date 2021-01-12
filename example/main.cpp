#include <unistd.h>

#include <iostream>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_USEC = 100000;

int main() {
    std::cout << "keyboard-auto-type example" << std::endl;

    keyboard_auto_type::AutoType typer;
    // typer.key_press(L'a', keyboard_auto_type::Modifier::Command);
    // typer.press_paste();
    typer.text(L"Hello");
    typer.key_press(L'!');
    typer.key_press(L'\n');
    // typer.key_press(0, keyboard_auto_type::KeyCode::BackwardDelete,
    // keyboard_auto_type::Modifier::Option);

    typer.key_press(0, keyboard_auto_type::KeyCode::ANSI_S,
                    keyboard_auto_type::AutoType::shortcut_modifier());
    typer.key_press(0, keyboard_auto_type::KeyCode::ANSI_Q,
                    keyboard_auto_type::AutoType::shortcut_modifier());

    usleep(KEY_EVENT_SUBMIT_SLEEP_TIME_USEC);
}
