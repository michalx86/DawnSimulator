/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#include "LightProfile.h"
#include "LightComposite.h"
#include <unity.h>

#define SAMPLE_LAST_VALUE 8191

LightProfile lp(LightProfileName::Alarm);
LightComposite lc(lp, 1);

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_function_lightprofile_get0percent(void) {
    TEST_ASSERT_EQUAL(0, lc.getPercent());
}

void test_function_lightprofile_getMaxPercent(void) {
    lc.setLevelToMax();
    Color_t color;
    color[0]= SAMPLE_LAST_VALUE;
    color[1]= SAMPLE_LAST_VALUE;
    color[2]= SAMPLE_LAST_VALUE;
    color[3]= SAMPLE_LAST_VALUE;
    color[4]= SAMPLE_LAST_VALUE;
    lc.setTargetValue(color);
    TEST_ASSERT_EQUAL(100, lc.getPercent());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_function_lightprofile_get0percent);
    RUN_TEST(test_function_lightprofile_getMaxPercent);
    UNITY_END();

    return 0;
}
