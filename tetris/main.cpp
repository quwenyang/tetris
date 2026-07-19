#include <windows.h>
#include "Game.h"

// ============================================================
// 程序入口（Windows 应用程序）
// ============================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    Game game(hInstance);
    game.run();
    return 0;
}
