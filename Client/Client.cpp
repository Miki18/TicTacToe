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
#include <cctype>
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
#include "Network.h"

int d;    //for coordinate for multiplayer
int g;

std::mutex wait_for_player;
std::condition_variable cv;

std::mutex IsPlayersTurn;
std::condition_variable PT;

std::mutex WaitForServer;
std::condition_variable WFS;    //mutex for waiting for server

//Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib (for multiplayer game)
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

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
        ResultValue = "None";
        while(true)      //game loop
        {

            if (ResultValue == "Lose")       //we need double check
            {
                break;
            }
            if (ResultValue == "Win")
            {
                break;
            }
            if (ResultValue == "Draw")
            {
                break;
            }

            std::unique_lock<std::mutex> lck(WaitForServer);           //and we wait for answer
            WFS.wait(lck);

            if (ResultValue == "Lose")
            {
                break;
            }
            if (ResultValue == "Win")
            {
                break;
            }
            if (ResultValue == "Draw")
            {
                break;
            }

            YourTurn = 1;     //set your turn on true

            PT.notify_one();     //start timer

            std::unique_lock<std::mutex> lock(wait_for_player);   //wait for player
            cv.wait(lock);

            if (TimeLimit == 0)
            {
                ServerMove(6, 6);     //we send 6 when we used all time
                ResultValue = "Lose";
                TimeLimit = 15;
                break;
            }
            else
            {
                TimeLimit = 15;
            }

            YourTurn = 0;    //your turn ends

            int result = BoardCheck();   //we check Game Board

            if (result == 0)
            {
                ServerMove(d, g);    //send info to the server
            }
            else if (result == 1)
            {
                ServerMove(4, 4);     //4 4 means that X won
            }
            else if (result == 2)
            {
                ServerMove(5, 5);    //5 5 means that O won
            }
        }
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
    char IP[16];     //for IP
    memset(IP, NULL, 16);
    bool IsConnected = false;   //we need to know if we are connected right now or not.
    bool FlowControl = false;    //some code we need to do only once in infinite loop, so we need that variable

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
            SingleMultiChoose(screen);
        }

        //player chooses easy or hard opponent
        else if (screen == Single_Menu)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());  //it tell us screen width and height

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
            ImGui::SetNextWindowPos(ImVec2(mode->width * 3.7 / 6, mode->height * 7 / 24), NULL);
            ImGui::Begin("Time control", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::Text("Time control:");
            if (HasTimeControl == false)
            {
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - 25);
                if (ImGui::Button("Disabled", ImVec2(mode->width * 0.14, mode->height * 0.1)))
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
            IPinsert(screen, IP);
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
                                    d = a;
                                    g = i;
                                }
                                else
                                {
                                    GameBoardStatus[a][i] = 'O';
                                    d = a;
                                    g = i;
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
            ResultScreen(screen, ResultValue, IsConnected);
        }

        else if (screen == Connect_not)
        {
            ConnectNot(screen);
        }

        else if (screen == Connect)
        {
            ShowLogo();

            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
            ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 2.5 / 10);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
            ImGui::Text("Connecting...");
            ImGui::End();
            
            int ConnectionResult = ConnectToServer(std::string(IP));

            if (ConnectionResult == 1)    //if not able to connect show that information to user
            {
                screen = Connect_not;
            }

            else if (ConnectionResult == 0)  //if we connect to server go to multiplayer main menu
            {
                IsConnected = true;

                std::thread(ServerMessages, std::ref(IsConnected), std::ref(Mark), std::ref(ResultValue)).detach();   //we start messages with server

                std::unique_lock<std::mutex> lck(WaitForServer);
                WFS.wait(lck);                   //wait for server

                screen = LoginOrCreateNewAccount;
            }
        }

        else if (screen == LoginOrCreateNewAccount)
        {
            ShowLogo();

            //Login
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);          //set button color
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            if (ImGui::Button("Login", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Login;
            }

            //Create new account
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Register", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Register", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = CreateNewAccount;
            }

            //Exit Button
            SetBackButton();
            if (ImGui::Button("Disconnect", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                Disconnect(IsConnected);
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        else if (screen == Login)
        {
            bool fill[2];    //we use "fill" variable to check which data user provides. Fill is when datas are correct
            bool error[2];  //we use error[2] to show user which data are incorrect
            char nick[30];       //char nick and password are for data
            char password[30];

            if (FlowControl == false)    //we do this only once
            {
                memset(fill, NULL, 2);
                memset(error, false, sizeof(error));
                memset(nick, NULL, sizeof(nick));
                memset(password, NULL, sizeof(password));

                FlowControl = true;
            }

            ShowLogo();

            //write nick
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("Insert_nick", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[0] == true)    //we show user where error is
            {
                ImGui::Text("Maximum 20");
            }
            ImGui::Text("Nick:");
            if (ImGui::InputText("", nick, sizeof(nick), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (std::string(nick).length() < 20)  //check if nick is longer than 20.
                {
                    fill[0] = true;     //user filled nick properly
                    error[0] = false;    //we don't have error now

                    if (fill[0] == true and fill[1] == true)   //if we fill both we sent nick and password to the server
                    {
                        std::string send = std::string(nick) + ' ' + std::string(password);
                        int LoginOrRegisterResult = LoginOrRegister('L', send, IsConnected);    //we send to serwer info what we do (L -> we want to login, R -> we want to register) and nick with password

                        if (LoginOrRegisterResult == 0)
                        {
                            FlowControl = false;
                            screen = MultiplayerMenu;
                        }
                        else if(LoginOrRegisterResult == 1)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = LoginRegiserFailed;
                        }
                        else if (LoginOrRegisterResult == 2)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = Connect_not;
                        }
                    }
                }
                else
                {
                    error[0] = true;   //if nick is longer than 20 we can't accept it and we have to tell user to insert another nick
                }
            }
            ImGui::End();

            //password
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Insert_password", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[1] == true)    //we show user where error is
            {
                ImGui::Text("Maximum 20");
            }
            ImGui::Text("Password:");
            if (ImGui::InputText("", password, sizeof(password), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (std::string(password).length() < 20)  //check if nick is longer than 20.
                {
                    fill[1] = true;     //user filled nick properly
                    error[1] = false;    //we don't have error now

                    if (fill[0] == true and fill[1] == true)   //if we fill both we sent nick and password to the server
                    {
                        std::string send = std::string(nick) + ' ' + std::string(password);
                        int LoginOrRegisterResult = LoginOrRegister('L', send, IsConnected);    //we send to serwer info what we do (L -> we want to login, R -> we want to register) and nick with password

                        if (LoginOrRegisterResult == 0)
                        {
                            FlowControl = false;
                            screen = MultiplayerMenu;
                        }
                        else if (LoginOrRegisterResult == 1)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = LoginRegiserFailed;
                        }
                        else if (LoginOrRegisterResult == 2)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = Connect_not;
                        }
                    }
                }
                else
                {
                    error[1] = true;   //if nick is longer than 20 we can't accept it and we have to tell user to insert another nick
                }
            }
            ImGui::End();

            //Exit Button
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                FlowControl = false;         //we do this one time everytime we enter sceen == login, so if we leave this we have to change FlowControl
                screen = LoginOrCreateNewAccount;
            }
            ImGui::End();
        }

        else if (screen == CreateNewAccount)
        {
            bool fill[3];    //we use "fill" variable to check which data user provides
            bool error[3];
            char nick[30];       //char nick and password are for data
            char password[30];
            char reppassword[30];   //repeat password - user has to provide the password 2 times

            if (FlowControl == false)    //we do this only once
            {
                memset(error, false, 3);
                memset(nick, NULL, sizeof(nick));
                memset(password, NULL, sizeof(password));
                memset(reppassword, NULL, sizeof(password));

                FlowControl = true;
            }

            ShowLogo();

            //write nick
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 5 / 24), NULL);
            ImGui::Begin("Insert_nick", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[0] == true)
            {
                ImGui::Text("Wrong nick");
            }
            ImGui::Text("Nick:");
            if (ImGui::InputText("", nick, sizeof(nick), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                bool alfanumeric = true;      //When client creates new account we have to control what he inserts
                if (std::string(nick).length() < 20)
                {
                    for (int checkalfa = 0; checkalfa < std::string(nick).length(); checkalfa++)
                    {
                        if (!isalnum(nick[checkalfa]))
                        {
                            alfanumeric = false;
                            break;
                        }
                    }

                    if (alfanumeric == false)
                    {
                        error[0] = true;
                        fill[0] = false;
                    }
                    else
                    {
                        error[0] = false;
                        fill[0] = true;
                    }

                    if (fill[0] == true and fill[1] == true and fill[2] == true)
                    {
                        std::string send = std::string(nick) + ' ' + std::string(password);
                        int LoginOrRegisterResult = LoginOrRegister('R', send, IsConnected);    //we send to serwer info what we do (L -> we want to login, R -> we want to register) and nick with password

                        if (LoginOrRegisterResult == 0)
                        {
                            FlowControl = false;
                            screen = MultiplayerMenu;
                        }
                        else if (LoginOrRegisterResult == 1)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = LoginRegiserFailed;
                        }
                        else if (LoginOrRegisterResult == 2)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = Connect_not;
                        }
                    }
                }
                else
                {
                    error[0] = true;
                }
            }
            ImGui::End();

            //password
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
            ImGui::Begin("Insert_password", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[1] == true)
            {
                ImGui::Text("Wrong password");
            }
            ImGui::Text("Password:");
            if (ImGui::InputText("", password, sizeof(password), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                bool alfanumeric = true;
                for (int checkalfa = 0; checkalfa < std::string(password).length(); checkalfa++)
                {
                    if (!isalnum(password[checkalfa]))
                    {
                        alfanumeric = false;
                        break;
                    }
                }

                if (std::string(password).length() >= 20 or (std::string(password) != std::string(reppassword) and std::string(reppassword).empty() == false) or std::string(password).length() <= 0)
                {
                    error[1] = true;
                }
                else
                {
                    if (alfanumeric == false)
                    {
                        error[1] = true;
                        fill[1] = false;
                    }
                    else
                    {
                        error[1] = false;
                        fill[1] = true;
                    }

                    if (fill[0] == true and fill[1] == true and fill[2] == true)
                    {
                        std::string send = std::string(nick) + ' ' + std::string(password);
                        int LoginOrRegisterResult = LoginOrRegister('R', send, IsConnected);    //we send to serwer info what we do (L -> we want to login, R -> we want to register) and nick with password

                        if (LoginOrRegisterResult == 0)
                        {
                            FlowControl = false;
                            screen = MultiplayerMenu;
                        }
                        else if (LoginOrRegisterResult == 1)
                        {
                             FlowControl = false;     //we reset nick and password
                             screen = LoginRegiserFailed;
                        }
                        else if (LoginOrRegisterResult == 2)
                        {
                             FlowControl = false;     //we reset nick and password
                             screen = Connect_not;
                        }
                    }
                }
            }
            ImGui::End();

            //repeat password
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Insert_repeat_password", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[2] == true)
            {
                ImGui::Text("Wrong password");
            }
            ImGui::Text("Repeat Password:");
            if (ImGui::InputText("", reppassword, sizeof(reppassword), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                bool alfanumeric = true;
                for (int checkalfa = 0; checkalfa < std::string(reppassword).length(); checkalfa++)
                {
                    if (!isalnum(reppassword[checkalfa]))
                    {
                        alfanumeric = false;
                        break;
                    }
                }

                if (std::string(reppassword).length() >= 20 or (std::string(password) != std::string(reppassword) and std::string(password).empty() == false) or std::string(reppassword).length() <= 0)
                {
                    error[2] = true;
                }
                else
                {
                    if (alfanumeric == false)
                    {
                        error[2] = true;
                        fill[2] = false;
                    }
                    else
                    {
                        error[2] = false;
                        fill[2] = true;
                    }

                    if (fill[0] == true and fill[1] == true and fill[2] == true)
                    {
                        std::string send = std::string(nick) + ' ' + std::string(password);
                        int LoginOrRegisterResult = LoginOrRegister('R', send, IsConnected);    //we send to serwer info what we do (L -> we want to login, R -> we want to register) and nick with password

                        if (LoginOrRegisterResult == 0)
                        {
                            FlowControl = false;
                            screen = MultiplayerMenu;
                        }
                        else if (LoginOrRegisterResult == 1)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = LoginRegiserFailed;
                        }
                        else if (LoginOrRegisterResult == 2)
                        {
                            FlowControl = false;     //we reset nick and password
                            screen = Connect_not;
                        }
                    }
                }
            }
            ImGui::End();

            //Exit Button
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = LoginOrCreateNewAccount;
            }
            ImGui::End();
        }

        else if (screen == LoginRegiserFailed)
        {
            LoginRegisterFail(screen);
        }

        else if (screen == MultiplayerMenu)
        {
            ShowLogo();

            //Profile
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 11 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("Profile", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);          //set button color
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            if (ImGui::Button("Profile", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Profile;
            }

            //DeleteAccount
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 11 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Delete", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Delete Account", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = DeleteAccount;
            }

            //Singleplayer game
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 4 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("Singleplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);          //set button color
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            if (ImGui::Button("Singleplayer", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = Single_Menu;
            }

            //Multiplayer game
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 4 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Multiplayer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Multiplayer", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                bool res = EnterGame(IsConnected);
                if (res == false)
                {
                    screen = Single_Multi_Choose;
                }
                else
                {
                    screen = WaitForPlayers;
                }
            }

            //Exit Button
            SetBackButton();
            if (ImGui::Button("Disconnect", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                Disconnect(IsConnected);
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        else if (screen == DeleteAccount)     //if player want to delete his account we ask him if he is sure about that
        {
            ShowLogo();

            ImGui::SetNextWindowSize(ImVec2(mode->width / 3, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 9 / 24), NULL);
            ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 4 / 10);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
            ImGui::Text("Are you sure?");
            ImGui::Text("This action is permanent!");
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 4 / 20, mode->height * 19 / 24), NULL);
            ImGui::Begin("No", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("No", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                screen = MultiplayerMenu;
            }
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 11 / 20, mode->height * 19 / 24), NULL);
            ImGui::Begin("Yes", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (ImGui::Button("Yes", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                DeleteAccounts();
                Disconnect(IsConnected);
                screen = Single_Multi_Choose;
            }
            ImGui::End();
        }

        else if (screen == Profile)
        {
            ProfileScreen(screen);
        }

        else if (screen == Stats)
        {
            ShowLogo();

            ImGui::SetNextWindowSize(ImVec2(mode->width / 3, mode->height / 4), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 6 / 20, mode->height * 7 / 24), NULL);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);    //we set gray background for out stats window
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);
            ImGui::SetWindowFontScale(3.0f);
            if (FlowControl == false)
            {
                bool res = GetServerData(IsConnected);
                if (res == false)     //if something is wrong we disconnect and return to starter menu
                {
                    screen = Single_Multi_Choose;
                    FlowControl = false;
                }
                else
                {
                    FlowControl = true;
                }
            }
            ImGui::Text("Nick   win   lose");
            for (int i = 0; i < data.size(); i++)
            {
                ImGui::Text("%s", data[i].c_str());
            }
            ImGui::End();

            style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   //we return to white background
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                FlowControl = false;
                screen = Profile;
            }
            ImGui::End();
        }

        else if (screen == ChangePassword)   //to change password user have to write password 2 times
        {
            char password[30];   //variables
            char reppassword[30];
            bool error[2];
            bool fill[2];

            if (FlowControl == false)     //clear memory
            {
                memset(password, NULL, sizeof(password));
                memset(reppassword, NULL, sizeof(reppassword));
                memset(error, false, sizeof(error));
                memset(fill, false, sizeof(fill));

                FlowControl = true;
            }
            
            ShowLogo();

            //password
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 7 / 24), NULL);
            ImGui::Begin("Insert_nick", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[0] == true)
            {
                ImGui::Text("Wrong password");
            }
            ImGui::Text("New password:");
            if (ImGui::InputText("", password, sizeof(password), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                //cases when error should be true
                if (std::string(password).length() >= 20 or (std::string(password) != std::string(reppassword) and std::string(reppassword).empty() == false) or std::string(password).length() <= 0)
                {
                    error[0] = true;
                }
                else
                {
                    error[0] = false;
                    fill[0] = true;
                    if (fill[0] == true and fill[1] == true)
                    {
                        std::string send = "C " + std::string(password);    //we write C in first position to tell server that we want to change the password
                        bool ret = ChangePasswordMessage(send, IsConnected);
                        if (ret == false)
                        {
                            FlowControl = false;
                            screen = Single_Multi_Choose;    //if something went wrong we go to main menu
                        }
                        else if (ret == true)
                        {
                            FlowControl = false;
                            screen = Profile;    //if we changed the password we go to profile
                        }
                    }
                }
            }
            ImGui::End();

            //repeat password
            ImGui::SetNextWindowSize(ImVec2(mode->width / 4.5, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7.6 / 20, mode->height * 13 / 24), NULL);
            ImGui::Begin("Insert_password", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            if (error[1] == true)
            {
                ImGui::Text("Wrong password");
            }
            ImGui::Text("Repeat password:");
            if (ImGui::InputText("", reppassword, sizeof(reppassword), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                //cases when error should be true
                if (std::string(reppassword).length() >= 20 or (std::string(password) != std::string(reppassword) and std::string(password).empty() == false) or std::string(reppassword).length() <= 0)
                {
                    error[1] = true;
                }
                else
                {
                    error[1] = false;
                    fill[1] = true;
                    if (fill[0] == true and fill[1] == true)
                    {
                        std::string send = "C " + std::string(password);    //we write C in first position to tell server that we want to change the password
                        bool ret = ChangePasswordMessage(send, IsConnected);
                        if (ret == false)
                        {
                            FlowControl = false;
                            screen = Single_Multi_Choose;    //if something went wrong we go to main menu
                        }
                        else if (ret == true)
                        {
                            FlowControl = false;
                            screen = Profile;    //if we changed the password we go to profile
                        }
                    }
                }
            }

            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                FlowControl = false;
                screen = Profile;
            }
            ImGui::End();
        }

        else if (screen == WaitForPlayers)
        {
            ShowLogo();

            //Info about results
            ImGui::SetNextWindowSize(ImVec2(mode->width / 3, mode->height / 6), NULL);
            ImGui::SetNextWindowPos(ImVec2(mode->width * 7 / 20, mode->height * 9 / 24), NULL);
            ImGui::Begin("Wait", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 1 / 10);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
            ImGui::Text("Wait for other player");
            ImGui::End();

            //check if server doesn't send anything
            if (EnterTheGame == true)
            {
                EnterTheGame = false;
                std::thread(GameThread, 'm', std::ref(Mark), std::ref(screen), std::ref(ResultValue), std::ref(TimeLimit), std::ref(YourTurn)).detach();
                HasTimeControl = true;                  //in multiplayer we always have time control
                std::thread(TimeControl, std::ref(TimeLimit), std::ref(YourTurn), std::ref(screen)).detach();
                screen = GameScreen;
            }

            //back
            SetBackButton();
            if (ImGui::Button("Back", ImVec2(mode->width / 5, mode->height / 6.5)))
            {
                bool res = LeaveGame(IsConnected);
                if (res == false)
                {
                    screen = Single_Multi_Choose;
                }
                else
                {
                    screen = MultiplayerMenu;
                }
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