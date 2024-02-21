#define WIN32_LEAN_AND_MEAN       //this is necessary

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
#include "Screens.h"
#include "GameMechanics.h"

std::mutex wait_for_player;
std::condition_variable cv;

std::mutex IsPlayersTurn;
std::condition_variable PT;

//enum - client has can be in different menus like main menu or multiplayer game menu.
// The enum help us to control where the client is at this time and what should see right now.
enum Screen {
    Single_Multi_Choose,
    Single_Menu,
    IP_Insert,
    GameScreen,
    Result
};

//table for game controlling; ? - empty place; X - x in that place; O - o in that place
char GameBoardStatus[3][3] = { {'?','?','?'} 
                              ,{'?','?','?'}
                              ,{'?','?','?'} };

void TimeControl(int& TimeLimit, bool& YourTurn, Screen& screen)   //time control controlling timer that player need to do the move. In single player this is optional, but in multiplayer it is necessary
{
    TimeLimit = 15;  //Time limit is 15 seconds
    for (TimeLimit; TimeLimit > 0; TimeLimit--)
    {
        if (screen != GameScreen)   //if game ends we end this thread, otherwise we focus on timer
        {
            return;
        }
        else
        {
            if (YourTurn == false)    //timer will pause when player's turn ends
            {
                std::unique_lock<std::mutex> lock(IsPlayersTurn);
                PT.wait(lock);
            }
            else             //when it's player's turn
            {
                Sleep(1000);       //wait for 1 second
            }
        }
    }

    YourTurn = 0;    //time for your move ends
    cv.notify_one();
}

void GameThread(char Opponent_level, char& Mark, Screen& screen, std::string& ResultValue, int& TimeLimit, bool& YourTurn)
{   
    char YourMark;   //we need to know what is player's mark to identify if he win or lose later
    ResultValue = "Draw";
    YourTurn = 0;
    if (Opponent_level == 'm')
    {
        //multiplayer game
    }
    else    //single player fragment
    {
        srand(time(NULL));    //in single player game we use random to detect if you go first or second. If 0 u go second if 1 u go first
        if (rand() % 2 == 0)
        {
            YourMark = 'O';   //If your move is not first then you have 'O', because O also is a second one
            YourTurn = 0;
        }
        else
        {
            YourMark = 'X';  //If your move is first then you have 'X', because X goes first
            YourTurn = 1;
        }

        for(int x = 0; x<9; x++)   //we do it 9 times, because we have 9 places to fill
        {
            if (YourTurn == 1)
            {
                PT.notify_one();   //start timer
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

                YourTurn = 0;        //enemy's turn

                if (TimeLimit == 0)    //if time ends you lose and the game is ended
                {
                    ResultValue = "Lose";
                    break;
                }
                else
                {
                    TimeLimit = 15; //after player move we restart TimeLimit
                }
            }
            else     //opponent move
            {
                int aiStatus = 0;      //for AI controlling
                if (Opponent_level == 'h') //if opponent AI is set on 'hard' level then it checks if it has to block or can istant win. If not he does the same as easy AI
                {
                    aiStatus = HardAiAlgorithm(Mark);   //hard AI algorithm - we give it 'Mark', because it must know if it has X or O
                }

                if (aiStatus == 0) //randomly place marks (this is what easy AI do or hard AI when it doesn't block or istant win)
                {
                    AiRandomMove(Mark);
                }

                if (Mark == 'X')   //change to opposite
                {
                    Mark = 'O';
                }
                else
                {
                    Mark = 'X';
                }

                YourTurn = 1;       //player's turn
            }
            
            int result = BoardCheck(); //we check if anyone win

            if (result == 1 or result == 2)  //if someone win we break the loop
            {
                YourTurn = 2;        //here we block 2 players, because the game ends, because someone wins

                if (result == 1)    //check what exactly the function BoardCheck returns. If X wins then check if X was player's mark. If yes player wins if not player loses
                {
                    if (YourMark == 'X')      //result tell us if X wins or O   ResultValue tell us if player wins or not.
                    {
                        ResultValue = "Win";
                        break;
                    }
                    else
                    {
                        ResultValue = "Lose";
                        break;
                    }
                }
                else
                {
                    if (YourMark == 'O')   //the same for O
                    {
                        ResultValue = "Win";
                        break;
                    }
                    else
                    {
                        ResultValue = "Lose";
                        break;
                    }
                }
            }
        }
    }
    Sleep(500);  //wait for half second, so player can see the final board
    ResetGameBoard();
    Mark = 'X';      //reset Mark to X, cause X must be first, otherwise we will have problems (line 57 Client.cpp)
    YourTurn = 0; //reset YourTurn variable
    screen = Result;
    PT.notify_one();   //The Time Control thread can be locked, so we have to unlock it 
}

