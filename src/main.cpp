#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>

void handleSystemCallError(std::string errorMsg);
int createClientSocket(const std::string &serverIP, int serverPort);
void receiveMessages(int clientSocket);
void connectToServerThread();
void AddMessage(const char *message);
void connectToServer();
void sendMessage();


const int bufferSize = 10240;
std::mutex chatMutex;
std::vector<std::string> chatMessages; // Store chat messages
char inputBuffer[1024] = "";           // Buffer for input text
char IP[20] = "";
char Port[10] = "";
char User[50] = "";
char Password[1000] = "";

bool focusInput = false; // Flag to set keyboard focus
bool isConnected = false;
char buffer[bufferSize];
char outMessage[bufferSize];
int clientSocket;

// Main code
int main(int, char **)
{

    if (!glfwInit())
        return 1;

    // Create a GLFW window
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Freia Thiwi", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load custom font (must stay alive/loaded while using it)
    ImFont *customFont = io.Fonts->AddFontFromFileTTF("fonts/Px437_IBM_VGA_8x14.ttf", 18.0f);
    if (!customFont)
    {
        return 1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(customFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.5f, 1.0f)); // Red color
        {
            const float labelWidth = 220.0f;
            ImGui::Begin("Connection Data", NULL, ImGuiWindowFlags_None);
            ImGui::Text("IP: ");
            ImGui::SameLine(labelWidth);
            ImGui::InputText("##IP", IP, IM_ARRAYSIZE(IP), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Text("Port: ");
            ImGui::SameLine(labelWidth);
            ImGui::InputText("##PORT", Port, IM_ARRAYSIZE(Port), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Text("User Name: ");
            ImGui::SameLine(labelWidth);
            ImGui::InputText("##USERNAME", User, IM_ARRAYSIZE(User), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Text("Encryption Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::InputText("##ENCRYPTIONPASSWORD", Password, IM_ARRAYSIZE(Password), ImGuiInputTextFlags_EnterReturnsTrue);

            if (!isConnected)
            {
                if (ImGui::Button("Connect"))
                {
                    connectToServer();
                }
            }

            ImGui::End();
        }
        {
            // Chat window
            ImGui::Begin("Chat Window", NULL, ImGuiWindowFlags_None);

            // Display the chat log in a scrollable area
            ImGui::BeginChild("ChatArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
            {
                std::lock_guard<std::mutex> lock(chatMutex);
                for (const auto &message : chatMessages)
                {
                    ImGui::TextUnformatted(message.c_str());
                }
            }
            ImGui::SetScrollHereY(1.0f); // Scroll to bottom
            ImGui::EndChild();

            // Input box for typing new messages
            if (focusInput)
            {
                ImGui::SetKeyboardFocusHere(); // Set focus to the input box after message sent
                focusInput = false;            // Reset focus flag
            }
            // ImGui::SetKeyboardFocusHere();
            ImGui::InputText("##Input", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            // Send button or pressing Enter submits the message

            if (ImGui::Button("Send") || ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                if (strlen(inputBuffer) > 0)
                {
                    // Add the input text as a new chat message
                    AddMessage(inputBuffer);
                    sendMessage();
                    focusInput = true;
                }
            }

            ImGui::End();
        }

        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void handleSystemCallError(std::string errorMsg)
{
    std::cout << errorMsg << ", errno: " << errno << "\n";
    //exit(EXIT_FAILURE);
}

// creates a client socket
int createClientSocket(const std::string &serverIP, int serverPort)
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        handleSystemCallError("Failed to create socket");
    }

    // Set up the server address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;

    serverAddress.sin_port = htons(serverPort);

    // check IP validity by converting it to binary
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0)
    {
        handleSystemCallError("Invalid address or address not supported\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        handleSystemCallError("Error when connecting to server\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    return clientSocket;
}

void receiveMessages(int clientSocket)
{
    char buffer[10240];

    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            AddMessage("[Disconnected from server]");
            close(clientSocket);
            break;
        }

        buffer[bytesRead] = '\0';
        AddMessage(buffer);
    }
}

void AddMessage(const char *message)
{
    std::lock_guard<std::mutex> lock(chatMutex);
    chatMessages.push_back(message);
}

void connectToServer()
{
    int PortNumber = std::atoi(Port);
    std::string ipString = std::string(IP);

    clientSocket = createClientSocket(ipString, PortNumber);

    // Start background receive loop
    std::thread(receiveMessages, clientSocket).detach();
    isConnected = true;
}


void sendMessage()
{
    std::string userMessage;
    std::string userString = std::string(User);
    std::string passwordString = std::string(Password);

    std::string messageContent = std::string(inputBuffer);
    userMessage = userString + ": " + messageContent + "\n";

    send(clientSocket, userMessage.data(), userMessage.size(), 0);

    // Clear the input buffer
    inputBuffer[0] = '\0';
}