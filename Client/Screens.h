#pragma once

void ImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Single_Multi_Choose()
{
    ImGuiStyle& style = ImGui::GetStyle();
    enum Screen;
    extern Screen screen;

    ImGuiFrame();
    
    //TicTacToe LOGO
    texture = loadTexture("TicTacToe.bmp");
    ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
    ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
    ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
    style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);        //window borders have the same color as background so they will be invisible
    ImGui::Image((void*)(intptr_t)texture, ImVec2(400, 150));
    ImGui::End();

    //Single Player Button
    ImGui::SetNextWindowPos(ImVec2(740, 280), NULL);
    ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
    ImGui::Begin("Singleplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    if (ImGui::Button("Singleplayer", ImVec2(400, 150)))
    {
        //screen = Single_Menu;
    }

    //Multi Player Button
    ImGui::SetNextWindowPos(ImVec2(740, 500), NULL);
    ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
    ImGui::Begin("Multiplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    if (ImGui::Button("Multiplayer", ImVec2(400, 150)))
    {
        //screen = IP_Insert;
    }

    //Exit Button
    ImGui::SetNextWindowPos(ImVec2(740, 700), NULL);
    ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
    ImGui::Begin("Exit", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    if (ImGui::Button("Exit", ImVec2(400, 150)))
    {
        exit(0);
    }
    ImGui::End();
}