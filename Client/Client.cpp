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
#include <random>
#include "LoadTexture.h"

std::mutex wait_for_player;
std::condition_variable cv;

//enum - client has can be in different menus like main menu or multiplayer game menu.
// The enum help us to control where the client is at this time and what should see right now.
enum Screen {
    Single_Multi_Choose,
    Single_Menu,
    IP_Insert,
    GameScreen,
    Won,
    Lost,
    Draw
};

Screen screen = Single_Multi_Choose;

//table for game controlling; ? - empty place; X - x in that place; O - o in that place
char GameBoardStatus[3][3] = { {'?','?','?'} 
                              ,{'?','?','?'}
                              ,{'?','?','?'} };

int BoardCheck()    //return 0 means no winner, 1 - X wins, 2 - O wins
{
    for (int i = 0; i < 3; i++)
    {
        if (GameBoardStatus[i][0] == 'X' and GameBoardStatus[i][1] == 'X' and GameBoardStatus[i][2] == 'X')  //check rows
        {
            return 1;
        }

        if (GameBoardStatus[i][0] == 'O' and GameBoardStatus[i][1] == 'O' and GameBoardStatus[i][2] == 'O')
        {
            return 2;
        }

        if (GameBoardStatus[0][i] == 'X' and GameBoardStatus[1][i] == 'X' and GameBoardStatus[2][i] == 'X')  //check colums
        {
            return 1;
        }

        if (GameBoardStatus[0][i] == 'O' and GameBoardStatus[1][i] == 'O' and GameBoardStatus[2][i] == 'O')
        {
            return 2;
        }
    }
    if ((GameBoardStatus[0][0] == 'X' and GameBoardStatus[1][1] == 'X' and GameBoardStatus[1][2] == 'X') or (GameBoardStatus[0][2] == 'X' and GameBoardStatus[1][1] == 'X' and GameBoardStatus[2][0] == 'X'))  //check diagonal
    {
        return 1;
    }
    else if ((GameBoardStatus[0][0] == 'O' and GameBoardStatus[1][1] == 'O' and GameBoardStatus[1][2] == 'O') or (GameBoardStatus[0][2] == 'O' and GameBoardStatus[1][1] == 'O' and GameBoardStatus[2][0] == 'O'))
    {
        return 2;
    }
    else
    {
        return 0;   //there is no winner
    }
}

void GameThread(char Opponent_level, char& Mark)
{   
    bool YourTurn = 0;
    if (Opponent_level == 'm')
    {
        //multiplayer game
    }
    else    //single player fragment
    {
        srand(time(NULL));    //in single player game we use random to detect if you go first or second. If 0 u go second if 1 u go first
        if (rand() % 2 == 0)
        {
            YourTurn = 0;
        }
        else
        {
            YourTurn = 1;
        }

        for(int x = 0; x<8; x++)   //we do it 9 times, because we have 9 places to fill
        {
            if (YourTurn == 1)
            {
                //wait here for player
                std::unique_lock<std::mutex> lck(wait_for_player);
                cv.wait(lck);

                if (Mark == 'X')   //change to opposite
                {
                    Mark = 'O';
                }
                else
                {
                    Mark = 'X';
                }

                YourTurn = 0;    //enemy move
            }
            else     //opponent move
            {
                //if hard check if u can win or block
                do
                {
                    int i = rand() % 3;
                    int a = rand() % 3;
                    if (GameBoardStatus[i][a] == '?')
                    {
                        GameBoardStatus[i][a] = Mark;
                        break;
                    }
                } while (true);

                if (Mark == 'X')   //change to opposite
                {
                    Mark = 'O';
                }
                else
                {
                    Mark = 'X';
                }

                YourTurn = 1;    //your move
            }
            
            int result = BoardCheck(); //we check if anyone win

            if (result == 1 or result == 2)
            {
                for (int i = 0; i < 3; i++)
                {
                    for (int a = 0; a < 3; a++)
                    {
                        GameBoardStatus[i][a] = '?';
                    }
                }
                screen = Single_Multi_Choose;
                break;
            }
        }
    }
}

int main()
{
    char Mark = 'X';
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
    //Screen screen = Single_Multi_Choose;  - goes to the global variable

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
                screen = GameScreen;
                std::thread(GameThread, 'e', std::ref(Mark)).detach();      //Here we starts thread for single player game and detach.
                //e means easy opponent, h means hard opponent, m means multiplayer game
            }

            //hard opponent
            ImGui::SetNextWindowPos(ImVec2(740, 500), NULL);
            ImGui::SetNextWindowSize(ImVec2(400, 180), NULL);
            ImGui::Begin("HardOpponent", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Hard Opponent", ImVec2(400, 150)))
            {
                screen = GameScreen;
                std::thread(GameThread, 'h', std::ref(Mark)).detach();      //Here we starts thread for single player game and join.
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
                screen = Single_Multi_Choose;
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
        else if (screen == GameScreen)
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

            texture = loadTexture("Board.bmp");
            ImGui::SetNextWindowPos(ImVec2(610, 300), NULL);
            ImGui::SetNextWindowSize(ImVec2(700, 700), NULL);
            ImGui::Begin("Board", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(700, 700));
            ImGui::End();

            //we need to fill all 9 field
            for(int i = 0; i < 3; i++)
            {
                for(int a = 0; a < 3; a++)
                {
                    int ID = 3 * i + a;   //calculating ID value
                    if (GameBoardStatus[a][i] == '?')  //if the field is empty than we don't load any texture, but only put a button there
                    {
                        ImGui::SetNextWindowPos(ImVec2(620 + (i * 250), 320 + (a * 250)), NULL);
                        ImGui::SetNextWindowSize(ImVec2(200, 200), NULL);
                        ImGui::Begin(std::to_string(ID).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
                        if (ImGui::Button("Button", ImVec2(200, 200)))
                        {
                            if (Mark == 'X')
                            {
                                GameBoardStatus[a][i] = 'X';
                            }
                            else
                            {
                                GameBoardStatus[a][i] = 'O';
                            }

                            cv.notify_one();
                        }
                        ImGui::End();
                    }
                    else
                    {
                        if (GameBoardStatus[a][i] == 'X')    //check if there is an X or O and load its texture
                        {
                            texture = loadTexture("x.bmp");
                        }
                        else if (GameBoardStatus[a][i] == 'O')
                        {
                            texture = loadTexture("o.bmp");
                        }
                        
                        //In ImGui every window have to have different name (this is like their ID). With that (3*i+a) windows will have names like 1, 2, 3 etc
                        ImGui::SetNextWindowPos(ImVec2(620 + (i * 250), 320 + (a * 250)), NULL);
                        ImGui::SetNextWindowSize(ImVec2(200, 200), NULL);
                        ImGui::Begin(std::to_string(ID).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
                        ImGui::Image((void*)(intptr_t)texture, ImVec2(170, 170));
                        ImGui::End();
                    }
                }
            }
        }

        //Rendering ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //GLFW swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));
}