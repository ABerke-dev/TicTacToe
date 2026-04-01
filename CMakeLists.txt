cmake_minimum_required(VERSION 3.15)
project(TicTacToe VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()


find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

set(SOURCES
    src/Board.cpp
    src/Player.cpp
    src/HumanPlayer.cpp
    src/AIPlayer.cpp
    src/GameEngine.cpp
    src/main.cpp
)

set(HEADERS
    inc/CellState.hpp
    inc/GameState.hpp
    inc/Board.hpp
    inc/Player.hpp
    inc/HumanPlayer.hpp
    inc/AIPlayer.hpp
    inc/GameEngine.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W4
        /utf-8
        /permissive-
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  TicTacToe Battle Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "SDL2 Found: ${SDL2_FOUND}")
message(STATUS "========================================")
message(STATUS "")