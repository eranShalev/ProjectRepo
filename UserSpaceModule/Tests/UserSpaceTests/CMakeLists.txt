include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file(GLOB tests *.cpp)

add_executable(UserSpaceTests ${tests})

target_link_libraries(UserSpaceTests gtest gtest_main)
