#include "Renderer.h"
#include "Constants.h"
#include <string>

// ============================================================
// 构造函数：创建字体，初始化缓冲状态
// ============================================================
Renderer::Renderer()
    : m_bufferReady(false)
    , m_memDC(nullptr)
    , m_memBitmap(nullptr)
    , m_oldBitmap(nullptr)
    , m_width(WINDOW_WIDTH)
    , m_height(WINDOW_HEIGHT)
{
    m_font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    m_fontSmall = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    m_fontLarge = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    m_fontTitle = CreateFontW(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
}

// ============================================================
// 析构函数：释放双缓冲和字体资源
// ============================================================
Renderer::~Renderer() {
    if (m_bufferReady) {
        SelectObject(m_memDC, m_oldBitmap);
        DeleteObject(m_memBitmap);
        DeleteDC(m_memDC);
    }
    DeleteObject(m_fontTitle);
    DeleteObject(m_fontLarge);
    DeleteObject(m_fontSmall);
    DeleteObject(m_font);
}

// ============================================================
// 初始化持久化双缓冲（窗口创建后调用一次）
// ============================================================
bool Renderer::initBuffer(HWND hwnd) {
    HDC screenDC = GetDC(hwnd);
    m_memDC = CreateCompatibleDC(screenDC);
    m_memBitmap = CreateCompatibleBitmap(screenDC, m_width, m_height);
    if (!m_memDC || !m_memBitmap) {
        ReleaseDC(hwnd, screenDC);
        return false;
    }
    m_oldBitmap = static_cast<HBITMAP>(SelectObject(m_memDC, m_memBitmap));
    m_bufferReady = true;
    ReleaseDC(hwnd, screenDC);
    return true;
}

// ============================================================
// 从双缓冲 BitBlt 到屏幕（供 WM_PAINT 使用）
// ============================================================
void Renderer::blitToScreen(HWND hwnd) {
    if (!m_bufferReady) return;
    HDC screenDC = GetDC(hwnd);
    BitBlt(screenDC, 0, 0, m_width, m_height, m_memDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, screenDC);
}

// ============================================================
// 主绘制：所有内容绘制到持久化双缓冲，然后一次性 BitBlt 到屏幕
// ============================================================
void Renderer::draw(HWND hwnd, const Board& board, const Tetromino& currentPiece,
                    const Tetromino* nextPiece, const ScoreManager& score,
                    int ghostRow, bool isGameOver, bool isPaused)
{
    if (!m_bufferReady) return;

    // 清空背景
    drawFilledRect(m_memDC, 0, 0, m_width, m_height, COLOR_DARK);

    drawBoard(m_memDC);
    drawCells(m_memDC, board);
    drawGhostPiece(m_memDC, currentPiece, ghostRow);
    drawPiece(m_memDC, currentPiece, currentPiece.getRow(), currentPiece.getCol(),
              currentPiece.getColor());
    drawSidePanel(m_memDC, score, nextPiece);

    if (isPaused) {
        // 暂停遮罩叠加在游戏画面上（统一的一次 BitBlt）
        drawFilledRect(m_memDC, 0, 0, m_width, m_height,
                       Color(0, 0, 0, 150));
        drawText(m_memDC, L"PAUSED", m_width / 2 - 80,
                 m_height / 2 - 20, 160, 40,
                 COLOR_TEXT, 48);
    }

    if (isGameOver) {
        drawFilledRect(m_memDC, 0, 0, m_width, m_height,
                       Color(0, 0, 0, 180));

        int cx = m_width / 2;
        int cy = m_height / 2;

        drawText(m_memDC, L"Game Over", cx - 120, cy - 60, 240, 50,
                 COLOR_RED, 48);

        std::wstring scoreText = L"Score: " + std::to_wstring(score.getScore());
        drawText(m_memDC, scoreText.c_str(), cx - 100, cy + 10, 200, 30,
                 COLOR_TEXT, 22);

        drawText(m_memDC, L"Press ENTER to restart", cx - 120, cy + 60, 240, 20,
                 COLOR_TEXT_DARK, 16);
    }

    // 一次性输出到屏幕
    blitToScreen(hwnd);
}

// ============================================================
// 将 Color 转换为 GDI COLORREF
// ============================================================
COLORREF Renderer::toColorRef(Color color, int alpha) const {
    if (alpha < 255) {
        int r = color.r * alpha / 255;
        int g = color.g * alpha / 255;
        int b = color.b * alpha / 255;
        return RGB(r, g, b);
    }
    return RGB(color.r, color.g, color.b);
}

// ============================================================
// 绘制填充矩形
// ============================================================
void Renderer::drawFilledRect(HDC hdc, int x, int y, int w, int h, Color color) {
    HBRUSH brush = CreateSolidBrush(toColorRef(color));
    RECT rect = { x, y, x + w, y + h };
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

// ============================================================
// 绘制矩形边框
// ============================================================
void Renderer::drawRect(HDC hdc, int x, int y, int w, int h, Color color) {
    HPEN pen = CreatePen(PS_SOLID, 1, toColorRef(color));
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

// ============================================================
// 绘制文本
// ============================================================
void Renderer::drawText(HDC hdc, const wchar_t* text, int x, int y,
                        int w, int h, Color color, int fontSize) {
    HFONT font = nullptr;
    switch (fontSize) {
        case 12:  font = m_fontSmall; break;
        case 16:  font = m_font; break;
        case 22:  font = m_fontLarge; break;
        case 48:  font = m_fontTitle; break;
        default:  font = m_font; break;
    }

    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, toColorRef(color));
    SetBkMode(hdc, TRANSPARENT);

    RECT rect = { x, y, x + w, y + h };
    DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
}

// ============================================================
// 绘制面板背景和网格
// ============================================================
void Renderer::drawBoard(HDC hdc) {
    drawFilledRect(hdc, 0, 0, BOARD_PIXEL_WIDTH, BOARD_PIXEL_HEIGHT, COLOR_BG);
    drawRect(hdc, 0, 0, BOARD_PIXEL_WIDTH, BOARD_PIXEL_HEIGHT, COLOR_BORDER);

    for (int r = 0; r <= BOARD_ROWS; ++r) {
        drawFilledRect(hdc, 0, r * CELL_SIZE, BOARD_PIXEL_WIDTH, 1, COLOR_GRID);
    }
    for (int c = 0; c <= BOARD_COLS; ++c) {
        drawFilledRect(hdc, c * CELL_SIZE, 0, 1, BOARD_PIXEL_HEIGHT, COLOR_GRID);
    }
}

// ============================================================
// 绘制面板中的方块
// ============================================================
void Renderer::drawCells(HDC hdc, const Board& board) {
    for (int r = BOARD_HIDDEN_ROWS; r < BOARD_TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            int cellValue = board.getCell(r, c);
            if (cellValue > 0) {
                Color color = Tetromino::COLORS[cellValue - 1];
                int py = (r - BOARD_HIDDEN_ROWS) * CELL_SIZE;
                int px = c * CELL_SIZE;
                drawFilledRect(hdc, px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2, color);
            }
        }
    }
}

// ============================================================
// 绘制单个方块
// ============================================================
void Renderer::drawPiece(HDC hdc, const Tetromino& piece, int row, int col,
                         Color color, int alpha) {
    const auto& offsets = piece.getBlockOffsets();
    for (const auto& block : offsets) {
        int r = row + block.first - BOARD_HIDDEN_ROWS;
        int c = col + block.second;
        if (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS) {
            int py = r * CELL_SIZE + 1;
            int px = c * CELL_SIZE + 1;
            drawFilledRect(hdc, px, py, CELL_SIZE - 2, CELL_SIZE - 2, color);
        }
    }
}

// ============================================================
// 绘制幽灵方块
// ============================================================
void Renderer::drawGhostPiece(HDC hdc, const Tetromino& piece, int ghostRow) {
    Color ghostColor(255, 255, 255, 80);
    drawPiece(hdc, piece, ghostRow, piece.getCol(), ghostColor, 80);
}

// ============================================================
// 绘制右侧信息面板
// ============================================================
void Renderer::drawSidePanel(HDC hdc, const ScoreManager& score,
                             const Tetromino* nextPiece) {
    int px = BOARD_PIXEL_WIDTH + 15;
    int textY = 20;

    // 分数
    drawText(hdc, L"SCORE", px, textY, 120, 20, COLOR_TEXT_DIM, 12);
    textY += 22;
    std::wstring scoreStr = std::to_wstring(score.getScore());
    drawText(hdc, scoreStr.c_str(), px, textY, 120, 30, COLOR_TEXT, 22);
    textY += 40;

    // 最高分
    drawText(hdc, L"HIGH SCORE", px, textY, 120, 20, COLOR_TEXT_DIM, 12);
    textY += 22;
    std::wstring hsStr = std::to_wstring(score.getHighScore());
    drawText(hdc, hsStr.c_str(), px, textY, 120, 30, COLOR_TEXT, 22);
    textY += 40;

    // 等级
    drawText(hdc, L"LEVEL", px, textY, 120, 20, COLOR_TEXT_DIM, 12);
    textY += 22;
    std::wstring lvlStr = std::to_wstring(score.getLevel());
    drawText(hdc, lvlStr.c_str(), px, textY, 120, 30, COLOR_TEXT, 22);
    textY += 40;

    // 消除行数
    drawText(hdc, L"LINES", px, textY, 120, 20, COLOR_TEXT_DIM, 12);
    textY += 22;
    std::wstring linesStr = std::to_wstring(score.getLinesCleared());
    drawText(hdc, linesStr.c_str(), px, textY, 120, 30, COLOR_TEXT, 22);
    textY += 60;

    // 下一个方块
    drawText(hdc, L"NEXT", px, textY, 120, 20, COLOR_TEXT_DIM, 12);
    textY += 30;

    if (nextPiece) {
        drawNextPiecePreview(hdc, nextPiece, textY);
    }

    // 操作提示
    textY = WINDOW_HEIGHT - 170;
    drawText(hdc, L"CONTROLS:", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"\u2190\u2192  Move", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"\u2191  Rotate", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"\u2193  Soft Drop", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"SPACE  Hard Drop", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"Z  Rotate CCW", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
    textY += 18;
    drawText(hdc, L"P  Pause", px, textY, 120, 15, COLOR_TEXT_DARK, 12);
}

// ============================================================
// 绘制下一个方块预览
// ============================================================
void Renderer::drawNextPiecePreview(HDC hdc, const Tetromino* piece, int baseY) {
    int baseX = BOARD_PIXEL_WIDTH + 35;
    int cellSize = 22;

    const auto& offsets = piece->getBlockOffsets();
    Color color = piece->getColor();

    for (const auto& block : offsets) {
        int r = block.first;
        int c = block.second;
        int px = baseX + c * cellSize;
        int py = baseY + r * cellSize;
        drawFilledRect(hdc, px, py, cellSize - 2, cellSize - 2, color);
    }
}
