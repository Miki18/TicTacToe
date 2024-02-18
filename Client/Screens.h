#pragma once

void ShowLogo()
{
    ImGuiStyle& style = ImGui::GetStyle();

    texture = loadTexture("TicTacToe.bmp");
    ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
    ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
    ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
    style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);        //window borders have the same color as background so they will be invisible
    ImGui::Image((void*)(intptr_t)texture, ImVec2(400, 150));
    ImGui::End();
}