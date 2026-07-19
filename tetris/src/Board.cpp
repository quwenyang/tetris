#include "Board.h"

// ============================================================
// 构造函数：初始化面板为空
// ============================================================
Board::Board() {
    reset();
}

// ============================================================
// 重置面板：所有格子置空
// ============================================================
void Board::reset() {
    for (int r = 0; r < BOARD_TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            m_grid[r][c] = 0;
        }
    }
}

// ============================================================
// 检查方块在指定位置和旋转状态下是否有效（无碰撞）
// ============================================================
bool Board::isValidPosition(const Tetromino& piece, int row, int col, int rotation) const {
    // 获取方块4个格子的偏移坐标
    const auto& offsets = piece.getBlockOffsets();  // 注意：这里需要根据rotation获取

    // 实际上我们需要根据传入的 rotation 获取偏移
    // 由于 Tetromino 没有提供按指定旋转获取偏移的方法，我们直接用 piece 的当前旋转
    // 由调用方保证在调用前设置好旋转

    // 获取当前方块的4个格子偏移
    const auto& blocks = piece.getBlockOffsets();

    for (const auto& block : blocks) {
        int r = row + block.first;
        int c = col + block.second;

        // 检查边界
        if (r < 0 || r >= BOARD_TOTAL_ROWS || c < 0 || c >= BOARD_COLS) {
            return false;
        }

        // 检查是否与其他方块重叠
        if (m_grid[r][c] != 0) {
            return false;
        }
    }
    return true;
}

bool Board::isValidPosition(const Tetromino& piece) const {
    return isValidPosition(piece, piece.getRow(), piece.getCol(), piece.getRotation());
}

// ============================================================
// 将方块锁定到面板中
// ============================================================
void Board::lockPiece(const Tetromino& piece) {
    const auto& blocks = piece.getBlockOffsets();
    int colorIdx = static_cast<int>(piece.getType()) + 1;  // +1 因为0表示空格

    for (const auto& block : blocks) {
        int r = piece.getRow() + block.first;
        int c = piece.getCol() + block.second;

        if (r >= 0 && r < BOARD_TOTAL_ROWS && c >= 0 && c < BOARD_COLS) {
            m_grid[r][c] = colorIdx;
        }
    }
}

// ============================================================
// 消除完整的行，返回消除的行数
// ============================================================
int Board::clearLines(std::vector<int>& clearedRows) {
    clearedRows.clear();

    // 从底部向上检查每一行
    for (int r = BOARD_TOTAL_ROWS - 1; r >= 0; --r) {
        if (isRowFull(r)) {
            clearedRows.push_back(r);
        }
    }

    // 从下往上依次消除行（按行号升序处理，避免多次移位干扰）
    for (auto it = clearedRows.rbegin(); it != clearedRows.rend(); ++it) {
        clearRow(*it);
    }

    return static_cast<int>(clearedRows.size());
}

// ============================================================
// 检查指定行是否完整
// ============================================================
bool Board::isRowFull(int row) const {
    for (int c = 0; c < BOARD_COLS; ++c) {
        if (m_grid[row][c] == 0) {
            return false;
        }
    }
    return true;
}

// ============================================================
// 清除指定行，上方所有行下移一行
// ============================================================
void Board::clearRow(int row) {
    // 从该行开始，将上方所有行下移
    for (int r = row; r > 0; --r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            m_grid[r][c] = m_grid[r - 1][c];
        }
    }

    // 最顶行置空
    for (int c = 0; c < BOARD_COLS; ++c) {
        m_grid[0][c] = 0;
    }
}

// ============================================================
// 检查游戏是否结束（顶部隐藏行有方块进入可视区域）
// ============================================================
bool Board::isGameOver() const {
    // 如果顶部隐藏行有方块，游戏结束
    // 检查 BOARD_HIDDEN_ROWS - 1（即隐藏行最下面一行）是否有方块
    for (int r = 0; r < BOARD_HIDDEN_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_grid[r][c] != 0) {
                return true;
            }
        }
    }
    return false;
}

// ============================================================
// 获取指定格子的颜色索引
// ============================================================
int Board::getCell(int row, int col) const {
    if (row < 0 || row >= BOARD_TOTAL_ROWS || col < 0 || col >= BOARD_COLS) {
        return 0;
    }
    return m_grid[row][col];
}

// ============================================================
// 设置指定格子的颜色索引
// ============================================================
void Board::setCell(int row, int col, int value) {
    if (row >= 0 && row < BOARD_TOTAL_ROWS && col >= 0 && col < BOARD_COLS) {
        m_grid[row][col] = value;
    }
}
