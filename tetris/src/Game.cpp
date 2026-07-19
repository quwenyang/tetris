#include "Game.h"
#include "Constants.h"
#include <algorithm>
#include <random>
LRESULT CALLBACK Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 获取 Game 实例指针
    Game* game = reinterpret_cast<Game*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_ERASEBKGND:
            // 阻止 Windows 擦除背景（消除闪烁的核心）
            return TRUE;
        case WM_PAINT: {
            // 从持久化双缓冲输出到屏幕，不做额外绘制
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if (game) {
                game->m_renderer.blitToScreen(hwnd);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_MOVE || (wParam & 0xFFF0) == SC_SIZE) {
                // 禁止进入模态循环，改为自定义拖拽
                if (game && (wParam & 0xFFF0) == SC_MOVE) {
                    game->beginWindowDrag();
                }
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
                return 0;
            }
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
// 构造函数
// ============================================================
Game::Game(HINSTANCE hInstance)
    : m_hwnd(nullptr)
    , m_hInstance(hInstance)
    , m_gameState(GameState::PLAYING)
    , m_nextPiece(nullptr)
    , m_dropAccumulator(0.0f)
    , m_bagIndex(0)
    , m_lockDelayTimer(0.0f)
    , m_isLocking(false)
    , m_leftHeld(false)
    , m_rightHeld(false)
    , m_leftDasTimer(0.0f)
    , m_rightDasTimer(0.0f)
    , m_leftArrTimer(0.0f)
    , m_rightArrTimer(0.0f)
    , m_leftDasReady(false)
    , m_rightDasReady(false)
{
    QueryPerformanceFrequency(&m_freq);

    // 注册窗口类
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"TetrisWindow";
    RegisterClassW(&wc);

    // 创建窗口
    RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX, FALSE);

    m_hwnd = CreateWindowExW(0, L"TetrisWindow", L"Tetris",
        WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (m_hwnd) {
        // 保存 this 指针，供 WndProc 使用
        SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        // 初始化双缓冲
        m_renderer.initBuffer(m_hwnd);
    }

    // 初始化游戏状态（必须先于 ShowWindow，这样第一次 WM_PAINT 看到的是游戏画面）
    init();

    if (m_hwnd) {
        // 渲染第一帧到缓冲（确保 WM_PAINT 不显示垃圾数据）
        render();
        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hwnd);
    }
}

// ============================================================
// 析构函数：保存最高分
// ============================================================
Game::~Game() {
    m_score.saveHighScore();
}

// ============================================================
// 初始化游戏
// ============================================================
void Game::init() {
    m_board.reset();
    m_score.reset();
    m_bag.clear();
    m_bagIndex = 0;
    m_dropAccumulator = 0.0f;
    m_lockDelayTimer = 0.0f;
    m_isLocking = false;
    m_leftHeld = m_rightHeld = false;
    m_leftDasTimer = m_rightDasTimer = 0.0f;
    m_leftArrTimer = m_rightArrTimer = 0.0f;
    m_leftDasReady = m_rightDasReady = false;
    m_gameState = GameState::PLAYING;

    // 7-bag
    m_bag = {
        TetrominoType::I, TetrominoType::O, TetrominoType::T,
        TetrominoType::S, TetrominoType::Z, TetrominoType::J,
        TetrominoType::L
    };
    static std::mt19937 rng(std::random_device{}());
    std::shuffle(m_bag.begin(), m_bag.end(), rng);

    m_nextPieceStorage = Tetromino(getNextPieceType());
    m_nextPiece = &m_nextPieceStorage;
    spawnPiece();
}

// ============================================================
// 主循环
// ============================================================
void Game::run() {
    // 固定 60 FPS 渲染
    const double TARGET_FRAME_TIME = 1.0 / 60.0;

    LARGE_INTEGER lastFrameTime;
    QueryPerformanceCounter(&lastFrameTime);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        // 每轮循环都处理消息（不能跳过，否则窗口会卡死）
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 帧率控制：未到渲染时机则短暂休眠降低 CPU 占用
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsed = static_cast<double>(now.QuadPart - lastFrameTime.QuadPart)
                        / static_cast<double>(m_freq.QuadPart);

        if (elapsed < TARGET_FRAME_TIME) {
            DWORD sleepMs = static_cast<DWORD>((TARGET_FRAME_TIME - elapsed) * 1000);
            if (sleepMs > 1) Sleep(1);
            continue;
        }

        lastFrameTime = now;
        float dt = static_cast<float>(std::min(elapsed, 0.05));

        // 更新游戏
        m_input.update();
        handleInput(dt);
        update(dt);

        // 渲染
        render();
    }
}

