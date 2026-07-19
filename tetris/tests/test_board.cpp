#include "doctest.h"
#include "Board.h"
#include "Constants.h"
#include <vector>

// ============================================================
// Board 单元测试
// ============================================================

TEST_CASE("Board: 新面板全部为空") {
    Board board;
    for (int r = 0; r < BOARD_TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            CHECK(board.getCell(r, c) == 0);
        }
    }
    CHECK_FALSE(board.isGameOver());
}

TEST_CASE("Board: reset 清除所有格子") {
    Board board;
    board.setCell(10, 3, 1);
    board.setCell(19, 7, 2);
    board.reset();
    for (int r = 0; r < BOARD_TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            CHECK(board.getCell(r, c) == 0);
        }
    }
}

TEST_CASE("Board: isValidPosition 在空白面板上接受有效位置") {
    Board board;
    Tetromino piece(TetrominoType::T);
    piece.setRow(4);
    piece.setCol(3);
    CHECK(board.isValidPosition(piece));
}

TEST_CASE("Board: isValidPosition 拒绝超出左边界") {
    Board board;
    Tetromino piece(TetrominoType::T);
    piece.setRow(4);
    piece.setCol(-2);
    CHECK_FALSE(board.isValidPosition(piece));
}

TEST_CASE("Board: isValidPosition 拒绝超出右边界") {
    Board board;
    Tetromino piece(TetrominoType::T);
    piece.setRow(4);
    piece.setCol(9);
    // T 方块宽 3 列，col=9 时会越界
    CHECK_FALSE(board.isValidPosition(piece));
}

TEST_CASE("Board: isValidPosition 拒绝超出下边界") {
    Board board;
    Tetromino piece(TetrominoType::I);
    piece.setRow(BOARD_TOTAL_ROWS - 1);
    piece.setCol(3);
    CHECK_FALSE(board.isValidPosition(piece));
}

TEST_CASE("Board: isValidPosition 拒绝重叠") {
    Board board;
    // O 方块 at (5,4), 偏移 {0,1},{0,2},{1,1},{1,2} → 占据 (5,5)(5,6)(6,5)(6,6)
    board.setCell(6, 5, 1);
    Tetromino piece(TetrominoType::O);
    piece.setRow(5);
    piece.setCol(4);
    CHECK_FALSE(board.isValidPosition(piece));
}

TEST_CASE("Board: lockPiece 正确锁定方块") {
    Board board;
    Tetromino piece(TetrominoType::O);
    piece.setRow(16);
    piece.setCol(4);
    board.lockPiece(piece);

    // O 方块占用 (0,0)(0,1)(1,0)(1,1) 偏移
    const auto& offsets = piece.getBlockOffsets();
    for (const auto& block : offsets) {
        int r = piece.getRow() + block.first;
        int c = piece.getCol() + block.second;
        int expectedType = static_cast<int>(TetrominoType::O) + 1;
        CHECK(board.getCell(r, c) == expectedType);
    }
}

TEST_CASE("Board: lockPiece 颜色索引正确") {
    Board board;
    Tetromino pieceI(TetrominoType::I);
    pieceI.setRow(16);
    pieceI.setCol(3);
    board.lockPiece(pieceI);
    // I 方块偏移 {1,0},{1,1},{1,2},{1,3} → 占据 (17,3)(17,4)(17,5)(17,6)
    CHECK(board.getCell(17, 3) == 1); // I 的索引为 0+1

    Board board2;
    Tetromino pieceL(TetrominoType::L);
    pieceL.setRow(16);
    pieceL.setCol(3);
    board2.lockPiece(pieceL);
    // L 方块偏移 {0,2},{1,0},{1,1},{1,2} → 包含 (16,5)
    CHECK(board2.getCell(16, 5) == 7); // L 的索引为 6+1
}

TEST_CASE("Board: isRowFull 检测完整行") {
    Board board;
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(18, c, 1);
    }
    CHECK(board.isRowFull(18));
    CHECK_FALSE(board.isRowFull(17));
}

TEST_CASE("Board: clearLines 空面板返回 0") {
    Board board;
    std::vector<int> cleared;
    CHECK(board.clearLines(cleared) == 0);
    CHECK(cleared.empty());
}

TEST_CASE("Board: clearLines 消除一行") {
    Board board;
    // 填充第 19 行（最底部）
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(19, c, 1);
    }
    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    CHECK(lines == 1);
    CHECK(cleared.size() == 1);

    // 该行现在应为空
    for (int c = 0; c < BOARD_COLS; ++c) {
        CHECK(board.getCell(19, c) == 0);
    }
}

TEST_CASE("Board: clearLines 消除后上方行下移") {
    Board board;
    // 在第 18 行放一个方块
    board.setCell(18, 5, 2);
    // 填充第 19 行
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(19, c, 1);
    }
    std::vector<int> cleared;
    board.clearLines(cleared);
    // 原来的方块应从 18 行下移到 19 行
    CHECK(board.getCell(19, 5) == 2);
    CHECK(board.getCell(18, 5) == 0);
}

TEST_CASE("Board: clearLines 消除多行") {
    Board board;
    for (int r = 17; r <= 19; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            board.setCell(r, c, 1);
        }
    }
    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    CHECK(lines == 3);
}

TEST_CASE("Board: clearLines 只消除完整行") {
    Board board;
    // 第 19 行完整
    for (int c = 0; c < BOARD_COLS; ++c) {
        board.setCell(19, c, 1);
    }
    // 第 18 行不完整
    board.setCell(18, 0, 1);
    board.setCell(18, 3, 1);

    std::vector<int> cleared;
    int lines = board.clearLines(cleared);
    CHECK(lines == 1);
    CHECK(board.getCell(18, 0) == 0); // 第 18 行下移后
}

TEST_CASE("Board: clearRow 清空指定行并下移") {
    Board board;
    board.setCell(5, 5, 3);
    board.setCell(6, 5, 4);
    board.clearRow(6);
    // row 6 被 row 5 覆盖，row 5 被 row 4 覆盖（row 4 为空）
    CHECK(board.getCell(5, 5) == 0); // row 5 的内容被 row 4 覆盖
    CHECK(board.getCell(6, 5) == 3); // row 6 得到原 row 5 的内容
}

TEST_CASE("Board: isGameOver 初始为 false") {
    Board board;
    CHECK_FALSE(board.isGameOver());
}

TEST_CASE("Board: isGameOver 顶部行有方块时返回 true") {
    Board board;
    board.setCell(0, 5, 1);
    CHECK(board.isGameOver());
}

TEST_CASE("Board: setCell/getCell 边界处理") {
    Board board;
    board.setCell(-1, 0, 1);  // 负行，应忽略
    board.setCell(0, -1, 1);  // 负列，应忽略
    board.setCell(30, 0, 1);  // 超出行，应忽略
    board.setCell(0, 15, 1);  // 超出列，应忽略
    for (int r = 0; r < BOARD_TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            CHECK(board.getCell(r, c) == 0);
        }
    }
}

TEST_CASE("Board: getCell 边界检测返回 0") {
    Board board;
    CHECK(board.getCell(-1, 0) == 0);
    CHECK(board.getCell(0, -1) == 0);
    CHECK(board.getCell(30, 0) == 0);
    CHECK(board.getCell(0, 20) == 0);
}
