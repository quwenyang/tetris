cmake_minimum_required(VERSION 3.20)
project(Tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 游戏源文件
set(SOURCES
    main.cpp
    src/Constants.h
    src/Tetromino.h
    src/Tetromino.cpp
    src/Board.h
    src/Board.cpp
    src/ScoreManager.h
    src/ScoreManager.cpp
    src/Renderer.h
    src/Renderer.cpp
    src/InputHandler.h
    src/InputHandler.cpp
    src/Game.h
    src/Game.cpp
)

add_executable(tetris WIN32 ${SOURCES})
target_link_libraries(tetris PRIVATE gdi32 user32)
target_include_directories(tetris PRIVATE src)

# ============================================================
# 测试目标（纯逻辑，无需 Win32）
# ============================================================
set(TEST_SOURCES
    tests/main.cpp
    tests/test_board.cpp
    tests/test_tetromino.cpp
    tests/test_score.cpp
    tests/test_integration.cpp
    src/Board.cpp
    src/Tetromino.cpp
    src/ScoreManager.cpp
)

add_executable(tetris_tests ${TEST_SOURCES})
target_include_directories(tetris_tests PRIVATE src tests)
set_target_properties(tetris_tests PROPERTIES WIN32_EXECUTABLE FALSE)
