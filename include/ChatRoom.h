#pragma once
#include <cstring>


class ChatRoom
{
public:
    ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages);
    ~ChatRoom();
private:
    std::string ChatRoomName;
    std::string Password;
    std::vector<std::string> ChatMessages;

}