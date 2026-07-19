#include "ScoreManager.h"
#include "Constants.h"
#include <fstream>
#include <iostream>

// ============================================================
// 计分规则（NES 风格）
// 单消: 40 × (等级 + 1)
// 双消: 100 × (等级 + 1)
// 三消: 300 × (等级 + 1)
// 四消: 1200 × (等级 + 1)
// 软降: 每格 1 分
// 硬降: 每格 2 分
// ============================================================
static const int SCORE_TABLE[4] = { 40, 100, 300, 1200 };

// ============================================================
// 构造函数：初始化并读取最高分
// ============================================================
ScoreManager::ScoreManager()
    : m_score(0)
    , m_highScore(0)
    , m_level(1)
    , m_linesCleared(0)
{
    loadHighScore();
}

// ============================================================
// 重置当前游戏分数，保留最高分
// ============================================================
void ScoreManager::reset() {
    m_score = 0;
    m_level = 1;
    m_linesCleared = 0;
}

// ============================================================
// 根据消除行数更新分数和等级
// ============================================================
void ScoreManager::addLinesCleared(int lines) {
    if (lines <= 0 || lines > 4) return;

    // 增加分数
    m_score += SCORE_TABLE[lines - 1] * m_level;

    // 更新消除行数
    m_linesCleared += lines;

    // 更新等级（每消除 LINES_PER_LEVEL 行升一级）
    m_level = 1 + m_linesCleared / LINES_PER_LEVEL;

    // 更新最高分
    if (m_score > m_highScore) {
        m_highScore = m_score;
    }
}

// ============================================================
// 软降落加分（每格1分）
// ============================================================
void ScoreManager::addSoftDrop(int cells) {
    m_score += cells;
    if (m_score > m_highScore) {
        m_highScore = m_score;
    }
}

// ============================================================
// 硬降落加分（每格2分）
// ============================================================
void ScoreManager::addHardDrop(int cells) {
    m_score += cells * 2;
    if (m_score > m_highScore) {
        m_highScore = m_score;
    }
}

// ============================================================
// 保存最高分到文件
// ============================================================
void ScoreManager::saveHighScore() const {
    std::ofstream file(HIGHSCORE_FILE);
    if (file.is_open()) {
        file << m_highScore << std::endl;
        file.close();
    }
}

// ============================================================
// 从文件读取最高分
// ============================================================
void ScoreManager::loadHighScore() {
    std::ifstream file(HIGHSCORE_FILE);
    if (file.is_open()) {
        file >> m_highScore;
        file.close();
    }
}

// ============================================================
// 获取当前等级的下落间隔
// 等级越高，下落越快
// ============================================================
int ScoreManager::getDropInterval() const {
    int interval = BASE_DROP_INTERVAL - (m_level - 1) * 50;
    return std::max(interval, MIN_DROP_INTERVAL);
}
