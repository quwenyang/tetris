#ifndef BOARD_H
#define BOARD_H

#include "Constants.h"
#include "Tetromino.h"
#include <array>
#include <vector>

// 游戏面板类：管理 10x20 网格、碰撞检测、行消除
class Board {
public:
    Board();

    // 重置面板
    void reset();

    // 检查当前方块是否可以放置在指定位置
    bool isValidPosition(const Tetromino& piece, int row, int col, int rotation) const;
    bool isValidPosition(const Tetromino& piece) const;

    // 将当前方块锁定到面板中
    void lockPiece(const Tetromino& piece);

    // 消除完整的行，返回消除的行数和消除的行索引
    int clearLines(std::vector<int>& clearedRows);

    // 检查游戏是否结束（方块堆叠到顶部）
    bool isGameOver() const;

    // 检查指定行是否完整
    bool isRowFull(int row) const;

    // 清除指定行的数据，上方所有行下移
    void clearRow(int row);

    // 获取面板中格子的颜色索引（0=空）
    int getCell(int row, int col) const;

    // 设置面板中格子的颜色索引
    void setCell(int row, int col, int value);

    // 获取面板数据（用于渲染）
    const std::array<std::array<int, BOARD_COLS>, BOARD_TOTAL_ROWS>& getGrid() const { return m_grid; }

private:
    // 游戏面板数据：0=空格，1-7=对应方块类型的颜色索引
    // m_grid[0][*] 是最顶部（隐藏行），m_grid[BOARD_TOTAL_ROWS-1][*] 是最底部
    std::array<std::array<int, BOARD_COLS>, BOARD_TOTAL_ROWS> m_grid;
};

#endif // BOARD_H
