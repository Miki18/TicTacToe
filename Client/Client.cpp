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
    IP_Insert
};

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

        if (screen == Single_Multi_Choose)
        {
            //TicTacToe LOGO
            texture = loadTexture("TicTacToe.bmp");
            ImGui::SetNextWindowPos(ImVec2(740, 110), NULL);             //window position
            ImGui::SetNextWindowSize(ImVec2(408,180), NULL);
            ImGui::Begin("TicTacToe", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);   //create ImGui window for example
            ImGuiStyle& style = ImGui::GetStyle();                      // ImGui background color (white)
            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(400,150));
            ImGui::End();

            //Single Player Button

            //Multi Player Button

            //Exit Button
            ImGui::SetNextWindowPos(ImVec2(740, 820), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 150), NULL);
            ImGui::Begin("Exit", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
            style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            if (ImGui::Button("Exit", ImVec2(400, 150)))
            {
                exit(0);
            }
            ImGui::End();
        }

        //Rendering ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //GLFW swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));
}