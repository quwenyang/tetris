#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

#include <string>

// 计分管理器：管理当前分数、等级、消除行数和最高分
class ScoreManager {
public:
    ScoreManager();

    // 重置游戏分数（保留最高分）
    void reset();

    // 根据消除行数更新分数
    // 计分规则：根据同时消除的行数和当前等级计算
    void addLinesCleared(int lines);

    // 软降落加分
    void addSoftDrop(int cells);

    // 硬降落加分
    void addHardDrop(int cells);

    // 保存最高分到文件
    void saveHighScore() const;

    // 从文件读取最高分
    void loadHighScore();

    // Getter 方法
    int getScore() const { return m_score; }
    int getHighScore() const { return m_highScore; }
    int getLevel() const { return m_level; }
    int getLinesCleared() const { return m_linesCleared; }
    int getDropInterval() const;

private:
    int m_score;          // 当前分数
    int m_highScore;      // 历史最高分
    int m_level;          // 当前等级
    int m_linesCleared;   // 已消除总行数
};

#endif // SCOREMANAGER_H
