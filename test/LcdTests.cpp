#include <gtest/gtest.h>
#include "../LightProfile.h"

TEST(TestLcdTS, TestLcdCommands)
{
    LightProfile lp(LightProfileName::Alarm);
    auto value = lp.samplesNum();

    ASSERT_EQ(10, value);
}


