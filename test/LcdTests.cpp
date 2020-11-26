#include <gtest/gtest.h>

#define String std::string
#define byte uint8_t
#include "../Lcd.h"

TEST(TestLcd, TestLcdPrint)
{
    Lcd lcd;
    auto value = lcd.print("");

    ASSERT_EQ(0, value);
}



