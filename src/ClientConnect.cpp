#include "ClientConnect.h"
#include "Validation.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>

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
    if (!hasServerKey) {
        addMessage("[Error] No server password set");
        return false;
    }

    // 1. build and encrypt the package
    std::string frame = buildProt2Frame();
    std::string transportCipher = FreiaEncryption::encryptData(frame, serverSessionKey);
    if (transportCipher.empty()) {
        addMessage("[Error] Failed to encrypt handshake (transport)");
        return false;
    }

    // 2. connect TCP
    clientSocket = createClientSocket(ip, port);
    if (clientSocket == -1)
        return false;

    // 3. Send handshake with length prefix
    uint32_t len = transportCipher.size();
    uint32_t netLen = htonl(len);
    if (send(clientSocket, &netLen, sizeof(netLen), 0) != sizeof(netLen) ||
        send(clientSocket, transportCipher.data(), transportCipher.size(), 0) != static_cast<ssize_t>(transportCipher.size())) {
        addMessage("[Error] Failed to send handshake to server");
        disconnect();
        return false;
    }

    addMessage("[Info] Handshake sent, waiting for server authentication...");

    // 4. Wait for server's reply (welcome / OK packet) — blocking read here
    uint32_t replyLenNet = 0;
    int r = recv(clientSocket, &replyLenNet, sizeof(replyLenNet), MSG_WAITALL);
    if (r != sizeof(replyLenNet)) {
        addMessage("[Auth failed] Server did not respond or connection dropped");
        disconnect();
        return false;
    }

    uint32_t replyLen = ntohl(replyLenNet);
    if (replyLen == 0 || replyLen > 65536) {  // reasonable max for small reply
        addMessage("[Auth failed] Invalid reply length from server");
        disconnect();
        return false;
    }

    std::string replyCipher(replyLen, '\0');
    r = recv(clientSocket, replyCipher.data(), replyLen, MSG_WAITALL);
    if (r != static_cast<int>(replyLen)) {
        addMessage("[Auth failed] Incomplete server reply");
        disconnect();
        return false;
    }

    // 5. Decrypt server's reply
    std::string replyPlain = FreiaEncryption::decryptData(replyCipher, serverSessionKey);
    if (replyPlain.empty()) {
        addMessage("[Auth failed] Server reply decryption failed - wrong server password?");
        disconnect();
        return false;
    }

    // 6. Check content (minimal check — just starts with "OK" or exact match)
    if (replyPlain != "PROT2" && !replyPlain.starts_with("PROT2\n")) {
        addMessage("[Auth failed] Invalid server response: " + replyPlain.substr(0, 50));
        disconnect();
        return false;
    }

    // Success!
    addMessage("[Connected & authenticated]");
    isConnected = true;
    std::vector<std::string> lines = splitByNewline(replyPlain);
    addMessage(lines[1]);

    // Now safe to start background receive thread for normal messages
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
    if (!isConnected || text.empty()) {
        return;
    }

    if (text.size() > 16384) {
        addMessage("[Error] Message too long");
        return;
    }

    // 1. Build & encrypt inner payload
    std::string chatCipher = FreiaEncryption::encryptData(text, sessionKey);
    if (chatCipher.empty()) {
        addMessage("[Error] Failed to encrypt message (E2EE)");
        return;
    }

    std::string frame = buildProt1Frame(chatCipher);

    // 2. Encrypt for transport
    std::string transportCipher = FreiaEncryption::encryptData(frame, serverSessionKey);
    if (transportCipher.empty()) {
        addMessage("[Error] Failed to encrypt message (transport)");
        return;
    }

    // 3. Send with length prefix
    uint32_t len = transportCipher.size();
    uint32_t netLen = htonl(len);

    if (send(clientSocket, &netLen, sizeof(netLen), 0) != sizeof(netLen) ||
        send(clientSocket, transportCipher.data(), transportCipher.size(), 0) != static_cast<ssize_t>(transportCipher.size())) {
        addMessage("[Error] Failed to send to server");
        isConnected = false;
        return;
    }

    // 4. Local echo
    addMessage(user + ": " + text);
    return;
}

std::string ClientConnect::buildProt1Frame(const std::string& ciphertext) const
{
    std::string frame = "PROT1\n";
    frame += user;
    frame += '\n';
    frame += std::to_string(ciphertext.size()); // ciphertext size
    frame += '\n';
    frame += ciphertext;
    return frame;
}

std::string ClientConnect::buildProt2Frame() const
{
    std::string frame = "PROT2\n";
    frame += user;
    return frame;
}

