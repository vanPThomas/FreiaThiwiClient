#include "FreiaUI.h"
#include "Validation.h"
#include <iostream>
#include <cstring>
#include <cctype>

FreiaUI::FreiaUI()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return;
    }

    window = glfwCreateWindow(1280, 720, "Freia Thiwi", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    customFont = io->Fonts->AddFontFromFileTTF("fonts/Px437_IBM_VGA_8x14.ttf", 18.0f);
    if (!customFont)
    {
        std::cerr << "Failed to load font\n";
    }
    ImGui::GetIO().FontGlobalScale = 1.0f;  // 100% scaling
}

FreiaUI::~FreiaUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool FreiaUI::render()
{
    if (!window || glfwWindowShouldClose(window))
        return false;
    if (quitRequested)
    return false;

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    showPopup();

    ImGui::PushFont(customFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.5f, 1.0f));

    renderMenuBar();
    renderConnectionPanel();
    renderChatPanel();

    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    return true;
}

void FreiaUI::renderConnectionPanel()
{
    ImGui::SetNextWindowPos(ImVec2(50, 50),  ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(
    ImVec2(530, 230),           // min size
    ImVec2(FLT_MAX, FLT_MAX));    // max size (unlimited)
    ImGui::SetNextWindowSize(ImVec2(530, 280), ImGuiCond_FirstUseEver);

    const float labelWidth = 220.0f;
    const float inputWidth = 300.0f;
    ImGui::Begin("Connection Data");

    ImGui::Text("IP: ");
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint("##IP", "e.g. 192.168.1.100", IP, IM_ARRAYSIZE(IP));

    ImGui::Text("Port: ");
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint("##PORT", "e.g. 8080", Port, IM_ARRAYSIZE(Port));

    ImGui::Text("User Name: ");
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint("##USERNAME", "Your display name", User, IM_ARRAYSIZE(User));

    ImGui::Text("Encryption Password: ");
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint("##ENCPASS", "Shared chat secret", ChatPassword, IM_ARRAYSIZE(ChatPassword), ImGuiInputTextFlags_Password);

    ImGui::Text("Server Password:");
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint("##SERVERPASS", "Server access password", ServerPassword, IM_ARRAYSIZE(ServerPassword), ImGuiInputTextFlags_Password);

    if (!client || !client->isConnectedToServer())
    {
        connectButton();
    }
    else
    {
        disconnectButton();
    }

    ImGui::End();
}

void FreiaUI::renderChatPanel()
{
    ImGui::SetNextWindowPos(ImVec2(50, 350), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

    ImGui::Begin("Chat Window");

    ImGui::BeginChild("ChatArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
    if (client)
    {
        const auto& messages = client->getMessages();
        for (const auto& msg : messages)
            ImGui::TextUnformatted(msg.c_str());

        // Auto-scroll only if user is already at bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    if (focusInput)
    {
        ImGui::SetKeyboardFocusHere();
        focusInput = false;
    }

    ImGui::InputText("##Input", inputBuffer, IM_ARRAYSIZE(inputBuffer));
    ImGui::SameLine();

    if (ImGui::Button("Send") || ImGui::IsKeyPressed(ImGuiKey_Enter))
    {
        if (client && strlen(inputBuffer) > 0)
        {
            client->sendMessage(inputBuffer);
            inputBuffer[0] = '\0';
            focusInput = true;
        }
    }

    ImGui::End();
}

void FreiaUI::connectButton()
{
    if (ImGui::Button("Connect"))
    {
        // Basic UI validation before touching networking
        if (!Validation::isValidIP(IP))
        {
            openPopup("Invalid IP address.");
            return;
        }

        if (!Validation::isValidPort(Port))
        {
            openPopup("Invalid PORT.");
            return;
        }

        if(!Validation::isValidUser(User))
        {
            openPopup("Username is not valid.");
            return;
        }
        if(!Validation::isValidPassword(ChatPassword))
        {
            openPopup("Chat password is invalid.");
            return;
        }
        if(!Validation::isValidPassword(ServerPassword))
        {
            openPopup("Server password is invalid.");
            return;
        }

        // Create new client
        // if (client)
        // {
        //     delete client;
        //     client = nullptr;
        // }
        client = new ClientConnect();
        
        // Network-side validation
        if (client->configure(IP, Port, User, ChatPassword, ServerPassword))
        {
            if (!client->connectToServer())
            {
                delete client;
                client = nullptr;
                openPopup("Connection failed. Server unreachable.");
            }
        }
        else
        {
            delete client;
            client = nullptr;
            openPopup("Configuration rejected.");
        }
    }
}


void FreiaUI::disconnectButton()
{
    if (ImGui::Button("Disconnect"))
    {
        if(client)
        {
            client->disconnect();
            delete client;
            client = nullptr;
            clearInputFields();
        }
    }
}

void FreiaUI::clearInputFields()
{
    std::memset(IP, 0, sizeof(IP));
    std::memset(Port, 0, sizeof(Port));
    std::memset(User, 0, sizeof(User));
    std::memset(ChatPassword, 0, sizeof(ChatPassword));
    std::memset(inputBuffer, 0, sizeof(inputBuffer));
}

void FreiaUI::renderMenuBar()
{
    //ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.15f, 0.05f, 0.07f, 1.0f)); // darker/pinkish
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0.7, 1));

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Application"))
        {
            if (ImGui::MenuItem("Exit")) quitRequested = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void FreiaUI::openPopup(const std::string& message)
{
    popupMessage = message;
    popupOpen = true;
}

void FreiaUI::showPopup()
{
    if (popupOpen)
        ImGui::OpenPopup("Error");

    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("%s", popupMessage.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            popupOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
