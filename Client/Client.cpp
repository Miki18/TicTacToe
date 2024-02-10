#define WIN32_LEAN_AND_MEAN   //EXAMPLE CODE TO CHECK IF LIBRARIES WORK

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

int main()
{
    GLFWwindow* window{};   //GLFW window

    if (!glfwInit()) { //GLFW initialization
        fprintf(stderr, "Can't run GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(1280, 720, "TicTacToe", NULL, NULL);  //create window (size and title)
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) { //Glew initialization
        fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW.\n");
        exit(EXIT_FAILURE);
    }

    IMGUI_CHECKVERSION();    //ImGui initialization
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    do    //main loop
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  //Set background color

        ImGui_ImplOpenGL3_NewFrame();   //ImGui updating new frame
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("TicTacToe");   //create ImGui window for example
        ImGui::End();

        ImGui::Render();    //Rendering ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);   //GLFW swap buffers and poll events
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));
}