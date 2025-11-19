#include "ClientConnect.h"
#include "Validation.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h> 

ClientConnect::ClientConnect(){}
ClientConnect::ClientConnect(const char* ip,
                             const char* port,
                             const char* user,
                             const char* chatPassword)
    : ip(ip), port(std::atoi(port)), user(user), chatPassword(chatPassword) {}

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

// void ClientConnect::receiveMessages()
// {
//     while (isConnected)
//     {
//         uint32_t netLen = 0;
//         int r = recv(clientSocket, &netLen, sizeof(netLen), MSG_WAITALL);
//         if (r <= 0) break;

//         uint32_t len = ntohl(netLen);
//         if (len == 0 || len > bufferSize) break; // sanity check

//         std::string msg(len, '\0');
//         r = recv(clientSocket, msg.data(), len, MSG_WAITALL);
//         if (r <= 0) break;

//         addMessage(msg);
//     }

//     addMessage("[Disconnected from server]");
//     disconnect();
// }

void ClientConnect::receiveMessages()
{
    while (isConnected)
    {
        // 1) Read length prefix
        uint32_t netLen = 0;
        int r = recv(clientSocket, &netLen, sizeof(netLen), MSG_WAITALL);
        if (r <= 0)
        {
            addMessage("[Disconnected from server]");
            isConnected = false;
            break;
        }

        uint32_t len = ntohl(netLen);
        if (len == 0 || len > bufferSize)
        {
            addMessage("[Error] Invalid message length received.");
            isConnected = false;
            break;
        }

        // 2) Read encryptedData payload
        std::string encryptedData(len, '\0');
        r = recv(clientSocket, encryptedData.data(), len, MSG_WAITALL);
        if (r <= 0)
        {
            addMessage("[Disconnected from server]");
            isConnected = false;
            break;
        }

        // 3) Decrypt
        if (!hasKey)
        {
            addMessage("[Error] Received encrypted message but no password is set.");
            continue;
        }

        std::string plaintext = FreiaEncryption::decryptData(encryptedData, sessionKey);
        if (plaintext.empty())
        {
            addMessage("[Decryption failed]");
            continue;
        }

        // 4) Show decrypted text in the chat
        addMessage(plaintext);
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

    //encrypt message
    std::string encryptedMessage = FreiaEncryption::encryptData(fullMsg, sessionKey);
    if(encryptedMessage.empty())
    {
        addMessage("[Error] Encryption failed.");
        return;
    }

    uint32_t len = encryptedMessage.size();
    uint32_t netLen = htonl(len); // convert to network byte order

    send(clientSocket, &netLen, sizeof(netLen), 0);
    send(clientSocket, encryptedMessage.data(), encryptedMessage.size(), 0);
}

const std::vector<std::string>& ClientConnect::getMessages() const
{
    std::lock_guard<std::mutex> lock(chatMutex);
    return chatMessages;
}

bool ClientConnect::configure(const char* ip, const char* port, const char* user, const char* chatPassword)
{
    if (!Validation::isValidIP(ip)) return false;
    if (!Validation::isValidPort(port)) return false;
    if (!Validation::isValidUser(user)) return false;
    if (!Validation::isValidPassword(chatPassword)) return false;

    int p = std::atoi(port);


    this->ip = ip;
    this->port = p;
    this->user = user;
    this->chatPassword = chatPassword ? chatPassword : "";

    if (!this->chatPassword.empty()) {
        sessionKey = FreiaEncryption::deriveKey(this->chatPassword);
        hasKey = true;
    } else {
        hasKey = false;
    }

    return true;
}
