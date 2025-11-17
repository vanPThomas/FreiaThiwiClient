#include "FreiaUI.h"
#include <iostream>

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

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont(customFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.5f, 1.0f));

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
    const float labelWidth = 220.0f;
    ImGui::Begin("Connection Data");

    ImGui::Text("IP: ");
    ImGui::SameLine(labelWidth);
    ImGui::InputText("##IP", IP, IM_ARRAYSIZE(IP));

    ImGui::Text("Port: ");
    ImGui::SameLine(labelWidth);
    ImGui::InputText("##PORT", Port, IM_ARRAYSIZE(Port));

    ImGui::Text("User Name: ");
    ImGui::SameLine(labelWidth);
    ImGui::InputText("##USERNAME", User, IM_ARRAYSIZE(User));

    ImGui::Text("Encryption Password: ");
    ImGui::SameLine(labelWidth);
    ImGui::InputText("##ENCPASS", Password, IM_ARRAYSIZE(Password));

    if (!client || !client->isConnectedToServer())
    {
        if (ImGui::Button("Connect"))
        {
            client = new ClientConnect();
            client->configure(IP, Port, User, Password);
            if (!client->connectToServer())
            {
                delete client;
                client = nullptr;
                // TODO: pup up
            }
        }
    }

    ImGui::End();
}

void FreiaUI::renderChatPanel()
{
    ImGui::Begin("Chat Window");

    ImGui::BeginChild("ChatArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
    if (client)
    {
        const auto& messages = client->getMessages();
        for (const auto& msg : messages)
            ImGui::TextUnformatted(msg.c_str());
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

void FreiaUI::cleanupUI()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
