#include "Validation.h"
#include <cstdlib>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>


bool Validation::isValidIP(const std::string& ip)
{
    int dots = 0;
    for (int i = 0; ip[i]; i++)
    {
        if (ip[i] == '.') dots++;
        else if (!isdigit(ip[i])) return false;
    }
    if (dots != 3)
        return false;

    sockaddr_in sa{};
    if (inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) <= 0)
        return false;

    return true;
}

bool Validation::isValidPort(const std::string& portStr)
{
    if (portStr.empty())
    {
        std::cout << "error1\n";
        return false;
    }

    for (char c : portStr)
        if (!isdigit(c))
        {
            std::cout << "error2\n";
            return false;
        }

    int port = std::atoi(portStr.c_str());
    if (port < 1 || port > 65535)
    {
        std::cout << "error3\n";
        return false;
    }

    return true;
}

bool Validation::isValidUser(const std::string& user)
{
    return !user.empty() && user.size() <= 32;
}

bool Validation::isValidPassword(const std::string& password)
{
    return !password.empty() && password.size() <= 128;
}
