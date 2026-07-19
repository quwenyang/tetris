#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

// 颜色结构体（替代 SFML 的 sf::Color）
struct Color {
    uint8_t r, g, b, a;
    constexpr Color() : r(0), g(0), b(0), a(255) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
};

// 游戏面板尺寸（格子数）
constexpr int BOARD_COLS = 10;
constexpr int BOARD_ROWS = 20;
constexpr int BOARD_HIDDEN_ROWS = 4;   // 顶部隐藏行，用于方块生成
constexpr int BOARD_TOTAL_ROWS = BOARD_ROWS + BOARD_HIDDEN_ROWS;

// 每个格子的像素大小
constexpr int CELL_SIZE = 30;

// 窗口尺寸
constexpr int BOARD_PIXEL_WIDTH  = BOARD_COLS * CELL_SIZE;
constexpr int BOARD_PIXEL_HEIGHT = BOARD_ROWS * CELL_SIZE;
constexpr int SIDE_PANEL_WIDTH   = 150;
constexpr int WINDOW_WIDTH  = BOARD_PIXEL_WIDTH + SIDE_PANEL_WIDTH;
constexpr int WINDOW_HEIGHT = BOARD_PIXEL_HEIGHT;

// 游戏速度（毫秒）
constexpr int BASE_DROP_INTERVAL = 800;     // 初始下落间隔
constexpr int MIN_DROP_INTERVAL  = 50;       // 最快下落间隔
constexpr int LINES_PER_LEVEL    = 10;       // 每消除10行升一级
constexpr int SOFT_DROP_INTERVAL = 50;       // 软加速下落间隔

// 方块大小（每个方块由4个小格子组成）
constexpr int TETROMINO_SIZE = 4;

// 颜色常量
constexpr Color COLOR_BLACK(0, 0, 0);
constexpr Color COLOR_DARK(30, 30, 30);
constexpr Color COLOR_BG(20, 20, 40);
constexpr Color COLOR_GRID(40, 40, 60);
constexpr Color COLOR_BORDER(100, 100, 100);
constexpr Color COLOR_TEXT(255, 255, 255);
constexpr Color COLOR_TEXT_DIM(180, 180, 180);
constexpr Color COLOR_TEXT_DARK(140, 140, 140);
constexpr Color COLOR_OVERLAY(0, 0, 0);
constexpr Color COLOR_CYAN(0, 255, 255);
constexpr Color COLOR_YELLOW(255, 255, 0);
constexpr Color COLOR_PURPLE(128, 0, 128);
constexpr Color COLOR_GREEN(0, 255, 0);
constexpr Color COLOR_RED(255, 0, 0);
constexpr Color COLOR_BLUE(0, 0, 255);
constexpr Color COLOR_ORANGE(255, 165, 0);

// 文件路径
constexpr char HIGHSCORE_FILE[] = "highscore.dat";

#endif // CONSTANTS_H
