#pragma once
#include "ClientConnect.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <cctype>
#include "imgui_stdlib.h"

class FreiaUI
{
public:
    FreiaUI();
    ~FreiaUI();

    bool render();
    ClientConnect* getClient() const { return client; }
    void setClient(ClientConnect* c) {client = c;}

private:
    void renderConnectionPanel();
    void renderChatPanel();
    void disconnectButton();
    void clearInputFields();
    void renderMenuBar();
    void chatRoomListRender();
    void createRoomRender();

    
    void showPopup();
    void openPopup(const std::string& message);
    void renderOptions();
    void renderUserList();
    bool validateLoginFields();
    bool validateCreateFields();

    void labeledPasswordInput(const char* label, std::string& value, const char* hint);
    void labeledTextInput(const char* label, std::string& value, const char* hint);
    bool tryConnectAndConfigure(bool isCreation);


    static const int bufferSize = 1024;

    char inputBuffer[bufferSize] = "";

    std::string IP;
    std::string Port;
    std::string User;
    std::string ChatPassword;
    std::string ServerPassword;
    std::string AccountPassword;
    std::string ConfirmAccountPassword;


    bool focusInput = false;
    bool quitRequested = false;
    bool openOptions = false;

    ClientConnect* client = nullptr;
    ImGuiIO* io = nullptr;
    ImFont* customFont = nullptr;
    GLFWwindow* window = nullptr;

    //Popup Variables
    std::string popupMessage;
    bool popupOpen = false;

    // UI color
    static int theme;                          // current theme index
    static const ImVec4 themes[];              // array of accent colors
    static const char* themeNames[];           // human-readable names for combo
    static constexpr int NUM_THEMES = 9;       // update when adding more

    std::unordered_set<std::string> onlineUsers;
    bool showUserList = true;

    const float labelWidth = 220.0f;
    const float inputWidth = 300.0f;

};
