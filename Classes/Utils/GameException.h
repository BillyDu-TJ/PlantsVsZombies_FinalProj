// 异常类，用于处理游戏中的错误情况
// 2025.11.17 by BillyDu
#ifndef __GAME_EXCEPTION_H__
#define __GAME_EXCEPTION_H__

#include <stdexcept>
#include <string>

// 自定义游戏异常类，用于处理逻辑错误或数据加载失败
class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& message)
        : std::runtime_error("GameException: " + message) {}
};

#endif // __GAME_EXCEPTION_H__