#pragma once
#include <cstring>


class ChatRoom
{
public:
    ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages);
    ~ChatRoom();

    const std::std::string getChatRoomNames() const {
        return chatRoomName;
    }


private:
    std::string chatRoomName;
    std::string password;
    std::vector<std::string> chatMessages;

}