#if __APPLE__
#include <Carbon/Carbon.h>
#endif

#include <iostream>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_USEC = 100000;

int main() {
    keyboard_auto_type::AutoType typer;

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

    typer.text(U"Hello");
    typer.key_press(U'!', keyboard_auto_type::KeyCode::D1, keyboard_auto_type::Modifier::Shift);

#if __APPLE__
    // wait for events to send
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
#endif
}
