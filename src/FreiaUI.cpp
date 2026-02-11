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

void FreiaUI::renderConnectionPanel()
{
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(530, 230), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowSize(ImVec2(530, 320), ImGuiCond_FirstUseEver);

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

            labeledTextInput("IP:",               IP,                "e.g. 192.168.1.100");
            labeledTextInput("Port:",             Port,              "e.g. 8080");
            labeledTextInput("User Name:",        User,              "Your display name");
            labeledPasswordInput("Encryption Password:", ChatPassword, "Shared chat secret");
            labeledPasswordInput("Server Password:",     ServerPassword, "Shared server secret");
            labeledPasswordInput("Account Password:",    AccountPassword, "Your account login password");

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Connect with Account")) {
                if (validateLoginFields()) {
                    tryConnectAndConfigure(false);
                }
            }

            ImGui::EndTabItem();
        }

        // Tab 2: Create new account
        if (ImGui::BeginTabItem("Create Account")) {
            ImGui::Text("Register a new account");

            labeledTextInput("IP:",               IP,                "e.g. 192.168.1.100");
            labeledTextInput("Port:",             Port,              "e.g. 8080");
            labeledTextInput("User Name:",        User,              "Your display name");
            labeledPasswordInput("Encryption Password:", ChatPassword, "Shared chat secret");
            labeledPasswordInput("Server Password:",     ServerPassword, "Shared server secret");
            labeledPasswordInput("Account Password:",    AccountPassword, "Your account login password");

            ImGui::Text("Confirm Password: ");
            ImGui::SameLine(labelWidth);
            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputTextWithHint("##CONFIRMPASS", "Repeat account password", &ConfirmAccountPassword, ImGuiInputTextFlags_Password);

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Create & Connect")) {
                if (validateCreateFields()) {
                    tryConnectAndConfigure(true);
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
    IP.clear();
    Port.clear();
    User.clear();
    ChatPassword.clear();
    ServerPassword.clear();
    AccountPassword.clear();
    ConfirmAccountPassword.clear();
    std::memset(inputBuffer, 0, sizeof(inputBuffer));
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

bool FreiaUI::validateLoginFields()
{
    if (!Validation::isValidIP(IP))            { openPopup("Invalid IP."); return false; }
    if (!Validation::isValidPort(Port))        { openPopup("Invalid Port."); return false; }
    if (!Validation::isValidUser(User))        { openPopup("Invalid username."); return false; }
    if (!Validation::isValidPassword(ChatPassword)) { openPopup("Invalid chat password."); return false; }
    if (!Validation::isValidPassword(AccountPassword)) { openPopup("Invalid account password."); return false; }
    return true;
}

bool FreiaUI::validateCreateFields()
{
    if (!validateLoginFields()) return false;

    if (AccountPassword != ConfirmAccountPassword) {
        openPopup("Account passwords do not match.");
        return false;
    }

    return true;
}

void FreiaUI::labeledPasswordInput(const char* label, std::string& value, const char* hint) {
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint(("##" + std::string(label)).c_str(), hint, &value, ImGuiInputTextFlags_Password);
}

void FreiaUI::labeledTextInput(const char* label, std::string& value, const char* hint) {
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputTextWithHint(("##" + std::string(label)).c_str(), hint, &value);
}

bool FreiaUI::tryConnectAndConfigure(bool isCreation) {
    client = new ClientConnect();
    if (!client) return false;

    bool success = client->configureWithAccount(
        IP, Port, User,
        ChatPassword, ServerPassword, AccountPassword,
        isCreation
    );

    if (!success) {
        openPopup("Configuration rejected.");
        delete client;
        client = nullptr;
        return false;
    }

    if (!client->connectToServer()) {
        openPopup(isCreation ? "Connection/creation failed." : "Connection failed. Server unreachable.");
        delete client;
        client = nullptr;
        return false;
    }

    return true;
}