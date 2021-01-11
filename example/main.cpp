#include <unistd.h>

#include <iostream>

#include "keyboard-auto-type.h"

int main(int argc, char *argv[]) {
    std::cout << "keyboard-auto-type example" << std::endl;

    keyboard_auto_type::AutoType typer;
    // typer.key_press(L'a', keyboard_auto_type::Modifier::Command);
    // typer.press_paste();
    typer.text(L"Hello");
    typer.key_press(L'!');
    typer.key_press(0, keyboard_auto_type::KeyCode::BackwardDelete, keyboard_auto_type::Modifier::Option);

    usleep(100000);
}
