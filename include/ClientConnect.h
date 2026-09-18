#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "FreiaEncryption.h"
#include <unordered_set>
#include <chrono>
#include <thread>
#include "ChatRoom.h"


class ClientConnect
{
public:
    ClientConnect();
    ClientConnect(const std::string& ip, const std::string& portStr, const std::string& user, const std::string& chatPassword);
    ~ClientConnect();

    bool connectToServer();
    void disconnect();
    void sendMessage(const std::string& text);

    const std::vector<std::string>& getMessages() const;
    bool isConnectedToServer() const { return isConnected; }

    bool configureWithAccount(
        const std::string& ip,
        const std::string& port,
        const std::string& user,
        const std::string& chatPassword,
        const std::string& serverPassword,
        const std::string& accountPassword,
        bool isCreate);

    void createRoom(std::string ChatRoomName, std::string ChatRoomPassword);
    bool connectToRoom(std::string chatRoomName, std::string chatRoomPassword);
    
    // ====================
    // GETTERS
    // ====================

    const std::unordered_set<std::string>& getOnlineUsers() const { return onlineUsers; }
    const std::vector<std::string>& getChatRooms() const { return onlineRooms; }
    const std::vector<ChatRoom>& getConnectedChatRooms() const { return connectedChatRooms; }
    bool getIsConnected() const { return isConnected; }

    void sendMessageToRoom(const std::string& roomName, const std::string& text);
    
private:

    void handleSystemCallError(const std::string& errorMsg);
    int createClientSocket(const std::string &serverIP, int serverPort);
    void receiveMessages();
    void addMessage(const std::string& message);
    void handleProtocolPacket(const std::string& encryptedData);
    std::vector<std::string> splitByNewline(const std::string& s);

    // Protocol framework
    std::string buildProt1Frame(const std::string& ciphertext) const;
    std::string buildProt2Frame() const;
    std::string buildProt4Frame() const;
    std::string buildProt5Frame(const std::string& messageType, const ChatRoom& chatRoom);
    std::string createProt5Message(ChatRoom& room, const std::string& text);


    bool processProt5Room(
        const std::vector<std::string>& parts,
        std::string& chatRoomName,
        std::string& password,
        std::vector<std::string>& chatMessages,
        std::vector<std::string>& connectedUsers,
        std::string& roomCreator,
        std::string& roomCreationTime);

    uint16_t safeParsePort(const std::string& s);
    bool sendWithLengthPrefix(int sock, const std::string& data);

    int clientSocket = -1;
    bool isConnected = false;

    mutable std::mutex chatMutex;
    std::vector<std::string> chatMessages;
    std::vector<ChatRoom> chatRooms;
    std::vector<ChatRoom> connectedChatRooms;
    
    std::string ip;
    int port;
    std::string user;
    std::string chatPassword;
    std::string serverPassword;
    std::string accountPassword;
    
    const int bufferSize = 10240;
    
    FreiaEncryption::Key sessionKey{};          //E2EE
    FreiaEncryption::Key serverSessionKey{};    //Transport
    FreiaEncryption::Key accountSessionKey{};   //account
    bool hasChatKey = false;
    bool hasServerKey = false;
    bool hasAccountKey = false;
    bool isCreateMode = false;
    
    std::vector<std::string> onlineRooms;
    std::unordered_set<std::string> onlineUsers;
    bool showUserList = true;
};