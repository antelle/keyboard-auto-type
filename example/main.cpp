#include <unistd.h>

#include <iostream>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_USEC = 100000;

int main() {
    keyboard_auto_type::AutoType typer;

    // typer.text(U"Hello!");
    // typer.key_press(U'!', keyboard_auto_type::KeyCode::ANSI_1,
    //     keyboard_auto_type::Modifier::Shift);
    // typer.key_press(0, keyboard_auto_type::KeyCode::ANSI_A,
    // keyboard_auto_type::Modifier::Command);
    auto win = typer.active_window();
    std::cout << "pid: " << win.pid << std::endl
              << "app_name: " << win.app_name << std::endl
              << "title: " << win.title << std::endl
              << "window_id: " << win.window_id << std::endl
              << std::endl;

    usleep(KEY_EVENT_SUBMIT_SLEEP_TIME_USEC);
}
