#include "ChatRoom.h"

#include <utility>

ChatRoom::ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages)
    : chatRoomName(chatRoomName)
    , chatMessages(chatMessages){}

ChatRoom::ChatRoom(std::string chatRoomName, std::string chatRoomPassword)
    : chatRoomName(chatRoomName)
    , password(chatRoomPassword)
{
    roomKey = FreiaEncryption::deriveKey(chatRoomPassword);
}

ChatRoom::ChatRoom(std::string chatRoomName, std::string password, std::vector<std::string> chatMessages, std::vector<std::string> connectedUsers, std::string roomCreator, std::string roomCreationTime)
    : chatRoomName(std::move(chatRoomName))
    , password(std::move(password))
    , chatMessages(std::move(chatMessages))
    , connectedUsers(std::move(connectedUsers))
    , roomCreator(std::move(roomCreator))
    , roomCreationTime(std::move(roomCreationTime))
    {
        roomKey = FreiaEncryption::deriveKey(password);
    }

void ChatRoom::addMessage(std::string message)
{
    chatMessages.push_back(message);
}

void ChatRoom::addConnectedUser(std::string user)
{
    connectedUsers.push_back(user);
}