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
        static constexpr uint32_t MAX_PACKET = 10 * 1024 * 1024;
        if (len == 0 || len > MAX_PACKET)
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

        // 3) Handle Package
        if (!hasChatKey)
        {
            addMessage("[Error] Received encrypted message but no password is set.");
            continue;
        }
        
        handleProtocolPacket(encryptedData);
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

    // 1. Encrypt chat message (E2EE)
    std::string chatCipher = FreiaEncryption::encryptData(text, sessionKey);
    if (chatCipher.empty())
    {
        addMessage("[Error] Chat encryption failed.");
        return;
    }
    // 2. Build PROT1 frame (plaintext to server)

    std::string frame = "PROT1\n" + user + "\n" + std::to_string(chatCipher.size()) + "\n";
    frame.append(chatCipher);

    // 3. Encrypt with SERVER password (transport layer)
    std::string transportCipher = FreiaEncryption::encryptData(frame, serverSessionKey);

    if (transportCipher.empty())
    {
        addMessage("[Error] Server-layer encryption failed.");
        return;
    }

    // 4. Length prefix + send

    uint32_t len = transportCipher.size();
    uint32_t netLen = htonl(len); // convert to network byte order

    send(clientSocket, &netLen, sizeof(netLen), 0);
    send(clientSocket, transportCipher.data(), transportCipher.size(), 0);

    // 5. Local echo (PLAINTEXT)
    addMessage(user + ": " + text);
}

const std::vector<std::string>& ClientConnect::getMessages() const
{
    std::lock_guard<std::mutex> lock(chatMutex);
    return chatMessages;
}

bool ClientConnect::configure(
    const char* ip,
    const char* port,
    const char* user,
    const char* chatPassword,
    const char* serverPassword)
{
    if (!Validation::isValidIP(ip)) return false;
    if (!Validation::isValidPort(port)) return false;
    if (!Validation::isValidUser(user)) return false;
    if (!Validation::isValidPassword(chatPassword)) return false;
    if (!Validation::isValidPassword(serverPassword)) return false;

    int p = std::atoi(port);

    this->ip   = ip;
    this->port = p;
    this->user = user;

    this->chatPassword   = chatPassword   ? chatPassword   : "";
    this->serverPassword = serverPassword ? serverPassword : "";

    // Derive Chat Session Key
    if (!this->chatPassword.empty())
    {
        sessionKey = FreiaEncryption::deriveKey(this->chatPassword);
        hasChatKey = true;
    }
    else
    {
        hasChatKey = false;
    }

    // Derive Server Session Key
    if (!this->serverPassword.empty())
    {
        serverSessionKey = FreiaEncryption::deriveKey(this->serverPassword);
        hasServerKey = true;
    }
    else
    {
        hasServerKey = false;
    }

    return hasChatKey && hasServerKey;
}

void ClientConnect::handleProtocolPacket(const std::string& encryptedData)
{
    std::string plaintext =
        FreiaEncryption::decryptData(encryptedData, serverSessionKey);

    if (plaintext.empty()) {
        addMessage("[Decryption failed]");
        return;
    }

    auto parts = splitByNewline(plaintext);
    if (parts.empty()) {
        addMessage("[Protocol error] empty packet.");
        return;
    }

    const std::string& proto = parts[0];

    if (proto == "PROT1")
    {
        // We expect at least:
        // 0: "PROT1"
        // 1: username
        // 2: length
        // plus ciphertext bytes after the third newline
        if (parts.size() < 3) {
            addMessage("[Protocol error] malformed PROT1 header.");
            return;
        }

        const std::string& messageUser = parts[1];

        size_t len = 0;
        try {
            len = std::stoul(parts[2]);
        } catch (...) {
            addMessage("[Protocol error] invalid length in PROT1.");
            return;
        }

        if (len == 0 || len > plaintext.size()) {
            addMessage("[Protocol error] PROT1 length out of range.");
            return;
        }

        // Ciphertext is the last `len` bytes of the plaintext frame
        std::string cipher = plaintext.substr(plaintext.size() - len);

        std::string text = FreiaEncryption::decryptData(cipher, sessionKey);
        if (text.empty()) {
            addMessage("[Chat decryption failed]");
            return;
        }

        addMessage(messageUser + ": " + text);
    }
    else
    {
        addMessage("[Unknown protocol] " + proto);
    }
}



std::vector<std::string> ClientConnect::splitByNewline(const std::string& s)
{
    std::vector<std::string> out;
    std::string current;

    for (char c : s)
    {
        if (c == '\n')
        {
            out.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
        out.push_back(current);

    return out;
}