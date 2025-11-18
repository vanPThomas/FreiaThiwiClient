#pragma once
#include <string>

namespace Validation
{
    bool isValidIP(const std::string& ip);
    bool isValidPort(const std::string& portStr);
    bool isValidUser(const std::string& user);
    bool isValidPassword(const std::string& password);
}