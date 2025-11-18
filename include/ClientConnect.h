#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ClientConnect
{
public:
    ClientConnect();
    ClientConnect(const char* ip, const char* port, const char* user, const char* chatPassword);
    ~ClientConnect();

    bool connectToServer();
    void disconnect();
    void sendMessage(const std::string& text);

    const std::vector<std::string>& getMessages() const;
    bool isConnectedToServer() const { return isConnected; }
    bool configure(const char* ip, const char* port, const char* user, const char* chatPassword);


private:
    void handleSystemCallError(const std::string& errorMsg);
    int createClientSocket(const std::string &serverIP, int serverPort);
    void receiveMessages();
    void addMessage(const std::string& message);

    int clientSocket = -1;
    bool isConnected = false;

    mutable std::mutex chatMutex;
    std::vector<std::string> chatMessages;

    std::string ip;
    int port;
    std::string user;
    std::string chatPassword;

    const int bufferSize = 10240;
};
