#include <unistd.h>

#include <array>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class AutoTypeTest : public testing::Test {
  protected:
    const std::string file_name = "build/test/test.txt";
    time_t file_mod_time;
    std::u32string expected_text;

  protected:
    // cppcheck-suppress unusedFunction
    static void SetUpTestSuite() {}

    // cppcheck-suppress unusedFunction
    static void TearDownTestSuite() { kill_text_editor(); }

    // cppcheck-suppress unusedFunction
    virtual void SetUp() { open_text_editor(); }

    // cppcheck-suppress unusedFunction
    virtual void TearDown() {
        save_text();
        wait_for_file_save();
        kill_text_editor();
        check_expected_text();
    }

  private:
    static void kill_text_editor() {
#if __APPLE__
        system("killall TextEdit >/dev/null 2>/dev/null");
#else
        FAIL() << "kill_text_editor is not implemented";
#endif
    }

    void launch_text_editor() {
#if __APPLE__
        system(("open /System/Applications/TextEdit.app " + file_name).c_str());
#else
        FAIL() << "launch_text_editor is not implemented";
#endif
    }

    void wait_text_editor_window() {
        sleep(1);
        // for (auto i = 0; i < 100; i++) {
        //     usleep(100000);
        //     auto active_window = kbd::WindowHelper::active_window();
        //     std::cout << active_window.pid << " ";
        // }
        // FAIL() << "Text editor didn't appear";
    }

    void open_text_editor() {
        create_file();
        kill_text_editor();
        launch_text_editor();
        wait_text_editor_window();
    }

    void create_file() {
        std::filesystem::remove(file_name);
        std::fstream fstream(file_name, std::ios::out);
        // fstream << "\xEF\xBB\xBF";
        fstream.close();

        auto ftime = std::filesystem::last_write_time(file_name);
        file_mod_time = decltype(ftime)::clock::to_time_t(ftime);
    }

    void save_text() {
        kbd::AutoType typer;
        typer.key_press(0, kbd::KeyCode::ANSI_S, typer.shortcut_modifier());
        typer.key_press(0, kbd::KeyCode::ANSI_Q, typer.shortcut_modifier());
    }

    void wait_for_file_save() {
        for (auto i = 0; i < 100; i++) {
            auto ftime = std::filesystem::last_write_time(file_name);
            auto last_mod_time = decltype(ftime)::clock::to_time_t(ftime);
            usleep(100000);
            if (last_mod_time > file_mod_time) {
                return;
            }
        }
        FAIL() << "File date didn't change";
    }

    void check_expected_text() {
        std::ifstream ifs(file_name);
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string actual_text = converter.from_bytes(data);

        ASSERT_EQ(expected_text, actual_text);
    }
};

TEST_F(AutoTypeTest, key_press_letter) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    expected_text = U"a";
}

TEST_F(AutoTypeTest, key_press_two_letters) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    typer.key_press(U'b');
    expected_text = U"ab";
}

TEST_F(AutoTypeTest, key_press_two_lines) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    typer.key_press(U'b');
    typer.key_press(U'\n');
    typer.key_press(U'c');
    typer.key_press(U'd');
    expected_text = U"ab\ncd";
}

TEST_F(AutoTypeTest, key_press_capital) {
    kbd::AutoType typer;
    typer.key_press(U'A');
    typer.key_press(U'b');
    typer.key_press(U'C');
    expected_text = U"AbC";
}

TEST_F(AutoTypeTest, key_press_key_code) {
    kbd::AutoType typer;
    typer.key_press(0, kbd::KeyCode::ANSI_0);
    typer.key_press(0, kbd::KeyCode::ANSI_B);
    expected_text = U"0b";
}

TEST_F(AutoTypeTest, key_press_key_code_with_char) {
    kbd::AutoType typer;
    typer.key_press('0', kbd::KeyCode::ANSI_0);
    typer.key_press('b', kbd::KeyCode::ANSI_B);
    expected_text = U"0b";
}

TEST_F(AutoTypeTest, key_press_key_code_modifier) {
    kbd::AutoType typer;
    typer.key_press(0, kbd::KeyCode::ANSI_1);
    typer.key_press(0, kbd::KeyCode::ANSI_1, kbd::Modifier::Shift);
    typer.key_press(0, kbd::KeyCode::ANSI_C);
    typer.key_press(0, kbd::KeyCode::ANSI_C, kbd::Modifier::Shift);
    expected_text = U"1!cC";
}

TEST_F(AutoTypeTest, key_press_key_code_modifier_with_char) {
    kbd::AutoType typer;
    typer.key_press('1', kbd::KeyCode::ANSI_1);
    typer.key_press('!', kbd::KeyCode::ANSI_1, kbd::Modifier::Shift);
    typer.key_press('c', kbd::KeyCode::ANSI_C);
    typer.key_press('C', kbd::KeyCode::ANSI_C, kbd::Modifier::Shift);
    expected_text = U"1!cC";
}

TEST_F(AutoTypeTest, text_unicode_basic) {
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

    for (auto range : char_ranges) {
        for (auto ch = range.first; ch <= range.second; ch++) {
            expected_text += ch;
        }
        expected_text += U"\n";
    }

    typer.text(expected_text);
}

TEST_F(AutoTypeTest, text_unicode_emoji) {
    expected_text = U"🍆🍑😈";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeTest, text_unicode_supplementary_ideographic) {
    expected_text = U"𠀧𠀪";
    kbd::AutoType typer;
    typer.text(expected_text);
}
