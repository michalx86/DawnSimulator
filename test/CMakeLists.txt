cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DawnSimulator)

set(SRC_LIST
    ../LightProfile.cpp
)

add_library(${PROJECT_NAME}-lib STATIC ${SRC_LIST})
add_executable(${PROJECT_NAME} stubMain.cpp)
target_compile_options(${PROJECT_NAME} PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-lib)

set(TEST_SRC_LIST
    testMain.cpp
    LightProfileTests.cpp
)
add_executable(${PROJECT_NAME}-ut ${TEST_SRC_LIST})
target_compile_options(${PROJECT_NAME}-ut PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME}-ut PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}-ut ${PROJECT_NAME}-lib)