const std::vector<std::string>& ClientConnect::getMessages() const
{
    std::lock_guard<std::mutex> lock(chatMutex);
    return chatMessages;
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
    else if (proto == "PROT3")
    {
        if (parts.size() < 3) {
            addMessage("[Protocol error] Malformed PROT3");
            return;
        }

        const std::string& msgType = parts[1];
        const std::string& payload = parts[2];

        if (msgType == "userList")
        {
            std::string payloadList;
            for (size_t i = 2; i < parts.size(); ++i) {
                if (i > 2) payloadList += "\n";
                payloadList += parts[i];
            }
            std::cout << "TESTTEST!" << "\n";
            std::cout << "Payload: " << payloadList << "\n";
            onlineUsers.clear();
            auto names = splitByNewline(payloadList);
            for (const auto& name : names) {
                if (!name.empty()) {
                    onlineUsers.insert(name);
                }
            }
            addMessage("[User list received — " + std::to_string(onlineUsers.size()) + " online]");
        }
        else if (msgType == "userJoined")
        {
            if (!payload.empty())
            {
                onlineUsers.insert(payload);
                addMessage("[Joined] " + payload);
            }
        }
        else if (msgType == "userLeft")
        {
            if (!payload.empty() && onlineUsers.erase(payload))
            {
                addMessage("[Left] " + payload);
            }
        }
        else if (msgType == "userDisconnected") {
            // your existing disconnect message
            addMessage("[Server] " + payload);
        }
        else {
            // fallback for any other server notice
            addMessage("[Server notice] " + payload + " (" + msgType + ")");
        }
    }
    else
    {
        addMessage("[Unknown protocol] " + proto);
    }
}

std::vector<std::string> ClientConnect::splitByNewline(const std::string& s) {
    std::vector<std::string> lines;
    std::string line;
    std::istringstream iss(s);
    while (std::getline(iss, line)) {
        if (!line.empty() || !lines.empty()) {
            lines.push_back(std::move(line));
        }
    }
    return lines;
}

bool ClientConnect::configureWithAccount(
    const char* ip,
    const char* port,
    const char* user,
    const char* chatPassword,
    const char* serverPassword,
    const char* accountPassword)
{
    ConnectionParamsWithAccount p;

    // Validation checks first (fail early)
    if (!Validation::isValidIP(ip))          return false;
    if (!Validation::isValidPort(port))      return false;
    if (!Validation::isValidUser(user))      return false;
    if (!Validation::isValidPassword(chatPassword))   return false;
    if (!Validation::isValidPassword(serverPassword)) return false;
    if (!Validation::isValidPassword(accountPassword)) return false;

    // Assign sanitized / safe values
    p.ip         = ip ? ip : "";
    p.port       = port ? std::atoi(port) : 0;
    p.user       = Validation::sanitizeUsername(user ? user : "");
    p.chat_pw    = chatPassword ? chatPassword : "";
    p.server_pw  = serverPassword ? serverPassword : "";
    p.account_pw = accountPassword ? accountPassword : "";

    // Move into member variables
    this->ip             = std::move(p.ip);
    this->port           = p.port;
    this->user           = std::move(p.user);
    this->chatPassword   = std::move(p.chat_pw);
    this->serverPassword = std::move(p.server_pw);
    this->accountPassword = std::move(p.account_pw);

    // Derive keys from the cleaned values
    sessionKey       = FreiaEncryption::deriveKey(this->chatPassword);
    serverSessionKey = FreiaEncryption::deriveKey(this->serverPassword);
    accountSessionKey = FreiaEncryption::deriveKey(this->accountPassword);

    hasChatKey   = !this->chatPassword.empty();
    hasServerKey = !this->serverPassword.empty();
    hasAccountKey = !this->accountPassword.empty();

    return hasChatKey && hasServerKey && hasAccountKey;

    // return configure(ip, port, user, chatPassword, ""); // fallback to old configure for now
}

bool ClientConnect::configureForCreate(
    const char* ip,
    const char* port,
    const char* user,
    const char* chatPassword,
    const char* serverPassword,
    const char* accountPassword)
{
    // TODO: implement account creation
    return configure(ip, port, user, chatPassword, ""); // fallback
}

bool ClientConnect::configure(
    const char* ip,
    const char* port,
    const char* user,
    const char* chatPassword,
    const char* serverPassword)
{
    ConnectionParams p;

    // Validation checks first (fail early)
    if (!Validation::isValidIP(ip))          return false;
    if (!Validation::isValidPort(port))      return false;
    if (!Validation::isValidUser(user))      return false;
    if (!Validation::isValidPassword(chatPassword))   return false;
    if (!Validation::isValidPassword(serverPassword)) return false;

    // Assign sanitized / safe values
    p.ip         = ip           ? ip           : "";
    p.port       = port         ? std::atoi(port) : 0;
    p.user       = Validation::sanitizeUsername(user ? user : "");
    p.chat_pw    = chatPassword ? chatPassword : "";
    p.server_pw  = serverPassword ? serverPassword : "";

    // Move into member variables
    this->ip             = std::move(p.ip);
    this->port           = p.port;
    this->user           = std::move(p.user);
    this->chatPassword   = std::move(p.chat_pw);
    this->serverPassword = std::move(p.server_pw);

    // Derive keys from the cleaned values
    sessionKey       = FreiaEncryption::deriveKey(this->chatPassword);
    serverSessionKey = FreiaEncryption::deriveKey(this->serverPassword);

    hasChatKey   = !this->chatPassword.empty();
    hasServerKey = !this->serverPassword.empty();

    return hasChatKey && hasServerKey;
}