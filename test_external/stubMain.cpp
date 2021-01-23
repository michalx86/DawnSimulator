#include <iostream>
#include "../lib/LightProfile/src/LightProfile.h"

int main()
{
    std::cout << "Result: " << LightProfile(LightProfileName::Alarm).samplesNum() << std::endl;
    return 0;
}
