#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <windows.h>

// 键盘输入处理器
class InputHandler {
public:
    InputHandler();

    // 更新按键状态（每帧调用）
    void update();

    // 查询按键是否刚被按下（仅一帧）
    bool isKeyPressed(int vkKey);

    // 查询按键是否被按住
    bool isKeyHeld(int vkKey);

    // 重置所有输入状态
    void reset();

private:
    bool m_lastState[256] = {false};
    bool m_currentState[256] = {false};
};

#endif // INPUTHANDLER_H
