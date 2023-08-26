cmake_minimum_required(VERSION 3.10.0)
project(input_overlay VERSION 0.1.0)

add_compile_options(-Wall -Wextra -Wpedantic)

include(CTest)
enable_testing()

add_executable(input_overlay
        main.c
        config.c
)

target_link_libraries(input_overlay -lSDL2 -lSDL2_image)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
