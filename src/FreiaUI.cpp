#include "FreiaUI.h"
#include "Validation.h"

int FreiaUI::theme = 2;

const ImVec4 FreiaUI::themes[] = {
    ImVec4(1.0f, 0.0f, 0.5f, 1.0f),    // pink
    ImVec4(1.00f, 0.43f, 0.71f, 1.00f),  // Soft Pink / Magenta
    ImVec4(1.0f, 0.67f, 0.0f, 1.0f),   // amber
    ImVec4(1.00f, 0.67f, 0.00f, 1.00f),  // Warm Amber / Gold
    ImVec4(0.0f, 1.0f, 0.25f, 1.0f),   // green
    ImVec4(0.22f, 0.89f, 0.44f, 1.00f),  // Fresh Green
    ImVec4(0.00f, 0.78f, 0.78f, 1.00f),  // DOS White – Cyan/Teal accent
    ImVec4(0.18f, 0.62f, 0.95f, 1.00f), // Calm Blue
    ImVec4(0.80f, 0.20f, 0.20f, 1.00f), // Deep Red
};
    const char* FreiaUI::themeNames[] = {
    "Pink",
    "Soft Pink",
    "Amber",
    "Warm Amber",
    "Green",
    "Fresh Green",
    "DOS White",
    "Calm Blue",
    "Deep Red",
};

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
    ImGui::PushStyleColor(ImGuiCol_Text, themes[theme]);

    renderMenuBar();
    renderConnectionPanel();
    renderChatPanel();
    renderUserList();    

    if (openOptions)
    {
        renderOptions();
    }

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

// void FreiaUI::renderConnectionPanel()
// {
//     ImGui::SetNextWindowPos(ImVec2(50, 50),  ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowSizeConstraints(
//     ImVec2(530, 230),           // min size
//     ImVec2(FLT_MAX, FLT_MAX));    // max size (unlimited)
//     ImGui::SetNextWindowSize(ImVec2(530, 280), ImGuiCond_FirstUseEver);

//     const float labelWidth = 220.0f;
//     const float inputWidth = 300.0f;
//     ImGui::Begin("Connection Data");
    
//     ImGui::Text("IP: ");
//     ImGui::SameLine(labelWidth);
//     ImGui::SetNextItemWidth(inputWidth);
//     ImGui::InputTextWithHint("##IP", "e.g. 192.168.1.100", IP, IM_ARRAYSIZE(IP));

//     ImGui::Text("Port: ");
//     ImGui::SameLine(labelWidth);
//     ImGui::SetNextItemWidth(inputWidth);
//     ImGui::InputTextWithHint("##PORT", "e.g. 8080", Port, IM_ARRAYSIZE(Port));

//     ImGui::Text("User Name: ");
//     ImGui::SameLine(labelWidth);
//     ImGui::SetNextItemWidth(inputWidth);
//     ImGui::InputTextWithHint("##USERNAME", "Your display name", User, IM_ARRAYSIZE(User));

//     ImGui::Text("Encryption Password: ");
//     ImGui::SameLine(labelWidth);
//     ImGui::SetNextItemWidth(inputWidth);
//     ImGui::InputTextWithHint("##ENCPASS", "Shared chat secret", ChatPassword, IM_ARRAYSIZE(ChatPassword), ImGuiInputTextFlags_Password);

//     ImGui::Text("Server Password:");
//     ImGui::SameLine(labelWidth);
//     ImGui::SetNextItemWidth(inputWidth);
//     ImGui::InputTextWithHint("##SERVERPASS", "Server access password", ServerPassword, IM_ARRAYSIZE(ServerPassword), ImGuiInputTextFlags_Password);

//     if (!client || !client->isConnectedToServer())
//     {
//         connectButton();
//     }
//     else
//     {
//         disconnectButton();
//     }

//     ImGui::End();
// }