// ============================================================
// DAS 辅助函数
// ============================================================
static void handleDasDirection(float dt, bool keyDown,
                               bool& held, float& dasTimer, float& arrTimer, bool& dasReady,
                               int dRow, int dCol,
                               Tetromino& piece, const Board& board,
                               bool& isLocking, float& lockTimer)
{
    if (!keyDown) {
        held = false;
        dasTimer = arrTimer = 0.0f;
        dasReady = false;
        return;
    }

    if (!held) {
        held = true;
        dasTimer = arrTimer = 0.0f;
        dasReady = false;
        Tetromino test = piece;
        test.move(dRow, dCol);
        if (board.isValidPosition(test)) {
            piece.move(dRow, dCol);
            if (isLocking) lockTimer = 0.0f;
        }
        return;
    }

    dasTimer += dt;
    if (!dasReady) {
        if (dasTimer >= Game::DAS_DELAY) {
            dasReady = true;
            arrTimer = 0.0f;
            Tetromino test = piece;
            test.move(dRow, dCol);
            if (board.isValidPosition(test)) {
                piece.move(dRow, dCol);
                if (isLocking) lockTimer = 0.0f;
            }
        }
    } else {
        arrTimer += dt;
        while (arrTimer >= Game::ARR_INTERVAL) {
            arrTimer -= Game::ARR_INTERVAL;
            Tetromino test = piece;
            test.move(dRow, dCol);
            if (board.isValidPosition(test)) {
                piece.move(dRow, dCol);
                if (isLocking) lockTimer = 0.0f;
            }
        }
    }
}

// ============================================================
// 处理输入
// ============================================================
void Game::handleInput(float dt) {
    if (m_gameState == GameState::GAME_OVER) {
        if (m_input.isKeyPressed(VK_RETURN)) {
            restart();
        }
        return;
    }

    if (m_gameState == GameState::PAUSED) {
        if (m_input.isKeyPressed('P')) {
            m_gameState = GameState::PLAYING;
        }
        return;
    }

    // 暂停
    if (m_input.isKeyPressed('P')) {
        m_gameState = GameState::PAUSED;
        return;
    }

    // 左右移动
    handleDasDirection(dt, m_input.isKeyHeld(VK_LEFT),
                       m_leftHeld, m_leftDasTimer, m_leftArrTimer, m_leftDasReady,
                       0, -1, m_currentPiece, m_board, m_isLocking, m_lockDelayTimer);

    handleDasDirection(dt, m_input.isKeyHeld(VK_RIGHT),
                       m_rightHeld, m_rightDasTimer, m_rightArrTimer, m_rightDasReady,
                       0, 1, m_currentPiece, m_board, m_isLocking, m_lockDelayTimer);

    // 顺时针旋转
    if (m_input.isKeyPressed(VK_UP)) {
        tryRotateCW();
    }

    // 逆时针旋转
    if (m_input.isKeyPressed('Z')) {
        tryRotateCCW();
    }

    // 硬降
    if (m_input.isKeyPressed(VK_SPACE)) {
        hardDrop();
    }
}

// ============================================================
// 更新游戏逻辑
// ============================================================
void Game::update(float dt) {
    if (m_gameState != GameState::PLAYING) return;

    bool softDropping = m_input.isKeyHeld(VK_DOWN);
    float dropInterval = m_score.getDropInterval() / 1000.0f;

    // 软降加速
    if (softDropping) {
        m_dropAccumulator += dt * (static_cast<float>(BASE_DROP_INTERVAL) / SOFT_DROP_INTERVAL);
    } else {
        m_dropAccumulator += dt;
    }

    if (m_dropAccumulator >= dropInterval) {
        m_dropAccumulator -= dropInterval;

        Tetromino test = m_currentPiece;
        test.move(1, 0);
        if (m_board.isValidPosition(test)) {
            m_currentPiece.move(1, 0);
            m_isLocking = false;
            m_lockDelayTimer = 0.0f;
            if (softDropping) {
                m_score.addSoftDrop(1);
            }
        } else {
            if (!m_isLocking) {
                m_isLocking = true;
                m_lockDelayTimer = 0.0f;
            }
            m_lockDelayTimer += dropInterval;
            if (m_lockDelayTimer >= LOCK_DELAY_MAX) {
                lockAndProcess();
            }
        }
    }
}

// ============================================================
// 渲染
// ============================================================
void Game::render() {
    if (!m_hwnd) return;

    int ghostRow = calculateGhostRow();
    bool isOver = (m_gameState == GameState::GAME_OVER);
    bool isPaused = (m_gameState == GameState::PAUSED);

    m_renderer.draw(m_hwnd, m_board, m_currentPiece, m_nextPiece,
                    m_score, ghostRow, isOver, isPaused);
}

// ============================================================
// 生成方块
// ============================================================
void Game::spawnPiece() {
    m_currentPiece = *m_nextPiece;
    m_currentPiece.setRow(BOARD_HIDDEN_ROWS - 1);
    m_currentPiece.setCol(m_currentPiece.getSpawnCol());

    if (!m_board.isValidPosition(m_currentPiece)) {
        m_gameState = GameState::GAME_OVER;
        m_score.saveHighScore();
        return;
    }

    m_nextPieceStorage = Tetromino(getNextPieceType());
    m_nextPiece = &m_nextPieceStorage;

    m_isLocking = false;
    m_lockDelayTimer = 0.0f;
    m_dropAccumulator = 0.0f;
}

