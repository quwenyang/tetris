#include "doctest.h"
#include "ScoreManager.h"
#include "Constants.h"
#include <fstream>

// ============================================================
// ScoreManager 单元测试
// ============================================================

TEST_CASE("ScoreManager: 初始分数为 0") {
    ScoreManager sm;
    CHECK(sm.getScore() == 0);
}

TEST_CASE("ScoreManager: 初始等级为 1") {
    ScoreManager sm;
    CHECK(sm.getLevel() == 1);
}

TEST_CASE("ScoreManager: 初始消除行数为 0") {
    ScoreManager sm;
    CHECK(sm.getLinesCleared() == 0);
}

TEST_CASE("ScoreManager: addLinesCleared(0) 无变化") {
    ScoreManager sm;
    sm.addLinesCleared(0);
    CHECK(sm.getScore() == 0);
    CHECK(sm.getLinesCleared() == 0);
}

TEST_CASE("ScoreManager: addLinesCleared(5) 无变化（超出范围）") {
    ScoreManager sm;
    sm.addLinesCleared(5);
    CHECK(sm.getScore() == 0);
    CHECK(sm.getLinesCleared() == 0);
}

TEST_CASE("ScoreManager: 消除 1 行得分 40 * level") {
    ScoreManager sm;
    sm.addLinesCleared(1);
    // 等级 1 时：40 * 1 = 40
    CHECK(sm.getScore() == 40);
    CHECK(sm.getLinesCleared() == 1);
}

TEST_CASE("ScoreManager: 消除 2 行得分 100 * level") {
    ScoreManager sm;
    sm.addLinesCleared(2);
    CHECK(sm.getScore() == 100);
    CHECK(sm.getLinesCleared() == 2);
}

TEST_CASE("ScoreManager: 消除 3 行得分 300 * level") {
    ScoreManager sm;
    sm.addLinesCleared(3);
    CHECK(sm.getScore() == 300);
    CHECK(sm.getLinesCleared() == 3);
}

TEST_CASE("ScoreManager: 消除 4 行得分 1200 * level") {
    ScoreManager sm;
    sm.addLinesCleared(4);
    CHECK(sm.getScore() == 1200);
    CHECK(sm.getLinesCleared() == 4);
}

TEST_CASE("ScoreManager: 等级提升（每 10 行升一级）") {
    ScoreManager sm;
    sm.addLinesCleared(4); // 4 行，等级 1
    CHECK(sm.getLevel() == 1);
    sm.addLinesCleared(4); // 8 行，等级 1
    CHECK(sm.getLevel() == 1);
    sm.addLinesCleared(2); // 10 行，等级 2
    CHECK(sm.getLevel() == 2);
    CHECK(sm.getLinesCleared() == 10);
}

TEST_CASE("ScoreManager: 高等级下得分更多") {
    ScoreManager sm;
    // 升到等级 2：addLinesCleared 只接受 1-4 行，多次累加
    sm.addLinesCleared(4);
    sm.addLinesCleared(4);
    sm.addLinesCleared(2); // 总共 10 行
    CHECK(sm.getLevel() == 2);

    // 在等级 2 消除 1 行：40 * 2 = 80
    int scoreBefore = sm.getScore();
    sm.addLinesCleared(1);
    CHECK(sm.getScore() == scoreBefore + 80);
}

TEST_CASE("ScoreManager: addSoftDrop 每格加 1 分") {
    ScoreManager sm;
    sm.addSoftDrop(5);
    CHECK(sm.getScore() == 5);
    sm.addSoftDrop(3);
    CHECK(sm.getScore() == 8);
}

TEST_CASE("ScoreManager: addHardDrop 每格加 2 分") {
    ScoreManager sm;
    sm.addHardDrop(5);
    CHECK(sm.getScore() == 10);
    sm.addHardDrop(3);
    CHECK(sm.getScore() == 16);
}

TEST_CASE("ScoreManager: reset 保留最高分") {
    std::remove(HIGHSCORE_FILE);
    ScoreManager sm;
    sm.addLinesCleared(4); // 1200 分
    CHECK(sm.getHighScore() == 1200);

    sm.reset();
    CHECK(sm.getScore() == 0);
    CHECK(sm.getLevel() == 1);
    CHECK(sm.getLinesCleared() == 0);
    CHECK(sm.getHighScore() == 1200); // 保留
}

TEST_CASE("ScoreManager: 最高分自动更新") {
    std::remove(HIGHSCORE_FILE);
    ScoreManager sm;
    CHECK(sm.getHighScore() == 0);

    sm.addLinesCleared(1); // 40 分
    CHECK(sm.getHighScore() == 40);

    sm.reset();
    sm.addLinesCleared(1); // 40 分，小于最高分
    CHECK(sm.getHighScore() == 40);

    sm.addLinesCleared(4); // 40 + 1200 = 1240 分
    CHECK(sm.getHighScore() == 1240);
}

TEST_CASE("ScoreManager: 下落间隔随等级递减") {
    std::remove(HIGHSCORE_FILE);
    ScoreManager sm;
    // 等级 1: 800ms
    CHECK(sm.getDropInterval() == 800);

    // 升到 11 级：消除 100 行（每个 4 行，25 轮）
    for (int i = 0; i < 25; ++i) {
        sm.addLinesCleared(4);
    }
    CHECK(sm.getLevel() == 11);
    int interval = sm.getDropInterval();
    CHECK(interval == 300); // 800 - 10*50 = 300
}

TEST_CASE("ScoreManager: 下落间隔不低于最小值 50ms") {
    std::remove(HIGHSCORE_FILE);
    ScoreManager sm;
    for (int i = 0; i < 100; ++i) {
        sm.addLinesCleared(4);
    }
    CHECK(sm.getDropInterval() == 50);
}

TEST_CASE("ScoreManager: 保存和读取最高分") {
    // 清理测试文件
    std::remove(HIGHSCORE_FILE);

    // 创建带有记录的 ScoreManager
    {
        ScoreManager sm;
        sm.addLinesCleared(4); // 1200 分
        CHECK(sm.getHighScore() == 1200);
        sm.saveHighScore();
    }

    // 重新读取
    {
        ScoreManager sm2;
        CHECK(sm2.getHighScore() == 1200);
    }

    // 清理
    std::remove(HIGHSCORE_FILE);
}

TEST_CASE("ScoreManager: 最高分文件不存在时初始为 0") {
    std::remove(HIGHSCORE_FILE);
    ScoreManager sm;
    CHECK(sm.getHighScore() == 0);
}

TEST_CASE("ScoreManager: reset 不影响最大消除行数统计") {
    ScoreManager sm;
    sm.addLinesCleared(4);
    sm.addLinesCleared(4);
    sm.addLinesCleared(2); // 10 行, 等级 2

    sm.reset();
    CHECK(sm.getScore() == 0);
    CHECK(sm.getLevel() == 1);
    CHECK(sm.getLinesCleared() == 0);
}
