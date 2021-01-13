#include <unistd.h>

#include "keyboard-auto-type.h"

constexpr int KEY_EVENT_SUBMIT_SLEEP_TIME_USEC = 100000;

int main() {
    keyboard_auto_type::AutoType typer;
    typer.text(L"Hello");
    typer.key_press(L'!');

    usleep(KEY_EVENT_SUBMIT_SLEEP_TIME_USEC);
}
