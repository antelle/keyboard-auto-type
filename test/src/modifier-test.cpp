#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

namespace keyboard_auto_type_test {

class ModifierTest : public testing::Test {};

TEST_F(ModifierTest, modifier_none) { ASSERT_EQ(0, static_cast<uint8_t>(kbd::Modifier::None)); }

TEST_F(ModifierTest, modifier_ctrl) {
    ASSERT_NE(kbd::Modifier::None, kbd::Modifier::Ctrl);
    ASSERT_EQ(kbd::Modifier::Ctrl, kbd::Modifier::Control);

    ASSERT_NE(kbd::Modifier::Ctrl, kbd::Modifier::LeftCtrl);
    ASSERT_EQ(kbd::Modifier::LeftCtrl, kbd::Modifier::LeftControl);
    ASSERT_EQ(kbd::Modifier::Ctrl, kbd::Modifier::LeftCtrl & kbd::Modifier::Ctrl);

    ASSERT_NE(kbd::Modifier::Ctrl, kbd::Modifier::RightCtrl);
    ASSERT_EQ(kbd::Modifier::RightCtrl, kbd::Modifier::RightControl);
    ASSERT_EQ(kbd::Modifier::Ctrl, kbd::Modifier::RightCtrl & kbd::Modifier::Ctrl);

    ASSERT_NE(kbd::Modifier::RightCtrl, kbd::Modifier::LeftCtrl);
}

TEST_F(ModifierTest, modifier_alt) {
    ASSERT_NE(kbd::Modifier::None, kbd::Modifier::Alt);
    ASSERT_EQ(kbd::Modifier::Alt, kbd::Modifier::Option);

    ASSERT_NE(kbd::Modifier::Alt, kbd::Modifier::LeftAlt);
    ASSERT_EQ(kbd::Modifier::LeftAlt, kbd::Modifier::LeftOption);
    ASSERT_EQ(kbd::Modifier::Alt, kbd::Modifier::LeftAlt & kbd::Modifier::Alt);

    ASSERT_NE(kbd::Modifier::Alt, kbd::Modifier::RightAlt);
    ASSERT_EQ(kbd::Modifier::RightAlt, kbd::Modifier::RightOption);
    ASSERT_EQ(kbd::Modifier::Alt, kbd::Modifier::RightAlt & kbd::Modifier::Alt);

    ASSERT_NE(kbd::Modifier::RightAlt, kbd::Modifier::LeftAlt);
}

TEST_F(ModifierTest, modifier_shift) {
    ASSERT_NE(kbd::Modifier::None, kbd::Modifier::Shift);

    ASSERT_NE(kbd::Modifier::Shift, kbd::Modifier::LeftShift);
    ASSERT_EQ(kbd::Modifier::Shift, kbd::Modifier::LeftShift & kbd::Modifier::Shift);

    ASSERT_NE(kbd::Modifier::Shift, kbd::Modifier::RightShift);
    ASSERT_EQ(kbd::Modifier::Shift, kbd::Modifier::RightShift & kbd::Modifier::Shift);

    ASSERT_NE(kbd::Modifier::RightShift, kbd::Modifier::LeftShift);
}

TEST_F(ModifierTest, modifier_meta) {
    ASSERT_NE(kbd::Modifier::None, kbd::Modifier::Meta);
    ASSERT_EQ(kbd::Modifier::Meta, kbd::Modifier::Command);
    ASSERT_EQ(kbd::Modifier::Meta, kbd::Modifier::Win);

    ASSERT_NE(kbd::Modifier::Meta, kbd::Modifier::LeftMeta);
    ASSERT_EQ(kbd::Modifier::LeftMeta, kbd::Modifier::LeftCommand);
    ASSERT_EQ(kbd::Modifier::LeftMeta, kbd::Modifier::LeftWin);
    ASSERT_EQ(kbd::Modifier::Meta, kbd::Modifier::LeftMeta & kbd::Modifier::Meta);

    ASSERT_NE(kbd::Modifier::Meta, kbd::Modifier::RightMeta);
    ASSERT_EQ(kbd::Modifier::RightMeta, kbd::Modifier::RightCommand);
    ASSERT_EQ(kbd::Modifier::RightMeta, kbd::Modifier::RightWin);
    ASSERT_EQ(kbd::Modifier::Meta, kbd::Modifier::RightMeta & kbd::Modifier::Meta);

    ASSERT_NE(kbd::Modifier::RightMeta, kbd::Modifier::LeftMeta);
}

TEST_F(ModifierTest, modifier_or) {
    auto mod = kbd::Modifier::Ctrl | kbd::Modifier::Alt;
    ASSERT_EQ(kbd::Modifier::Alt, mod & kbd::Modifier::Alt);
    ASSERT_EQ(kbd::Modifier::Ctrl, mod & kbd::Modifier::Ctrl);
    ASSERT_EQ(kbd::Modifier::None, mod & kbd::Modifier::Shift);
    ASSERT_EQ(kbd::Modifier::None, mod & kbd::Modifier::Meta);
}

} // namespace keyboard_auto_type_test
