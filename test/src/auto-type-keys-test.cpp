#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"
#include "platform-util.h"
#include "test-util.h"

namespace kbd = keyboard_auto_type;

namespace keyboard_auto_type_test {

class AutoTypeKeysTest : public testing::Test {
  protected:
    const std::string file_name = "build/test.txt";
    std::u32string expected_text;
    std::vector<std::string> expected_events;

    static void SetUpTestSuite() {}

    static void TearDownTestSuite() {}

    virtual void SetUp() {
        expected_text = U"";
        expected_events.clear();
        open_test_app();
    }

    virtual void TearDown() {
        save_text_and_close_test_app();
        check_expected_text();
    }

  private:
    void check_expected_text() {
        constexpr std::string_view TEXT_LINE_START = "text ";
        constexpr std::string_view EVENT_LINE_START = "event ";
        constexpr std::string_view DONE_LINE = "done";

        std::string text;
        std::vector<std::string> events;
        bool finished = false;

        for (auto i = 0; i < 50; i++) {
            events.clear();
            std::string line;
            std::ifstream ifs(file_name);
            while (std::getline(ifs, line)) {
                if (line.starts_with(EVENT_LINE_START)) {
                    auto event = line.substr(EVENT_LINE_START.length());
                    events.push_back(event);
                } else if (line.starts_with(TEXT_LINE_START)) {
                    text = line.substr(TEXT_LINE_START.length());
                } else if (line == DONE_LINE) {
                    finished = true;
                    break;
                }
            }
            if (finished) {
                break;
            }
            wait_millis(100);
        }

        ASSERT_TRUE(finished) << "File is not complete: 'done' was not found";

        std::istringstream text_stream(text);

        for (size_t i = 0; i < expected_text.length(); i++) {
            uint32_t char_code = 0;
            ASSERT_FALSE(text_stream.eof()) << "Actual text is too short";
            text_stream >> char_code;
            auto expected_char = static_cast<uint32_t>(expected_text[i]);
            ASSERT_EQ(expected_char, char_code)
                << std::string("Character at index ") + std::to_string(i) + " is different";
        }

        ASSERT_TRUE(text_stream.eof()) << "Actual text is too long";

        if (expected_events.size()) {
            ASSERT_GE(events.size(), expected_events.size()) << "Not enough key events";
            for (auto i = 0; i < expected_events.size(); i++) {
                ASSERT_EQ(expected_events[i], events[i])
                    << std::string("Event with index ") + std::to_string(i) + " doesn't match";
            }
        }
    }

