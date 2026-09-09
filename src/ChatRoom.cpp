#include "ChatRoom.h"

#include <utility>

ChatRoom::ChatRoom(std::string chatRoomName, std::vector<std::string> chatMessages)
    : chatRoomName(chatRoomName)
    , chatMessages(chatMessages)
{
}

ChatRoom::ChatRoom(std::string chatRoomName, std::string chatRoomPassword)
    : chatRoomName(chatRoomName)
    , password(chatRoomPassword)
{
}