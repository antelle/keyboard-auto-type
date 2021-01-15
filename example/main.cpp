#if __APPLE__
#include <Carbon/Carbon.h>
#endif

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
    auto win = typer.active_window({
        .get_window_title = true,
        .get_browser_url = true,
    });
    std::cout << "Active window "
              << "pid: " << win.pid << ", "
              << "window_id: " << win.window_id << ", "
              << "app_name: \"" << win.app_name << "\", "
              << "title: \"" << win.title << "\", "
              << "url: \"" << win.url << "\"" << std::endl;

#if __APPLE__
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
#endif
}
