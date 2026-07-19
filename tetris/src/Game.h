#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <vector>
#include "Board.h"
#include "Tetromino.h"
#include "ScoreManager.h"
#include "Renderer.h"
#include "InputHandler.h"

// 游戏状态
enum class GameState {
    PLAYING,
    PAUSED,
    GAME_OVER
};

// 游戏主控制器
class Game {
public:
    Game(HINSTANCE hInstance);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // 运行游戏
    void run();

    // 窗口过程（静态）
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void init();
    void handleInput(float dt);
    void update(float dt);
    void render();
    void spawnPiece();
    TetrominoType getNextPieceType();
    int calculateGhostRow() const;
    bool tryRotateCW();
    bool tryRotateCCW();
    void hardDrop();
    void lockAndProcess();
    void restart();
    void beginWindowDrag();  // 自定义窗口拖拽（避免模态循环阻塞游戏）

    // 窗口句柄
    HWND m_hwnd;
    HINSTANCE m_hInstance;

    // 核心组件
    Board m_board;
    ScoreManager m_score;
    Renderer m_renderer;
    InputHandler m_input;

    // 游戏状态
    GameState m_gameState;

    // 方块
    Tetromino m_currentPiece;
    Tetromino* m_nextPiece;
    Tetromino m_nextPieceStorage;

    // 下落计时
    float m_dropAccumulator;

    // 7-bag
    std::vector<TetrominoType> m_bag;
    int m_bagIndex;

    // 锁定延迟
    float m_lockDelayTimer;
    static constexpr float LOCK_DELAY_MAX = 0.5f;
    bool m_isLocking;

    // DAS 常量（public 供 handleDasDirection 使用）
public:
    static constexpr float DAS_DELAY = 0.17f;
    static constexpr float ARR_INTERVAL = 0.05f;
private:

    // DAS
    bool m_leftHeld, m_rightHeld;
    float m_leftDasTimer, m_rightDasTimer;
    float m_leftArrTimer, m_rightArrTimer;
    bool m_leftDasReady, m_rightDasReady;

    // 高性能计时
    LARGE_INTEGER m_freq;
};

#endif // GAME_H
