#include "catch.hpp"
#include "../LightProfile.h"

SCENARIO("check LightProfile")
{
    GIVEN("Prime LightProfile")
    {
        LightProfile lp;
        WHEN("Number of samples is required")
        {
            auto value = lp.samplesNum();
            THEN("It should be 1")
            {
                CHECK(value == 1);
            }
        }
    }
}
