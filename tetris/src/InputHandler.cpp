#include "InputHandler.h"

// ============================================================
// 构造函数
// ============================================================
InputHandler::InputHandler() {
    reset();
}

// ============================================================
// 更新按键状态
// ============================================================
void InputHandler::update() {
    for (int i = 0; i < 256; ++i) {
        m_lastState[i] = m_currentState[i];
        m_currentState[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}

// ============================================================
// 查询按键是否刚被按下（边沿触发）
// ============================================================
bool InputHandler::isKeyPressed(int vkKey) {
    if (vkKey < 0 || vkKey >= 256) return false;
    return m_currentState[vkKey] && !m_lastState[vkKey];
}

// ============================================================
// 查询按键是否被按住（电平触发）
// ============================================================
bool InputHandler::isKeyHeld(int vkKey) {
    if (vkKey < 0 || vkKey >= 256) return false;
    return m_currentState[vkKey];
}

// ============================================================
// 重置所有输入状态
// ============================================================
void InputHandler::reset() {
    for (int i = 0; i < 256; ++i) {
        m_lastState[i] = false;
        m_currentState[i] = false;
    }
}
