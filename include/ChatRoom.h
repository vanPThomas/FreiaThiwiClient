#pragma once
#include <string>
#include <vector>



class ChatRoom
{
public:
    ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages);
    ChatRoom(std::string chatRoomName, std::string chatRoomPassword);

    const std::string getChatRoomNames() const
    {
        return chatRoomName;
    }


private:
    std::string chatRoomName;
    std::string password;
    std::vector<std::string> chatMessages;
};