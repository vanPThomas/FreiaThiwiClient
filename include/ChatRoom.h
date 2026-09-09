#pragma once
#include <string>
#include <vector>



class ChatRoom
{
public:
    ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages);
    ChatRoom(std::string chatRoomName, std::string chatRoomPassword);

    const std::string getChatRoomName() const
    {
        return chatRoomName;
    }

    const std::vector<std::string> getChatRoomMessages() const
    {
        return chatMessages;
    }

    void addMessage(std::string message);

private:
    std::string chatRoomName;
    std::string password;
    std::vector<std::string> chatMessages;
};