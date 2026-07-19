#ifndef TETROMINO_H
#define TETROMINO_H

#include "Constants.h"
#include <array>
#include <vector>

// 七种标准俄罗斯方块类型
enum class TetrominoType {
    I = 0,
    O = 1,
    T = 2,
    S = 3,
    Z = 4,
    J = 5,
    L = 6,
    COUNT = 7   // 类型总数
};

// 方块类：管理方块的类型、旋转状态、位置和颜色
class Tetromino {
public:
    Tetromino() = default;
    explicit Tetromino(TetrominoType type);

    // 旋转（顺时针/逆时针）
    void rotateCW();
    void rotateCCW();

    // 获取当前旋转状态下4个格子相对于方块原点的偏移坐标 (row, col)
    const std::array<std::pair<int, int>, 4>& getBlockOffsets() const;

    // 获取/设置方块在游戏面板中的位置
    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setRow(int row) { m_row = row; }
    void setCol(int col) { m_col = col; }

    // 移动
    void move(int dr, int dc);

    // 获取类型和旋转状态
    TetrominoType getType() const { return m_type; }
    int getRotation() const { return m_rotation; }

    // 获取颜色
    Color getColor() const;

    // 获取初始生成位置（使方块居中）
    int getSpawnCol() const;

    // 每种方块的颜色（公开供渲染器使用）
    static const std::array<Color, 7> COLORS;

private:
    TetrominoType m_type = TetrominoType::I;
    int m_rotation = 0;
    int m_row = 0;
    int m_col = 0;

    // 形状数据：[类型][旋转状态][4个格子] -> (row, col) 偏移
    static const std::array<std::array<std::array<std::pair<int, int>, 4>, 4>, 7> SHAPES;
};

#endif // TETROMINO_H
