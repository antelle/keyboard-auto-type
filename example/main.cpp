#include <chrono>
#include <iostream>
#include <thread>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_MS = 100;

int main() {
    try {
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

        typer.text(U"Hello!");
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(KEY_EVENT_SUBMIT_SLEEP_TIME_MS));
}
