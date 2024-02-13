#define WIN32_LEAN_AND_MEAN   //EXAMPLE CODE TO CHECK IF LIBRARIES WORK (client)

#include <math.h>
#include <GL/glew.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <locale.h>
#include <GLFW/glfw3.h>
#include <thread>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include "LoadTexture.h"

//enum - client has can be in different menus like main menu or multiplayer game menu.
// The enum help us to control where the client is at this time and what should see right now.
enum Screen {
    Single_Multi_Choose,
    Single_Menu,
    IP_Insert,
    Game,
    Won,
    Lost
};

//table for game controlling; ? - empty place; X - x in that place; O - o in that place
char GameBoardStatus[3][3] = { {'?','?','?'} ,{'?', '?', '?'},{'?', '?', '?'} };

void GameThread(char Opponent_level)
{
    if (Opponent_level == 'm')
    {
        //multiplayer game
    }
    else    //single player fragment
    {
        //single player
    }
}

int main()
{
    GLFWwindow* window{};   //GLFW window

    //GLFW initialization
    if (!glfwInit()) {
        fprintf(stderr, "Can't run GLFW.\n");
        exit(EXIT_FAILURE);
    }

    //create window (size and title)
    window = glfwCreateWindow(1920, 1080, "TicTacToe", NULL, NULL);
    glfwSetWindowSizeLimits(window, 1920, 1080, 1920, 1080);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwMakeContextCurrent(window);

    //Glew initialization
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Cannot initialize GLEW.\n");
        exit(EXIT_FAILURE);
    }

    //ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Enum Screen
    Screen screen = Single_Multi_Choose;

    //main loop
    do
    {
        //Set background color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //ImGui updating new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiStyle& style = ImGui::GetStyle();

        //The first screen - player can choose between singleplayer and multiplayer game
        if (screen == Single_Multi_Choose)
        {
            //TicTacToe LOGO
            texture = loadTexture("TicTacToe.bmp");
            ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
            ImGui::SetNextWindowSize(ImVec2(408,180), NULL);
            ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
            style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);        //window borders have the same color as background so they will be invisible
            ImGui::Image((void*)(intptr_t)texture, ImVec2(400,150));
            ImGui::End();

            //Single Player Button
            ImGui::SetNextWindowPos(ImVec2(740, 280), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("Singleplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Singleplayer", ImVec2(400, 150)))
            {
                screen = Single_Menu;
            }

            //Multi Player Button
            ImGui::SetNextWindowPos(ImVec2(740, 500), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("Multiplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Multiplayer", ImVec2(400, 150)))
            {
                screen = IP_Insert;
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

        //player chooses easy or hard opponent
        else if (screen == Single_Menu)
        {
            //TicTacToe LOGO
            texture = loadTexture("TicTacToe.bmp");
            ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
            ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
            ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
            style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(400, 150));
            ImGui::End();

            //easy opponent
            ImGui::SetNextWindowPos(ImVec2(740, 280), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("EasyOpponent", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Easy Opponent", ImVec2(400, 150)))
            {
                screen = Game;
                std::thread Game(GameThread, 'e');      //Here we starts thread for single player game and join.
                Game.join();
                //e means easy opponent, h means hard opponent, m means multiplayer game
            }

            //hard opponent
            ImGui::SetNextWindowPos(ImVec2(740, 500), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("HardOpponent", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Hard Opponent", ImVec2(400, 150)))
            {
                screen = Game;
                std::thread Game(GameThread, 'h');      //Here we starts thread for single player game and join.
                Game.join();
                //e means easy opponent, h means hard opponent, m means multiplayer game
            }

            //back
            ImGui::SetNextWindowPos(ImVec2(740, 700), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("Back", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            if (ImGui::Button("Back", ImVec2(400, 150)))
            {
                screen = Game;
            }
            ImGui::End();
        }

        //if player chooses multiplayer game we need him to insert an IP adres
        else if (screen == IP_Insert)
        {
            //TicTacToe LOGO
            texture = loadTexture("TicTacToe.bmp");
            ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
            ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
            ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
            style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(400, 150));
            ImGui::End();

            //insert IP
            ImGui::SetNextWindowPos(ImVec2(740, 400), NULL);
            ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);

            //back
            ImGui::SetNextWindowPos(ImVec2(740, 740), NULL);
            ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
            ImGui::Begin("Back", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            if (ImGui::Button("Back", ImVec2(400, 150)))
            {
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        //if player plays he should see a board
        else if (screen == Game)
        {
            //TicTacToe LOGO
            texture = loadTexture("TicTacToe.bmp");
            ImGui::SetNextWindowPos(ImVec2(740, 60), NULL);             //window position
            ImGui::SetNextWindowSize(ImVec2(408, 180), NULL);
            ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);   //create ImGui window for example                     
            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);       // ImGui background color (white)
            style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(400, 150));
            ImGui::End();
        }

        //Rendering ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (screen == Game)
        {
            //odpal w¹tek z gr¹
        }

        //GLFW swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));
}