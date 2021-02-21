cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DawnSimulator)

find_package(GTest REQUIRED)
message("GTest_INCLUDE_DIRS = ${GTest_INCLUDE_DIRS}")

set(SRC_LIST
    ../lib/LightProfile/src/LightProfile.cpp
)

set(INCLUDE_LIST
    ../lib/utils
)

add_library(${PROJECT_NAME}-lib STATIC ${SRC_LIST})
target_compile_options(${PROJECT_NAME}-lib PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${CMAKE_SOURCE_DIR} ${INCLUDE_LIST})

add_executable(${PROJECT_NAME} stubMain.cpp)
target_compile_options(${PROJECT_NAME} PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${INCLUDE_LIST})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-lib)

set(TEST_SRC_LIST
    testMain.cpp
    LightProfileTests.cpp
)
add_executable(${PROJECT_NAME}-ut ${TEST_SRC_LIST})
target_compile_options(${PROJECT_NAME}-ut PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME}-ut PUBLIC ${CMAKE_SOURCE_DIR} ${INCLUDE_LIST})
target_link_libraries(${PROJECT_NAME}-ut ${PROJECT_NAME}-lib)

set(GTEST_SRC_LIST
    gtestMain.cpp
    LcdTests.cpp
)
add_executable(${PROJECT_NAME}-gut ${GTEST_SRC_LIST})
target_compile_options(${PROJECT_NAME}-gut PUBLIC -g -O0 -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-variable)
target_include_directories(${PROJECT_NAME}-gut PUBLIC ${CMAKE_SOURCE_DIR} ${INCLUDE_LIST})
target_link_libraries(${PROJECT_NAME}-gut ${PROJECT_NAME}-lib ${GTEST_LIBRARIES} pthread)
