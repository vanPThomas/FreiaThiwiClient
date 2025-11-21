#pragma once
#include "ClientConnect.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

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
    void connectButton();
    void disconnectButton();
    void clearInputFields();
    void renderMenuBar();
    void showPopup();
    void openPopup(const std::string& message);

    static const int bufferSize = 1024;

    char inputBuffer[bufferSize] = "";
    char IP[20] = "";
    char Port[10] = "";
    char User[50] = "";
    char ChatPassword[1000] = "";

    bool focusInput = false;
    bool quitRequested = false;

    ClientConnect* client = nullptr;
    ImGuiIO* io = nullptr;
    ImFont* customFont = nullptr;
    GLFWwindow* window = nullptr;

    //Popup Variables
    std::string popupMessage;
    bool popupOpen = false;
};
