#include <Carbon/Carbon.h>
#include <unistd.h>

#include <iostream>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_USEC = 100000;

int main() {
    keyboard_auto_type::AutoType typer;

    // typer.text(U"Hello!");
    // typer.key_press(U'!', keyboard_auto_type::KeyCode::D1,
    //     keyboard_auto_type::Modifier::Shift);
    // typer.key_press(0, keyboard_auto_type::KeyCode::A,
    // keyboard_auto_type::Modifier::Command);
    while (true) {
        auto win = typer.active_window();
        std::cout << "Active window "
                  << "pid: " << win.pid << ", "
                  << "window_id: " << win.window_id << ", "
                  << "app_name: \"" << win.app_name << "\", "
                  << "title: \"" << win.title << "\", "
                  << "url: \"" << win.url << "\"" << std::endl;
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false);
    }

    usleep(KEY_EVENT_SUBMIT_SLEEP_TIME_USEC);
}