int main()
{
    //Variable for threads
    int TimeLimit = 15; //Time limit is 15 seconds
    std::string ResultValue;   //here we will have results
    char Mark = 'X';   //Mark is used to check the symbol (X or O) - X goes first
    bool YourTurn = 0;    //Tell us if it's player's turn or not

    bool HasTimeControl = false;

    GLFWwindow* window{};   //GLFW window

    //GLFW initialization
    if (!glfwInit()) {
        fprintf(stderr, "Can't run GLFW.\n");
        exit(EXIT_FAILURE);
    }

    //create window (size and title)
    window = glfwCreateWindow(1920, 1080, "TicTacToe", glfwGetPrimaryMonitor(), NULL);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height
    glfwSetWindowSizeLimits(window, mode->width, mode->height, mode->width, mode->height);      //window's size is the same as screen size
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
            ShowLogo();

            //Single Player Button
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height*7/24), NULL);
            ImGui::Begin("Singleplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
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

        //player chooses easy or hard opponent
        else if (screen == Single_Menu)
        {
            //TicTacToe LOGO
            ShowLogo();

            //easy opponent
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("EasyOpponent", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Easy Opponent", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = GameScreen;
                //Here we starts thread for single player game and detach.
                std::thread(GameThread, 'e', std::ref(Mark), std::ref(screen), std::ref(ResultValue), std::ref(TimeLimit), std::ref(YourTurn)).detach();
                if (HasTimeControl == true)                //if player didn't disable time control then we have to start extra thread for it
                {
                    std::thread(TimeControl, std::ref(TimeLimit), std::ref(YourTurn), std::ref(screen)).detach();
                }
                //e means easy opponent, h means hard opponent, m means multiplayer game
            }

            //hard opponent
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("HardOpponent", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Hard Opponent", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = GameScreen;
                //Here we starts thread for single player game and join.
                std::thread(GameThread, 'h', std::ref(Mark), std::ref(screen), std::ref(ResultValue), std::ref(TimeLimit), std::ref(YourTurn)).detach();
                if (HasTimeControl == true)             //if player didn't disable time control then we have to start extra thread for it
                {
                    std::thread(TimeControl, std::ref(TimeLimit), std::ref(YourTurn), std::ref(screen)).detach();
                }
                //e means easy opponent, h means hard opponent, m means multiplayer game
            }

            //Enable/disable timer button
            ImGui::SetNextWindowSize(ImVec2(mode->width / 6, mode->height / 7), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width*3.7/6, mode->height * 7 / 24), NULL);
            ImGui::Begin("Time control", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::Text("Time control:");
            if (HasTimeControl == false)
            {
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - 25);
                if (ImGui::Button("Disabled", ImVec2(mode->width*0.14,mode->height*0.1)))
                {
                    HasTimeControl = true;
                }
            }
            if (HasTimeControl == true)
            {
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - 25);
                if (ImGui::Button("Enabled", ImVec2(mode->width * 0.14, mode->height * 0.1)))
                {
                    HasTimeControl = false;
                }
            }

            //back
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        //if player chooses multiplayer game we need him to insert an IP adres
        else if (screen == IP_Insert)
        {
            //TicTacToe LOGO
            ShowLogo();

            //insert IP
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);

            //back
            SetBackButton();
            style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        //if player plays he should see a board
        else if (screen == GameScreen)
        {
            //TicTacToe LOGO
            ShowLogo();

            texture = loadTexture("Board.bmp");
            ImGui::SetNextWindowPos(ImVec2(mode->width*0.31, mode->height*0.27), NULL);
            ImGui::SetNextWindowSize(ImVec2(mode->width*0.36, mode->height*0.64), NULL);
            ImGui::Begin("Board", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(mode->width * 0.36, mode->height * 0.64));
            ImGui::End();

            if (HasTimeControl == true)
            {
                std::string TimeLeft = "Time: " + std::to_string(TimeLimit);
                ImGui::SetNextWindowSize(ImVec2(mode->width / 6, mode->height / 7), NULL);
                ImGui::SetNextWindowPos(ImVec2(mode->width*6.5/10, mode->height * 1 / 24));
                ImGui::Begin("Time", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
                ImGui::SetWindowFontScale(3.0f);
                ImGui::Text(TimeLeft.c_str());
                ImGui::End();
            }

            //we need to fill all 9 field
            for(int i = 0; i < 3; i++)
            {
                for(int a = 0; a < 3; a++)
                {
                    int ID = 3 * i + a;   //calculating ID value
                    if (GameBoardStatus[a][i] == '?')  //if the field is empty than we don't load any texture, but only put a button there
                    {
                        if (YourTurn)   //you are allowed to move only when it's your turn
                        {
                            ImGui::SetNextWindowPos(ImVec2(mode->width * 0.32 + (i * mode->width * 0.12), mode->height * 0.285 + (a * mode->height * 0.215)), NULL);
                            ImGui::SetNextWindowSize(ImVec2(mode->width * 0.11, mode->height * 0.19), NULL);
                            ImGui::Begin(std::to_string(ID).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
                            if (ImGui::InvisibleButton("Button", ImVec2(mode->width * 0.1, mode->height * 0.18)))   //that button allow player to place Mark at empty place
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
                        
                        //In ImGui every window must have different name (this is like their ID). With that (3*i+a) windows will have names like 1, 2, 3 etc . It also allow us to allocate them
                        ImGui::SetNextWindowPos(ImVec2(mode->width * 0.32 + (i * mode->width * 0.12), mode->height * 0.285 + (a * mode->height * 0.215)), NULL);
                        ImGui::SetNextWindowSize(ImVec2(mode->width * 0.11, mode->height * 0.19), NULL);
                        ImGui::Begin(std::to_string(ID).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
                        ImGui::Image((void*)(intptr_t)texture, ImVec2(mode->width * 0.1, mode->height * 0.18));
                        ImGui::End();
                    }
                }
            }
        }

        else if (screen == Result)
        {
            ShowLogo();

            //Info about results
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
            ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 4/10);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
            ImGui::Text(ResultValue.c_str());
            ImGui::End();

            //back
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Single_Multi_Choose;
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