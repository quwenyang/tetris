#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include "Board.h"
#include "Tetromino.h"
#include "ScoreManager.h"

// 渲染器：使用 Win32 GDI + 持久化双缓冲绘制游戏画面
class Renderer {
public:
    Renderer();
    ~Renderer();

    // 初始化双缓冲（在窗口创建后调用一次）
    bool initBuffer(HWND hwnd);

    // 绘制完整游戏场景到双缓冲，然后 BitBlt 到屏幕
    // isPaused 为 true 时叠加暂停遮罩后再输出（避免两次 BitBlt 导致闪烁）
    void draw(HWND hwnd, const Board& board, const Tetromino& currentPiece,
              const Tetromino* nextPiece, const ScoreManager& score,
              int ghostRow, bool isGameOver, bool isPaused = false);

    // 从双缓冲 BitBlt 到屏幕（供 WM_PAINT 调用）
    void blitToScreen(HWND hwnd);

private:
    // 绘制面板背景和网格
    void drawBoard(HDC hdc);

    // 绘制面板中已锁定的方块
    void drawCells(HDC hdc, const Board& board);

    // 绘制单个方块
    void drawPiece(HDC hdc, const Tetromino& piece, int row, int col,
                   Color color, int alpha = 255);

    // 绘制幽灵方块
    void drawGhostPiece(HDC hdc, const Tetromino& piece, int ghostRow);

    // 绘制右侧信息面板
    void drawSidePanel(HDC hdc, const ScoreManager& score,
                       const Tetromino* nextPiece);

    // 绘制下一个方块预览（使用动态 y 坐标以避免重叠）
    void drawNextPiecePreview(HDC hdc, const Tetromino* piece, int baseY);

    // 绘制带颜色的矩形（GDI 辅助函数）
    void drawFilledRect(HDC hdc, int x, int y, int w, int h, Color color);
    void drawRect(HDC hdc, int x, int y, int w, int h, Color color);

    // 绘制文本
    void drawText(HDC hdc, const wchar_t* text, int x, int y,
                  int w, int h, Color color, int fontSize = 16);

    // 创建带alpha的颜色
    COLORREF toColorRef(Color color, int alpha = 255) const;

    // 持久化双缓冲对象
    bool m_bufferReady;
    HDC m_memDC;         // 内存 DC
    HBITMAP m_memBitmap; // 兼容位图
    HBITMAP m_oldBitmap; // 原 DC 中的旧位图，析构时恢复
    int m_width;
    int m_height;

    // 字体缓存
    HFONT m_font;
    HFONT m_fontSmall;
    HFONT m_fontLarge;
    HFONT m_fontTitle;
};

#endif // RENDERER_H