void FreiaUI::renderConnectionPanel()
{
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(530, 230), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowSize(ImVec2(530, 320), ImGuiCond_FirstUseEver);  // taller for tabs + extra field

    ImGui::Begin("Connection Data");

    if (client && client->getIsConnected()) {
        disconnectButton();
        ImGui::End();
        return;
    }

    // Tabs
    if (ImGui::BeginTabBar("ConnectionMode", ImGuiTabBarFlags_NoTooltip)) {

        // Tab 1: Login with existing account
        if (ImGui::BeginTabItem("Login")) {
            ImGui::Text("Use an existing account");

            const float labelWidth = 220.0f;
            const float inputWidth = 300.0f;

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

            ImGui::Text("Account Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputTextWithHint("##ACCOUNTPASS", "Your account login password", AccountPassword, IM_ARRAYSIZE(AccountPassword), ImGuiInputTextFlags_Password);

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Connect with Account")) {
                if (validateLoginFields()) {
                    client = new ClientConnect();
                    if (client->configureWithAccount(IP, Port, User, ChatPassword, AccountPassword)) {  // new method you'll add
                        if (!client->connectToServer()) {
                            delete client;
                            client = nullptr;
                            openPopup("Connection failed. Server unreachable.");
                        }
                    } else {
                        delete client;
                        client = nullptr;
                        openPopup("Configuration rejected.");
                    }
                }
            }

            ImGui::EndTabItem();
        }

        // Tab 2: Create new account
        if (ImGui::BeginTabItem("Create Account")) {
            ImGui::Text("Register a new account");

            const float labelWidth = 220.0f;
            const float inputWidth = 300.0f;

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
            ImGui::InputTextWithHint("##USERNAME", "Choose a username", User, IM_ARRAYSIZE(User));

            ImGui::Text("Encryption Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputTextWithHint("##ENCPASS", "Shared chat secret", ChatPassword, IM_ARRAYSIZE(ChatPassword), ImGuiInputTextFlags_Password);

            ImGui::Text("Account Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputTextWithHint("##ACCOUNTPASS", "Choose account password", AccountPassword, IM_ARRAYSIZE(AccountPassword), ImGuiInputTextFlags_Password);

            ImGui::Text("Confirm Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputTextWithHint("##CONFIRMPASS", "Repeat account password", ConfirmAccountPassword, IM_ARRAYSIZE(ConfirmAccountPassword), ImGuiInputTextFlags_Password);

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Create & Connect")) {
                if (validateCreateFields()) {
                    client = new ClientConnect();
                    if (client->configureForCreate(IP, Port, User, ChatPassword, AccountPassword)) {  // new method
                        if (!client->connectToServer()) {  // server will handle creation
                            delete client;
                            client = nullptr;
                            openPopup("Connection/creation failed.");
                        }
                    } else {
                        delete client;
                        client = nullptr;
                        openPopup("Invalid fields for creation.");
                    }
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
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
        ImGui::PushTextWrapPos(0.0f);
        for (const auto& msg : messages)
        {
            ImGui::TextUnformatted(msg.c_str());
        }
        ImGui::PopTextWrapPos();

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
            else
            {
                
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
    std::memset(ServerPassword, 0, sizeof(ServerPassword));
}

void FreiaUI::renderMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, themes[theme]);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Application"))
        {
            if (ImGui::MenuItem("Options")) openOptions = true;
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

void FreiaUI::renderOptions()
{
    if (!openOptions) return;
    ImGui::SetNextWindowPos(ImVec2(50, 650), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 180), ImGuiCond_FirstUseEver);

    ImGui::Begin("Options", &openOptions, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Color Theme");

    // Dropdown / combo box
    if (ImGui::BeginCombo("##ThemeCombo", themeNames[theme]))
    {
        for (int i = 0; i < NUM_THEMES; ++i)
        {
            bool isSelected = (theme == i);
            if (ImGui::Selectable(themeNames[i], isSelected))
            {
                theme = i;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::ColorButton("##ThemePreview", themes[theme], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);

    ImGui::Separator();

    // Future options can go here (font size, language, etc.)
    ImGui::TextDisabled("(More settings coming soon)");

    if (ImGui::Button("Close"))
    {
        openOptions = false;
    }

    ImGui::End();
}

void FreiaUI::renderUserList()
{
    if (client && client->getIsConnected())
    {
        const auto& users = client->getOnlineUsers();

        ImGui::BeginChild("Online Users", ImVec2(180, 0), true);
        ImGui::Text("Online (%zu)", users.size());
        ImGui::Separator();

        for (const auto& name : users)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "%s", name.c_str());
        }
        ImGui::EndChild();
    } else
    {
        ImGui::TextDisabled("Not connected");
    }
}

bool FreiaUI::validateLoginFields() {
    if (!Validation::isValidIP(IP))            { openPopup("Invalid IP."); return false; }
    if (!Validation::isValidPort(Port))        { openPopup("Invalid Port."); return false; }
    if (!Validation::isValidUser(User))        { openPopup("Invalid username."); return false; }
    if (!Validation::isValidPassword(ChatPassword)) { openPopup("Invalid chat password."); return false; }
    if (!Validation::isValidPassword(AccountPassword)) { openPopup("Invalid account password."); return false; }
    return true;
}

bool FreiaUI::validateCreateFields() {
    if (!validateLoginFields()) return false;  // reuse common checks
    if (strcmp(AccountPassword, ConfirmAccountPassword) != 0) {
        openPopup("Account passwords do not match.");
        return false;
    }
    return true;
}