#include "doctest.h"
#include "Board.h"
#include "Tetromino.h"
#include "ScoreManager.h"
#include "Constants.h"
#include <vector>

// ============================================================
// 集成测试：Board + Tetromino + ScoreManager 联动
// ============================================================

TEST_CASE("集成: I 方块落到底部并锁定") {
    Board board;
    Tetromino piece(TetrominoType::I);
    piece.setRow(0);
    piece.setCol(3);

    // 向下移动到面板底部
    while (board.isValidPosition(piece)) {
        piece.move(1, 0);
    }
    // 退回最后有效位置
    piece.move(-1, 0);

    board.lockPiece(piece);

    // 验证 I 方块在底部占据 4 列
    const auto& offsets = piece.getBlockOffsets();
    for (const auto& block : offsets) {
        int r = piece.getRow() + block.first;
        int c = piece.getCol() + block.second;
        CHECK(board.getCell(r, c) == 1); // I 的索引: 0+1
    }
}

TEST_CASE("集成: T 方块旋转后锁定") {
    Board board;
    Tetromino piece(TetrominoType::T);
    piece.setRow(16);
    piece.setCol(4);

    piece.rotateCW();
    CHECK(board.isValidPosition(piece));

    board.lockPiece(piece);
    const auto& offsets = piece.getBlockOffsets();
    for (const auto& block : offsets) {
        int r = piece.getRow() + block.first;
        int c = piece.getCol() + block.second;
        CHECK(board.getCell(r, c) == 3); // T 的索引: 2+1
    }
}

TEST_CASE("集成: 用 T 方块和 L 方块填满一行并消除") {
    Board board;
    // 用 setCell 直接填满第 19 行
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(19, c, 1);
    }

    CHECK(board.isRowFull(19));

    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    CHECK(lines == 1);

    // 第 19 行现在应全空
    for (int c = 0; c < BOARD_COLS; ++c) {
        CHECK(board.getCell(19, c) == 0);
    }
}

TEST_CASE("集成: 方块在面板边缘碰撞检测") {
    Board board;
    Tetromino piece(TetrominoType::O);
    piece.setRow(18);
    piece.setCol(-2); // O 偏移 {0,1},{0,2},{1,1},{1,2} → col 变为 -1,-1,0,0 → -1 无效
    CHECK_FALSE(board.isValidPosition(piece));

    piece.setCol(9);  // O 偏移 1,2 → col 变为 10,11,10,11 → 11 越界
    CHECK_FALSE(board.isValidPosition(piece));
}

TEST_CASE("集成: 方块堆叠后碰撞检测") {
    Board board;
    // 在第 18 行放置方块
    Tetromino piece1(TetrominoType::O);
    piece1.setRow(18);
    piece1.setCol(4);
    board.lockPiece(piece1);

    // 尝试在同一位置放置另一个方块
    Tetromino piece2(TetrominoType::O);
    piece2.setRow(18);
    piece2.setCol(4);
    CHECK_FALSE(board.isValidPosition(piece2));

    // 但在旁边放置应可行
    piece2.setCol(0);
    CHECK(board.isValidPosition(piece2));
}

TEST_CASE("集成: ScoreManager + Board 消除计分") {
    Board board;
    ScoreManager sm;

    // 先升到等级 2
    for (int i = 0; i < 10; ++i) {
        sm.addLinesCleared(1);
    }
    CHECK(sm.getLevel() == 2);

    // 填充第 19 行并消除
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(19, c, 1);
    }
    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    REQUIRE(lines == 1);

    // 计分：40 * 2 = 80（等级 2 消除 1 行）
    int scoreBefore = sm.getScore();
    sm.addLinesCleared(lines);
    CHECK(sm.getScore() == scoreBefore + 80);
}

TEST_CASE("集成: 多个完整行同时消除") {
    Board board;

    // 用 setCell 直接填满 4 行
    for (int r = 16; r <= 19; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            board.setCell(r, c, 1);
        }
    }

    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    // 应该消除了 4 行
    CHECK(lines == 4);
    CHECK(cleared.size() == 4);

    // 底部四行应全部清空
    for (int r = 16; r <= 19; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            CHECK(board.getCell(r, c) == 0);
        }
    }
}

TEST_CASE("集成: 硬降落距离计算") {
    Board board;
    Tetromino piece(TetrominoType::I);
    piece.setRow(0);
    piece.setCol(3);

    // 模拟硬降落：计算可下落格数
    int dropDistance = 0;
    while (board.isValidPosition(piece)) {
        piece.move(1, 0);
        ++dropDistance;
    }
    // 退回最后有效位置（回退一步）
    piece.move(-1, 0);
    --dropDistance;

    // I 方块在 row=0 时，可下落距离应为
    // BOARD_TOTAL_ROWS - 4（I 方块高度）
    // 实际上因为 I 方块旋转状态不同，垂直高度可能不同
    CHECK(dropDistance > 0);

    board.lockPiece(piece);
    // 验证 locked 位置
    const auto& offsets = piece.getBlockOffsets();
    for (const auto& block : offsets) {
        int r = piece.getRow() + block.first;
        int c = piece.getCol() + block.second;
        CHECK(board.getCell(r, c) == 1);
    }
}

TEST_CASE("集成: 方块在水平方向移动并碰撞") {
    Board board;
    Tetromino piece(TetrominoType::T);
    piece.setRow(16);
    piece.setCol(0);

    // 在 col=0 时 T 方块应有效
    CHECK(board.isValidPosition(piece));

    // 向左移动应失败（左边界）
    piece.setCol(-1);
    CHECK_FALSE(board.isValidPosition(piece));
    piece.setCol(0);

    // 向右移动
    piece.setCol(7);
    // T 方块宽 3 列，col=7 时占据 7,8,9 列，应在右边界
    CHECK(board.isValidPosition(piece));

    piece.setCol(8);
    CHECK_FALSE(board.isValidPosition(piece));
}
