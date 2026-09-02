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
    
    const std::unordered_set<std::string>& getOnlineUsers() const {
        return onlineUsers;
    }

    const std::vector<std::ChatRoom>& getChatRooms() const {
        return chatRooms;
    }

    bool getIsConnected() const {
        return isConnected;
    }
    
private:

    void handleSystemCallError(const std::string& errorMsg);
    int createClientSocket(const std::string &serverIP, int serverPort);
    void receiveMessages();
    void addMessage(const std::string& message);
    void handleProtocolPacket(const std::string& encryptedData);
    std::vector<std::string> splitByNewline(const std::string& s);
    std::string buildProt1Frame(const std::string& ciphertext) const;
    std::string buildProt2Frame() const;
    uint16_t safeParsePort(const std::string& s);
    bool sendWithLengthPrefix(int sock, const std::string& data);

    int clientSocket = -1;
    bool isConnected = false;

    mutable std::mutex chatMutex;
    std::vector<std::string> chatMessages;
    std::vector<std::ChatRoom> chatRooms; 

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

    std::unordered_set<std::string> onlineUsers;
    bool showUserList = true;
};
