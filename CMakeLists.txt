cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(step1 step1/dasher.cpp)
target_compile_features(step1 PRIVATE cxx_std_17)
target_link_libraries(step1 PRIVATE SFML::Graphics)

add_executable(step2 step2/dasher.cpp)
target_compile_features(step2 PRIVATE cxx_std_17)
target_link_libraries(step2 PRIVATE SFML::Graphics)

add_executable(step3 step3/dasher.cpp)
target_compile_features(step3 PRIVATE cxx_std_17)
target_link_libraries(step3 PRIVATE SFML::Graphics)

add_executable(step4 step4/dasher.cpp)
target_compile_features(step4 PRIVATE cxx_std_17)
target_link_libraries(step4 PRIVATE SFML::Graphics)

add_executable(step5 step5/dasher.cpp)
target_compile_features(step5 PRIVATE cxx_std_17)
target_link_libraries(step5 PRIVATE SFML::Graphics)

add_executable(step6 step6/dasher.cpp)
target_compile_features(step6 PRIVATE cxx_std_17)
target_link_libraries(step6 PRIVATE SFML::Graphics)

add_executable(step7 step7/dasher.cpp)
target_compile_features(step7 PRIVATE cxx_std_17)
target_link_libraries(step7 PRIVATE SFML::Graphics)

add_executable(step8 step8/dasher.cpp step8/entities.cpp)
target_compile_features(step8 PRIVATE cxx_std_17)
target_link_libraries(step8 PRIVATE SFML::Graphics)

add_executable(step9 step9/dasher.cpp step9/entities.cpp)
target_compile_features(step9 PRIVATE cxx_std_17)
target_link_libraries(step9 PRIVATE SFML::Graphics)

add_executable(step10 step10/dasher.cpp step10/entities.cpp)
target_compile_features(step10 PRIVATE cxx_std_17)
target_link_libraries(step10 PRIVATE SFML::Graphics)

add_executable(dasher src/dasher.cpp src/entities.cpp)
target_compile_features(dasher PRIVATE cxx_std_17)
target_link_libraries(dasher PRIVATE SFML::Graphics SFML::Audio)
