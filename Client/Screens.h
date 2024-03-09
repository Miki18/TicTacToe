#pragma once

//enum - client has can be in different menus like main menu or multiplayer game menu.
// The enum help us to control where the client is at this time and what should see right now.
enum Screen {
    Single_Multi_Choose,
    Single_Menu,
    IP_Insert,
    GameScreen,
    Result,
    Connect,
    Connect_not,
    LoginOrCreateNewAccount,
    Login,
    CreateNewAccount,
    LoginRegiserFailed,
    MultiplayerMenu,
    DeleteAccount,
    Profile,
    Stats,
    ChangePassword,
    WaitForPlayers
};

GLuint texture;

GLuint loadTexture(const char* filename) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load a picture
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 0;
    }

    //Load a BMP
    char header[54];
    file.read(header, 54);
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];

    //Load pixels' data
    int size = 3 * width * height;
    std::vector<char> pixels(size);
    file.read(pixels.data(), size);

    //Create a texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    return texture;
}

void ShowLogo()
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    texture = loadTexture("TicTacToe.bmp");
    ImGui::SetNextWindowSize(ImVec2(mode->width/4.5, mode->height/6), NULL);                //We need to base on sceen width and height, because computers have different screen resolution
    ImGui::SetNextWindowPos(ImVec2(mode->width*7.6/20, mode->height*1/24), NULL);             //window position
    ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
    style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);        //window borders have the same color as background so they will be invisible
    ImGui::Image((void*)(intptr_t)texture, ImVec2(mode->width / 5, mode->height / 6.5));
    ImGui::End();
}

void SetBackButton()
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 19 / 24), NULL);
    ImGui::Begin("Back", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
}

void SingleMultiChoose(Screen& screen)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    //TicTacToe LOGO
    ShowLogo();

    //Single Player Button
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
    ImGui::Begin("Singleplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);          //set button color
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    if (ImGui::Button("Singleplayer", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = Single_Menu;
    }

    //Multi Player Button
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
    ImGui::Begin("Multiplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    if (ImGui::Button("Multiplayer", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = IP_Insert;
    }

    //Exit Button
    SetBackButton();        //exit button has the same settings as back button & every back button has the same settings
    if (ImGui::Button("Exit", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        exit(0);
    }
    ImGui::End();
}

void IPinsert(Screen& screen, char (&IP)[16])
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    //TicTacToe LOGO
    ShowLogo();

    //insert IP
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
    ImGui::Begin("Insert_window", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    ImGui::Text("Insert IP:");
    if (ImGui::InputText("", IP, sizeof(IP), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        screen = Connect;
    }
    ImGui::End();

    //back
    SetBackButton();
    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = Single_Multi_Choose;
    }
    ImGui::End();
}

void ResultScreen(Screen& screen, std::string& ResultValue, bool IsConnected)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    ShowLogo();

    //Info about results
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
    ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 4 / 10);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
    ImGui::Text(ResultValue.c_str());
    ImGui::End();

    //back
    SetBackButton();
    if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        if (IsConnected == true)   //if we are connected to the server go to server menu (aka MultiplayerMenu), if not we go to Single_Multi_Choose (aka starter menu).
        {
            screen = MultiplayerMenu;
        }
        else
        {
            screen = Single_Multi_Choose;
        }
    }
    ImGui::End();
}

void ConnectNot(Screen& screen)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    ShowLogo();

    ImGui::SetNextWindowSize(ImVec2(mode->width / 2, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 3 / 20, mode->height * 9 / 24), NULL);
    ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 3 / 10);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
    ImGui::Text("Unable to connect to server!");
    ImGui::End();

    SetBackButton();
    if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = Single_Multi_Choose;
    }
    ImGui::End();
}

void LoginRegisterFail(Screen& screen)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    ShowLogo();

    ImGui::SetNextWindowSize(ImVec2(mode->width / 2, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 3 / 20, mode->height * 9 / 24), NULL);
    ImGui::Begin("LoginOrRegisterFailed", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 3 / 10);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
    ImGui::Text("Login or register failed!");
    ImGui::Text("Try change your nick or check password!");
    ImGui::End();

    SetBackButton();
    if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = LoginOrCreateNewAccount;
    }
    ImGui::End();
}

void ProfileScreen(Screen& screen)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

    ShowLogo();

    //Stats
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    if (ImGui::Button("Stats", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = Stats;
    }

    //changePassword
    ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
    ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
    ImGui::Begin("ChangePassword", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    if (ImGui::Button("ChangePassword", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = ChangePassword;
    }

    SetBackButton();
    if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
    {
        screen = MultiplayerMenu;
    }
    ImGui::End();
}