#pragma once
#include <string>

namespace Validation
{
    [[nodiscard]] bool isValidIP(const std::string& ip);
    [[nodiscard]] bool isValidPort(const std::string& portStr);
    [[nodiscard]] bool isValidUser(const std::string& user);
    [[nodiscard]] bool isValidPassword(const std::string& password);
    [[nodiscard]] std::string sanitizeUsername(const std::string& input);
}