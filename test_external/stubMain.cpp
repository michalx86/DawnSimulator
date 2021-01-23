#include <iostream>
#include "../LightProfile.h"

int main()
{
    std::cout << "Result: " << LightProfile(LightProfileName::Alarm).samplesNum() << std::endl;
    return 0;
}