    void open_edit_menu() {
        kbd::AutoType typer;
#if __APPLE__
        // highlight the "Apple" menu
        typer.key_press(kbd::KeyCode::F2, kbd::Modifier::Ctrl);
        // go to "Edit" menu
        typer.key_press(kbd::KeyCode::RightArrow);
        typer.key_press(kbd::KeyCode::RightArrow);
        typer.key_press(kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(kbd::KeyCode::DownArrow);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // highlight the first menu ("File")
        typer.key_press(kbd::KeyCode::Alt);
        // go to "Edit" menu
        typer.key_press(kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(kbd::KeyCode::DownArrow);
#else
        FAIL() << "open_edit_menu not implemented";
#endif
    }

  public:
    void press_menu_select_all() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(kbd::KeyCode::S);
        typer.key_press(kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(kbd::KeyCode::S);
#endif
    }

    void press_menu_cut() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(kbd::KeyCode::C);
        typer.key_press(kbd::KeyCode::U);
        typer.key_press(kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(kbd::KeyCode::DownArrow);
        typer.key_press(kbd::KeyCode::DownArrow);
        typer.key_press(kbd::KeyCode::DownArrow);
        typer.key_press(kbd::KeyCode::Enter);
#endif
    }

    void press_menu_paste() {
        kbd::AutoType typer;
        open_edit_menu();
        typer.key_press(kbd::KeyCode::P);
#if __APPLE__
        typer.key_press(kbd::KeyCode::Enter);
#endif
    }
};

TEST_F(AutoTypeKeysTest, text_letter) {
    expected_text = U"a";
    expected_events = {
        "keydown 0 - KeyA standard",
        "keypress 97 - KeyA standard",
        "keyup 0 - KeyA standard",
    };

    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_two_letters) {
    expected_text = U"ab";
    expected_events = {
        "keydown 0 - KeyA standard", "keypress 97 - KeyA standard", "keyup 0 - KeyA standard",
        "keydown 0 - KeyB standard", "keypress 98 - KeyB standard", "keyup 0 - KeyB standard",
    };

    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_two_lines) {
    expected_text = U"ab\ncd";
    expected_events = {
        "keydown 0 - KeyA standard",  "keypress 97 - KeyA standard",  "keyup 0 - KeyA standard",
        "keydown 0 - KeyB standard",  "keypress 98 - KeyB standard",  "keyup 0 - KeyB standard",
        "keydown 0 - Enter standard", "keypress 13 - Enter standard", "keyup 0 - Enter standard",
        "keydown 0 - KeyC standard",  "keypress 99 - KeyC standard",  "keyup 0 - KeyC standard",
        "keydown 0 - KeyD standard",  "keypress 100 - KeyD standard", "keyup 0 - KeyD standard",
    };

    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_capital) {
    expected_text = U"AbCD!e";
    expected_events = {
        "keydown 0 shift ShiftLeft left",    "keydown 0 shift KeyA standard",
        "keypress 65 shift KeyA standard",   "keyup 0 shift KeyA standard",
        "keyup 0 - ShiftLeft left",          "keydown 0 - KeyB standard",
        "keypress 98 - KeyB standard",       "keyup 0 - KeyB standard",
        "keydown 0 shift ShiftLeft left",    "keydown 0 shift KeyC standard",
        "keypress 67 shift KeyC standard",   "keyup 0 shift KeyC standard",
        "keydown 0 shift KeyD standard",     "keypress 68 shift KeyD standard",
        "keyup 0 shift KeyD standard",       "keydown 0 shift Digit1 standard",
        "keypress 33 shift Digit1 standard", "keyup 0 shift Digit1 standard",
        "keyup 0 - ShiftLeft left",          "keydown 0 - KeyE standard",
        "keypress 101 - KeyE standard",      "keyup 0 - KeyE standard",
    };

    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_wstring) {
    expected_text = U"AbCßµḀ";

    kbd::AutoType typer;
    typer.text(L"AbCßµḀ");
}

TEST_F(AutoTypeKeysTest, text_unicode_basic) {
    kbd::AutoType typer;
    expected_text = U"";

    constexpr std::array char_ranges{
        // basic latin
        std::pair{U' ', U'~'},
        // latin1-supplement
        std::pair{U'¡', U'ÿ'},
        // latin extended
        std::pair{U'Ā', U'ɏ'},
        // IPA extensions
        std::pair{U'ɐ', U'ʯ'},
        // greek and coptic
        std::pair{U'Ͱ', U'ͳ'},
        std::pair{U'Α', U'Θ'},
        std::pair{U'α', U'μ'},
        std::pair{U'Ϯ', U'ϱ'},
        std::pair{U'ϼ', U'Ͽ'},
        // cyrillic
        std::pair{U'Ѐ', U'Б'},
        // armenian
        std::pair{U'Ա', U'Ե'},
        // hebrew
        std::pair{U'א', U'ה'},
        // arabic
        std::pair{U'ب', U'ج'},
        // bengali
        std::pair{U'৪', U'৭'},
        // thai
        std::pair{U'ก', U'ฅ'},
        // latin extended additional
        std::pair{U'Ḁ', U'ḅ'},
        // arrows
        std::pair{U'←', U'↔'},
        // CJK
        std::pair{U'一', U'三'},
        // hiragana
        std::pair{U'ぁ', U'う'},
        // katakana
        std::pair{U'゠', U'イ'},
    };

    for (auto [from, to] : char_ranges) {
        auto range_count = 0;
        for (auto ch = from; ch <= to; ch++) {
            expected_text += ch;
            if (++range_count % 50 == 0) {
                expected_text += U"\n";
            }
        }
        expected_text += U"\n";
    }

    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_unicode_emoji) {
    expected_text = U"🍆🍑😈";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_unicode_supplementary_ideographic) {
    expected_text = U"𠀧𠀪";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_unpress_modifiers) {
    expected_text = U"a";
    kbd::AutoType typer;
    typer.key_move(kbd::Direction::Down, kbd::Modifier::Shift | kbd::Modifier::Control);
    typer.text(U"a");
}

TEST_F(AutoTypeKeysTest, key_press_key_code) {
    expected_text = U"0b";
    kbd::AutoType typer;
    typer.key_press(kbd::KeyCode::D0);
    typer.key_press(kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_press_key_code_modifier) {
    expected_text = U"1!cC";
    kbd::AutoType typer;
    typer.key_press(kbd::KeyCode::D1);
    typer.key_press(kbd::KeyCode::D1, kbd::Modifier::Shift);
    typer.key_press(kbd::KeyCode::C);
    typer.key_press(kbd::KeyCode::C, kbd::Modifier::RightShift);
}

TEST_F(AutoTypeKeysTest, key_press_menu) {
    expected_text = U"more text";
    kbd::AutoType typer;

    typer.text(U"text");
    press_menu_select_all();
    press_menu_cut();
    typer.text(U"more ");
    press_menu_paste();
}

TEST_F(AutoTypeKeysTest, shortcut_copy_paste) {
    expected_text = U"hello o hell";
    kbd::AutoType typer;

    // type "hello"
    typer.text(U"hello");
    // "hello"

    // select all
    typer.shortcut(kbd::KeyCode::A);
    typer.shortcut(kbd::KeyCode::C);
    // "[hello]"

    // paste at the end
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.text(U" ");
    typer.shortcut(kbd::KeyCode::V);
    typer.text(U" ");
    // "hello hello "

    // cut "hell"
    for (int i = 0; i < 2; i++) {
        typer.key_press(kbd::KeyCode::LeftArrow);
    }
    for (int i = 0; i < 4; i++) {
        typer.key_press(kbd::KeyCode::LeftArrow, kbd::Modifier::Shift);
    }
    typer.shortcut(kbd::KeyCode::X);
    // "hello [hell]o "

    // paste at the end
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.shortcut(kbd::KeyCode::V);
    // "hello o hell"
}

TEST_F(AutoTypeKeysTest, key_move_single) {
    expected_text = U"a";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);
}

TEST_F(AutoTypeKeysTest, key_move_multiple) {
    expected_text = U"aab";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_shift) {
    expected_text = U"aABb";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Up, kbd::KeyCode::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_right_shift) {
    expected_text = U"aABb";
    expected_events = {
        "keydown 0 - KeyA standard",       "keypress 97 - KeyA standard",
        "keyup 0 - KeyA standard",         "keydown 0 shift ShiftRight right",
        "keydown 0 shift KeyA standard",   "keypress 65 shift KeyA standard",
        "keyup 0 shift KeyA standard",     "keydown 0 shift KeyB standard",
        "keypress 66 shift KeyB standard", "keyup 0 shift KeyB standard",
        "keyup 0 - ShiftRight right",      "keydown 0 - KeyB standard",
        "keypress 98 - KeyB standard",     "keyup 0 - KeyB standard",
    };

    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::RightShift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Up, kbd::KeyCode::RightShift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_all_keys) {
    expected_text = U"0123456789 abcdefghijklmnopqrstuvwxyz\t0123456789\n/-*+\n\\,=`[-.'];/";
    kbd::AutoType typer;

    static constexpr std::array ALL_KEYS{
        kbd::KeyCode::D0,
        kbd::KeyCode::D1,
        kbd::KeyCode::D2,
        kbd::KeyCode::D3,
        kbd::KeyCode::D4,
        kbd::KeyCode::D5,
        kbd::KeyCode::D6,
        kbd::KeyCode::D7,
        kbd::KeyCode::D8,
        kbd::KeyCode::D9,

        kbd::KeyCode::Space,

        kbd::KeyCode::A,
        kbd::KeyCode::B,
        kbd::KeyCode::C,
        kbd::KeyCode::D,
        kbd::KeyCode::E,
        kbd::KeyCode::F,
        kbd::KeyCode::G,
        kbd::KeyCode::H,
        kbd::KeyCode::I,
        kbd::KeyCode::J,
        kbd::KeyCode::K,
        kbd::KeyCode::L,
        kbd::KeyCode::M,
        kbd::KeyCode::N,
        kbd::KeyCode::O,
        kbd::KeyCode::P,
        kbd::KeyCode::Q,
        kbd::KeyCode::R,
        kbd::KeyCode::S,
        kbd::KeyCode::T,
        kbd::KeyCode::U,
        kbd::KeyCode::V,
        kbd::KeyCode::W,
        kbd::KeyCode::X,
        kbd::KeyCode::Y,
        kbd::KeyCode::Z,

        kbd::KeyCode::Tab,

        kbd::KeyCode::Keypad0,
        kbd::KeyCode::Keypad1,
        kbd::KeyCode::Keypad2,
        kbd::KeyCode::Keypad3,
        kbd::KeyCode::Keypad4,
        kbd::KeyCode::Keypad5,
        kbd::KeyCode::Keypad6,
        kbd::KeyCode::Keypad7,
        kbd::KeyCode::Keypad8,
        kbd::KeyCode::Keypad9,

        kbd::KeyCode::Return,

        kbd::KeyCode::KeypadDivide,
        kbd::KeyCode::KeypadMinus,
        kbd::KeyCode::KeypadMultiply,
        kbd::KeyCode::KeypadPlus,
        kbd::KeyCode::KeypadEnter,

        kbd::KeyCode::Backslash,
        kbd::KeyCode::Comma,
        kbd::KeyCode::Equal,
        kbd::KeyCode::Grave,
        kbd::KeyCode::LeftBracket,
        kbd::KeyCode::Minus,
        kbd::KeyCode::Period,
        kbd::KeyCode::Quote,
        kbd::KeyCode::RightBracket,
        kbd::KeyCode::Semicolon,
        kbd::KeyCode::Slash,

        kbd::KeyCode::UpArrow,
        kbd::KeyCode::DownArrow,
        kbd::KeyCode::LeftArrow,
        kbd::KeyCode::RightArrow,

        kbd::KeyCode::Home,
        kbd::KeyCode::End,
        kbd::KeyCode::PageUp,
        kbd::KeyCode::PageDown,

        kbd::KeyCode::X,
        kbd::KeyCode::BackwardDelete,

        kbd::KeyCode::X,
        kbd::KeyCode::LeftArrow,
        kbd::KeyCode::ForwardDelete,

        kbd::KeyCode::Ctrl,
        kbd::KeyCode::Function,
        kbd::KeyCode::Shift,
        kbd::KeyCode::Option,
        kbd::KeyCode::Meta,
        kbd::KeyCode::Meta,
        kbd::KeyCode::Escape,

        kbd::KeyCode::RightControl,
        kbd::KeyCode::RightShift,
        kbd::KeyCode::RightOption,
        kbd::KeyCode::RightMeta,
        kbd::KeyCode::RightMeta,
        kbd::KeyCode::Escape,
    };

    for (auto key_code : ALL_KEYS) {
        auto os_key_code = typer.os_key_code(key_code);
        if (os_key_code.has_value()) {
            typer.key_move(kbd::Direction::Down, key_code);
            typer.key_move(kbd::Direction::Up, key_code);
        }
    }

    wait_millis(500);

    // hide all possible menus
    typer.key_press(kbd::KeyCode::Escape);
    wait_millis(100);
}

} // namespace keyboard_auto_type_test
