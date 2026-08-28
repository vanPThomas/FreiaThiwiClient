#pragma once
#include <cstring>


class ChatRoom
{
public:
    ChatRoom();
    ~ChatRoom();
private:
    std::string ChatRoomName;
    std::string Password;
    std::vector<std::string> chatMessages;

}