// ============================================================
// 7-bag
// ============================================================
TetrominoType Game::getNextPieceType() {
    if (m_bagIndex >= static_cast<int>(m_bag.size())) {
        static std::mt19937 rng(std::random_device{}());
        std::shuffle(m_bag.begin(), m_bag.end(), rng);
        m_bagIndex = 0;
    }
    return m_bag[m_bagIndex++];
}

// ============================================================
// 幽灵方块
// ============================================================
int Game::calculateGhostRow() const {
    int row = m_currentPiece.getRow();
    while (true) {
        Tetromino test = m_currentPiece;
        test.setRow(row + 1);
        if (!m_board.isValidPosition(test)) break;
        row++;
    }
    return row;
}

// ============================================================
// 顺时针旋转（含墙踢）
// ============================================================
bool Game::tryRotateCW() {
    Tetromino test = m_currentPiece;
    test.rotateCW();

    if (m_board.isValidPosition(test)) {
        m_currentPiece.rotateCW();
        if (m_isLocking) m_lockDelayTimer = 0.0f;
        return true;
    }

    static const int kicks[5][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}, {-2,0}};
    int maxKicks = (m_currentPiece.getType() == TetrominoType::I) ? 5 : 4;

    for (int i = 0; i < maxKicks; ++i) {
        Tetromino kicked = test;
        kicked.move(kicks[i][0], kicks[i][1]);
        if (m_board.isValidPosition(kicked)) {
            m_currentPiece.rotateCW();
            m_currentPiece.move(kicks[i][0], kicks[i][1]);
            if (m_isLocking) m_lockDelayTimer = 0.0f;
            return true;
        }
    }
    return false;
}

// ============================================================
// 逆时针旋转
// ============================================================
bool Game::tryRotateCCW() {
    Tetromino test = m_currentPiece;
    test.rotateCCW();

    if (m_board.isValidPosition(test)) {
        m_currentPiece.rotateCCW();
        if (m_isLocking) m_lockDelayTimer = 0.0f;
        return true;
    }

    static const int kicks[5][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}, {-2,0}};
    int maxKicks = (m_currentPiece.getType() == TetrominoType::I) ? 5 : 4;

    for (int i = 0; i < maxKicks; ++i) {
        Tetromino kicked = test;
        kicked.move(kicks[i][0], kicks[i][1]);
        if (m_board.isValidPosition(kicked)) {
            m_currentPiece.rotateCCW();
            m_currentPiece.move(kicks[i][0], kicks[i][1]);
            if (m_isLocking) m_lockDelayTimer = 0.0f;
            return true;
        }
    }
    return false;
}

// ============================================================
// 硬降落
// ============================================================
void Game::hardDrop() {
    int dist = 0;
    while (true) {
        Tetromino test = m_currentPiece;
        test.move(1, 0);
        if (!m_board.isValidPosition(test)) break;
        m_currentPiece.move(1, 0);
        dist++;
    }
    if (dist > 0) {
        m_score.addHardDrop(dist);
    }
    lockAndProcess();
}

// ============================================================
// 锁定并消行
// ============================================================
void Game::lockAndProcess() {
    m_board.lockPiece(m_currentPiece);

    if (m_board.isGameOver()) {
        m_gameState = GameState::GAME_OVER;
        m_score.saveHighScore();
        return;
    }

    std::vector<int> clearedRows;
    int lines = m_board.clearLines(clearedRows);
    if (lines > 0) {
        m_score.addLinesCleared(lines);
    }
    spawnPiece();
}

// ============================================================
// 自定义窗口拖拽（替代 DefWindowProc 的模态循环）
// 拖拽期间继续运行游戏，避免界面卡死
// ============================================================
void Game::beginWindowDrag() {
    SetCapture(m_hwnd);

    POINT cursorStart;
    GetCursorPos(&cursorStart);
    RECT winRect;
    GetWindowRect(m_hwnd, &winRect);

    const double TARGET_FRAME_TIME = 1.0 / 60.0;
    LARGE_INTEGER lastFrameTime;
    QueryPerformanceCounter(&lastFrameTime);

    MSG msg = {};
    while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        // 处理窗口消息
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 移动窗口
        POINT curPos;
        GetCursorPos(&curPos);
        int dx = curPos.x - cursorStart.x;
        int dy = curPos.y - cursorStart.y;
        SetWindowPos(m_hwnd, nullptr,
            winRect.left + dx, winRect.top + dy,
            0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // 帧率控制：拖拽期间以 60 FPS 更新游戏和渲染
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsed = static_cast<double>(now.QuadPart - lastFrameTime.QuadPart)
                        / static_cast<double>(m_freq.QuadPart);

        if (elapsed < TARGET_FRAME_TIME) {
            DWORD sleepMs = static_cast<DWORD>((TARGET_FRAME_TIME - elapsed) * 1000);
            if (sleepMs > 1) Sleep(1);
            continue;
        }

        lastFrameTime = now;
        float dt = static_cast<float>(std::min(elapsed, 0.05));

        m_input.update();
        handleInput(dt);
        update(dt);
        render();
    }

    ReleaseCapture();
}

// ============================================================
// 重新开始
// ============================================================
void Game::restart() {
    init();
}
