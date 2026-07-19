#include "doctest.h"
#include "Tetromino.h"
#include "Constants.h"
#include <set>

// ============================================================
// Tetromino 单元测试
// ============================================================

TEST_CASE("Tetromino: 所有 7 种类型均可创建") {
    for (int t = 0; t < static_cast<int>(TetrominoType::COUNT); ++t) {
        TetrominoType type = static_cast<TetrominoType>(t);
        CHECK_NOTHROW(Tetromino(type));
    }
}

TEST_CASE("Tetromino: 每种类型有 4 个格子偏移") {
    for (int t = 0; t < static_cast<int>(TetrominoType::COUNT); ++t) {
        Tetromino piece(static_cast<TetrominoType>(t));
        const auto& offsets = piece.getBlockOffsets();
        REQUIRE(offsets.size() == 4);
    }
}

TEST_CASE("Tetromino: 每种类型的 4 个格子无重复") {
    for (int t = 0; t < static_cast<int>(TetrominoType::COUNT); ++t) {
        Tetromino piece(static_cast<TetrominoType>(t));
        const auto& offsets = piece.getBlockOffsets();
        std::set<std::pair<int, int>> unique;
        for (const auto& block : offsets) {
            unique.insert(block);
        }
        CHECK(unique.size() == 4);
    }
}

TEST_CASE("Tetromino: 旋转四次回到初始状态") {
    Tetromino piece(TetrominoType::T);
    const auto& initial = piece.getBlockOffsets();

    for (int i = 0; i < 4; ++i) {
        piece.rotateCW();
    }
    const auto& afterFour = piece.getBlockOffsets();
    for (size_t j = 0; j < 4; ++j) {
        CHECK(afterFour[j] == initial[j]);
    }
}

TEST_CASE("Tetromino: O 方块旋转后不变") {
    Tetromino piece(TetrominoType::O);
    const auto& initial = piece.getBlockOffsets();

    for (int i = 0; i < 4; ++i) {
        piece.rotateCW();
        const auto& current = piece.getBlockOffsets();
        for (size_t j = 0; j < 4; ++j) {
            CHECK(current[j] == initial[j]);
        }
    }
}

TEST_CASE("Tetromino: I 方块旋转改变形状") {
    Tetromino piece(TetrominoType::I);
    const auto& r0 = piece.getBlockOffsets();
    piece.rotateCW();
    const auto& r1 = piece.getBlockOffsets();

    // I 方块旋转后偏移应不同
    bool same = true;
    for (size_t j = 0; j < 4 && same; ++j) {
        if (r1[j] != r0[j]) same = false;
    }
    CHECK_FALSE(same);
}

TEST_CASE("Tetromino: 逆时针旋转") {
    Tetromino piece(TetrominoType::T);
    const auto& initial = piece.getBlockOffsets();

    piece.rotateCCW();
    piece.rotateCW();
    const auto& back = piece.getBlockOffsets();

    for (size_t j = 0; j < 4; ++j) {
        CHECK(back[j] == initial[j]);
    }
}

TEST_CASE("Tetromino: getSpawnCol 返回有效列") {
    for (int t = 0; t < static_cast<int>(TetrominoType::COUNT); ++t) {
        Tetromino piece(static_cast<TetrominoType>(t));
        int col = piece.getSpawnCol();
        CHECK(col >= 0);
        CHECK(col < BOARD_COLS);
    }
}

TEST_CASE("Tetromino: move 正确移动") {
    Tetromino piece(TetrominoType::T);
    int startRow = piece.getRow();
    int startCol = piece.getCol();

    piece.move(3, 2);
    CHECK(piece.getRow() == startRow + 3);
    CHECK(piece.getCol() == startCol + 2);

    piece.move(-1, -3);
    CHECK(piece.getRow() == startRow + 2);
    CHECK(piece.getCol() == startCol - 1);
}

TEST_CASE("Tetromino: 每种方块颜色都不为黑") {
    for (int t = 0; t < static_cast<int>(TetrominoType::COUNT); ++t) {
        Tetromino piece(static_cast<TetrominoType>(t));
        Color color = piece.getColor();
        bool isBlack = (color.r == 0 && color.g == 0 && color.b == 0);
        CHECK_FALSE(isBlack);
    }
}

TEST_CASE("Tetromino: 默认构造函数") {
    Tetromino piece;
    CHECK(piece.getType() == TetrominoType::I);
    CHECK(piece.getRotation() == 0);
    CHECK(piece.getRow() == 0);
    CHECK(piece.getCol() == 0);
}

TEST_CASE("Tetromino: I 方块有 4 种旋转状态") {
    Tetromino piece(TetrominoType::I);
    std::set<std::array<std::pair<int, int>, 4>> states;
    for (int i = 0; i < 4; ++i) {
        states.insert(piece.getBlockOffsets());
        piece.rotateCW();
    }
    CHECK(states.size() == 4);
}

TEST_CASE("Tetromino: S/Z 方块各有 4 种旋转状态") {
    Tetromino pieceS(TetrominoType::S);
    std::set<std::array<std::pair<int, int>, 4>> statesS;
    for (int i = 0; i < 4; ++i) {
        statesS.insert(pieceS.getBlockOffsets());
        pieceS.rotateCW();
    }
    CHECK(statesS.size() == 4);

    Tetromino pieceZ(TetrominoType::Z);
    std::set<std::array<std::pair<int, int>, 4>> statesZ;
    for (int i = 0; i < 4; ++i) {
        statesZ.insert(pieceZ.getBlockOffsets());
        pieceZ.rotateCW();
    }
    CHECK(statesZ.size() == 4);
}

TEST_CASE("Tetromino: J/L/T 方块有 4 种不同旋转状态") {
    Tetromino pieceT(TetrominoType::T);
    std::set<std::array<std::pair<int, int>, 4>> statesT;
    for (int i = 0; i < 4; ++i) {
        statesT.insert(pieceT.getBlockOffsets());
        pieceT.rotateCW();
    }
    CHECK(statesT.size() == 4);

    Tetromino pieceJ(TetrominoType::J);
    std::set<std::array<std::pair<int, int>, 4>> statesJ;
    for (int i = 0; i < 4; ++i) {
        statesJ.insert(pieceJ.getBlockOffsets());
        pieceJ.rotateCW();
    }
    CHECK(statesJ.size() == 4);

    Tetromino pieceL(TetrominoType::L);
    std::set<std::array<std::pair<int, int>, 4>> statesL;
    for (int i = 0; i < 4; ++i) {
        statesL.insert(pieceL.getBlockOffsets());
        pieceL.rotateCW();
    }
    CHECK(statesL.size() == 4);
}
