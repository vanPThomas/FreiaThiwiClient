#include "ClientConnect.h"
#include "Validation.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h> 

ClientConnect::ClientConnect(){}
ClientConnect::ClientConnect(const char* ip,
                             const char* port,
                             const char* user,
                             const char* password)
    : ip(ip), port(std::atoi(port)), user(user), password(password) {}

ClientConnect::~ClientConnect()
{
    disconnect();
}

void ClientConnect::handleSystemCallError(const std::string &errorMsg)
{
    std::cerr << errorMsg << ", errno: " << errno << "\n";
}

int ClientConnect::createClientSocket(const std::string &serverIP, int serverPort)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        handleSystemCallError("Failed to create socket");
        return -1;
    }

    // Set timeout (3 seconds example)
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0)
    {
        handleSystemCallError("Invalid IP address or unsupported format");
        close(sock);
        return -1;
    }

    if (connect(sock, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        handleSystemCallError("Connection failed");
        close(sock);
        return -1;
    }

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    addMessage("[Connected to server]");


    return sock;
}


bool ClientConnect::connectToServer()
{
    clientSocket = createClientSocket(ip, port);
    if (clientSocket == -1)
        return false;

    isConnected = true;
    std::thread(&ClientConnect::receiveMessages, this).detach();
    return true;
}

void ClientConnect::disconnect()
{
    if (isConnected)
    {
        isConnected = false;
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
    }
}
void ClientConnect::receiveMessages()
{
    char buffer[bufferSize];

    while (isConnected)
    {
        int bytesRead = recv(clientSocket, buffer, bufferSize - 1, 0);

        if (bytesRead <= 0)
        {
            addMessage("[Disconnected from server]");
            isConnected = false;
            break;
        }

        buffer[bytesRead] = '\0';
        addMessage(std::string(buffer));
    }

    disconnect();
}

void ClientConnect::addMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(chatMutex);
    chatMessages.push_back(message);
}

void ClientConnect::sendMessage(const std::string& text)
{
    if (!isConnected || text.empty())
        return;

    std::string fullMsg = user + ": " + text + "\n";
    addMessage(fullMsg);
    if (clientSocket != -1)
        send(clientSocket, fullMsg.data(), fullMsg.size(), 0);
}

const std::vector<std::string>& ClientConnect::getMessages() const
{
    std::lock_guard<std::mutex> lock(chatMutex);
    return chatMessages;
}

bool ClientConnect::configure(const char* ip, const char* port, const char* user, const char* password)
{
    if (!Validation::isValidIP(ip)) return false;
    if (!Validation::isValidPort(port)) return false;
    if (!Validation::isValidUser(user)) return false;
    if (!Validation::isValidPassword(password)) return false;

    int p = std::atoi(port);
    // Validate IPv4 format using inet_pton test
    // sockaddr_in sa{};
    // if (inet_pton(AF_INET, ip, &(sa.sin_addr)) <= 0)
    //     return false;

    this->ip = ip;
    this->port = p;
    this->user = user;
    this->password = password ? password : "";

    return true;
}
