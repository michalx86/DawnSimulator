#include "catch.hpp"
#include "../LightProfile.h"

#define ALARM_SAMPLES_NUM 10
#define ALARM_SAMPLE_LAST 9
#define SWITCH_SAMPLES_NUM 120
#define SWITCH_SAMPLE_LAST 119
#define SAMPLE_LAST_VALUE 8191

SCENARIO("check LightProfile")
{
    GIVEN("Prime LightProfile Alarm")
    {
        LightProfile lp(LightProfileName::Alarm);
        WHEN("Number of samples is required")
        {
            auto value = lp.samplesNum();
            THEN("It should be ALARM_SAMPLES_NUM")
            {
                CHECK(value == ALARM_SAMPLES_NUM);
            }
        }
        WHEN("Sample 0 is required")
        {
            auto value = lp[0];
            THEN("It should be 0")
            {
                CHECK(value == 0);
            }
        }
        WHEN("Sample ALARM_SAMPLE_LAST is required")
        {
            auto value = lp[ALARM_SAMPLE_LAST];
            THEN("It should be SAMPLE_LAST_VALUE")
            {
                CHECK(value == SAMPLE_LAST_VALUE);
            }
        }
        WHEN("Percent of Sample 0 is required")
        {
            auto value = lp.toPercent(0);
            THEN("It should be 0")
            {
                CHECK(value == 0);
            }
        }
        WHEN("Percent of Sample ALARM_SAMPLE_LAST is required")
        {
            auto value = lp.toPercent(ALARM_SAMPLE_LAST);
            THEN("It should be 0")
            {
                CHECK(value == 100);
            }
        }
        WHEN("Sample of value higher or equal to 0 is required")
        {
            auto value = lp.sampleHigherOrEqual(0);
            THEN("It should be 0")
            {
                CHECK(value == 0);
            }
        }
        WHEN("Sample of value higher or equal to SAMPLE_LAST_VALUE is required")
        {
            auto value = lp.sampleHigherOrEqual(SAMPLE_LAST_VALUE);
            THEN("It should be ALARM_SAMPLE_LAST")
            {
                CHECK(value == ALARM_SAMPLE_LAST);
            }
        }
    }
    GIVEN("Prime LightProfile Switch")
    {
        LightProfile lp(LightProfileName::Switch);
        WHEN("Number of samples is required")
        {
            auto value = lp.samplesNum();
            THEN("It should be SWITCH_SAMPLES_NUM")
            {
                CHECK(value == SWITCH_SAMPLES_NUM);
            }
        }

        WHEN("Sample 0 is required")
        {
            auto value = lp[0];
            THEN("It should be 0")
            {
                CHECK(value == 0);
            }
        }
        WHEN("Sample SWITCH_SAMPLE_LAST is required")
        {
            auto value = lp[SWITCH_SAMPLE_LAST];
            THEN("It should be SAMPLE_LAST_VALUE")
            {
                CHECK(value == SAMPLE_LAST_VALUE);
            }
        }
    }
}
