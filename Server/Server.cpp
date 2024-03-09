#undef UNICODE

#define WIN32_LEAN_AND_MEAN

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
#include "file.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 256

std::mutex semaphore;
std::condition_variable sem;    //should be more or a vector

struct SemData
{
    std::string id = "- 1";   //id
    int x;    //board coodinate
    int y;
    bool Done;   //for synchronization
    int rand;    //for random value
};

int GetPositionInVector(int Index, std::vector<Players>& PlayersData)
{
    for (int i = 0; i < PlayersData.size(); i++)
    {
        if (PlayersData[i].Index == Index)
        {
            return i;
        }
    }
}

void ClientThread(SOCKET ClientSocket, std::vector<Players>& PlayersData, std::vector<SemData>& semdata)
{
    char sendbuf[256];
    char recvbuf[256];
    memset(sendbuf, NULL, sizeof(sendbuf));    //we clear the memory
    memset(recvbuf, NULL, sizeof(recvbuf));
    int Index = -1;  //Index identify the struct in vector that belongs to out Client (PlayersData[x].Index = Index)
    int pos = -1;    //pos is position in vector where the struct is (PlayersData[pos] -> our data)
    bool PlayerRemoveAccount = false;
    int result;
    char Mark;

    sendbuf[0] = '0';  //we send to client message that everything is ready and client thread is created

    result = send(ClientSocket, sendbuf, 256, NULL);
    if (result < 0)
    {
        return;
    }

    do    //we create infinite loop. Server wait for command from client. When it get this, it does some stuff (send answer if necessary) and wait again for another command
    {
        recv(ClientSocket, recvbuf, 256, NULL);

        if (recvbuf[0] == 'L')   //if client try to login
        {
            std::string nick = std::string(recvbuf).substr(2,std::string(recvbuf).find(" ", 2) - 2);     //we take nick
            std::string password = std::string(recvbuf).substr(nick.length() + 3);    //we take password
            
            for (int i = 0; i < PlayersData.size(); i++)        //we check PlayersData to check if that player exist
            {
                if (PlayersData[i].nick == nick and PlayersData[i].password == password and PlayersData[i].IsLogged == false)    //if we find him, we copy the index and send message that the player exist
                {
                    Index = PlayersData[i].Index;
                    PlayersData[i].IsLogged = true;
                    pos = i;
                    result = send(ClientSocket, "Login", sizeof("Login"), NULL);
                    if (result < 0)
                    {
                        return;
                    }
                    break;
                }
            }

            if (Index == -1)    //if we didn't find player (and index is still NULL) we send info about failure
            {
                result = send(ClientSocket, "Error", sizeof("Error"), NULL);
                if (result < 0)
                {
                    return;
                }
            }

            memset(recvbuf, NULL, sizeof(recvbuf));      //we clean the memory
        }

        if (recvbuf[0] == 'R')        //if client try to create a new account
        {
            std::string nick = std::string(recvbuf).substr(2, std::string(recvbuf).find(" ", 2) - 2);     //we take nick
            std::string password = std::string(recvbuf).substr(nick.length() + 3);    //we take password

            bool NickUnique = true;          //we need to check if nick is unique
            for (int i = 0; i < PlayersData.size(); i++)
            {
                if (strcmp(PlayersData[i].nick.c_str(), nick.c_str()) == 0)    //if nick is not unique
                {
                    NickUnique = false;
                    break;            //In case, we find the same nick as we got from client, we don't have to check others 
                }
            }

            if (!NickUnique)
            {
                result = send(ClientSocket, "Error", sizeof("Error"), NULL);    //we send error message (2 players can't have the same nick)
                if (result < 0)
                {
                    return;
                }
            }
            else
            {
                result = send(ClientSocket, "Register", sizeof("Register"), NULL);
                if (result < 0)
                {
                    return;
                }

                do      //we need to find unique index
                {
                    Index++;
                    bool IndexUnique = true;
                    for (int i = 0; i < PlayersData.size(); i++)
                    {
                        if (PlayersData[i].Index == Index)
                        {
                            IndexUnique = false;
                            break;
                        }
                    }

                    if (IndexUnique == true)
                    {
                        break;
                    }

                } while (true);
                PlayersData.push_back({Index, nick, password, 0, 0, true, -2});    //we will identify player with index
                pos = GetPositionInVector(Index, PlayersData);   //because checking the whole vector everytime we need a variable from it would be suboptimal we save our structure position in vector to "pos"
                //everytime when we need our structure we use PlayersData[pos], but we need to make sure first if PlayersData[pos] is our struct, so we check if(PlayerData[pos].Index == Index) - if not we have to find pos again. 
            }
        }

        if (recvbuf[0] == 'C')   //if first char is C that means client wants to change his password
        {
            if (PlayersData[pos].Index != Index)
            {
                pos = GetPositionInVector(Index, PlayersData);
            }

            PlayersData[pos].password = std::string(recvbuf).substr(2);  //overwrite the old password

            result = send(ClientSocket, "CHANGED", sizeof("CHANGED"), NULL);
            if (result < 0)
            {
                return;
            }
        }

        if (std::string(recvbuf) == "DELETE")   //if client want to delete his account, we remove it from vector and break the infinite loop (and then in line 110 it overwrite the .txt file)
        {
            if (PlayersData[pos].Index != Index)
            {
                pos = GetPositionInVector(Index, PlayersData);
            }

            PlayersData.erase(PlayersData.begin() + pos);

            PlayerRemoveAccount = true;
            break;
        }

        if (std::string(recvbuf) == "GiveData")
        {
            result = send(ClientSocket, "START", std::string("START").length(), NULL);
            if (result < 0)
            {
                return;
            }
            std::string datamess;
            for (int i = 0; i < PlayersData.size(); i++)
            {
                datamess = PlayersData[i].nick + "   " + std::to_string(PlayersData[i].win_number) + "   " + std::to_string(PlayersData[i].lose_number);
                result = send(ClientSocket, datamess.c_str(), datamess.length(), NULL);
                if (result < 0)
                {
                    return;
                }
                Sleep(100);
            }
            result = send(ClientSocket, "END", std::string("END").length(), NULL);
            if (result < 0)
            {
                return;
            }
        }

        if (std::string(recvbuf) == "EnterGame")
        {
            if (PlayersData[pos].Index != Index)
            {
                pos = GetPositionInVector(Index, PlayersData);
            }

            PlayersData[pos].WantPlay = -1;

            memset(recvbuf, NULL, sizeof(recvbuf));

            while(true)
            {
                recv(ClientSocket, recvbuf, 256, ERROR_CANT_WAIT);   //this receive do not block

                if (std::string(recvbuf) == "AbandonGame")
                {
                    if (PlayersData[pos].Index != Index)
                    {
                        pos = GetPositionInVector(Index, PlayersData);
                    }

                    PlayersData[pos].WantPlay = -2;
                    break;
                }
                else
                {
                    for (int i = 0; i < PlayersData.size(); i++)
                    {
                        if (PlayersData[i].WantPlay == -1 and PlayersData[i].Index != Index)   //if someone else wants to play
                        {
                            if (PlayersData[pos].Index != Index)
                            {
                                pos = GetPositionInVector(Index, PlayersData);
                            }
                            PlayersData[pos].WantPlay = PlayersData[i].Index;    //we change his and our Index
                            PlayersData[i].WantPlay = Index;
                            break;
                        }
                    }
                }
                if (PlayersData[pos].Index != Index)
                {
                    pos = GetPositionInVector(Index, PlayersData);
                }

                if (PlayersData[pos].WantPlay != -1)    //if we are not waiting we get out of loop
                {
                    break;
                }
            }

            if (PlayersData[pos].WantPlay >= 0)    //if we are playing with someone
            {
                //game
                std::string id;

                send(ClientSocket, "GAME", std::string("GAME").length(), NULL);

                int game_pos;   //position in vector
                if (PlayersData[pos].Index > PlayersData[pos].WantPlay)      //we calculate index
                {
                    id = std::to_string(PlayersData[pos].Index) + std::to_string(PlayersData[pos].WantPlay);
                }
                else
                {
                    id = std::to_string(PlayersData[pos].WantPlay) + std::to_string(PlayersData[pos].Index);
                }

                bool GameIsCreated = false;
                for (int i = 0; i < semdata.size(); i++)      //check if data is created
                {
                    if (semdata[i].id == id)
                    {
                        GameIsCreated = true;
                        game_pos = i;
                        break;
                    }
                }

                if (GameIsCreated == false)     //if data is not created then we create one
                {
                    semdata.push_back({ id, -1, -1, false, -2});  //create data for game
                    for (int i = 0; i < semdata.size(); i++)
                    {
                        if (semdata[i].id == id)   //we write position
                        {
                            game_pos = i;
                        }
                    }
                }

                if (semdata[game_pos].id != id)      //we need to be sure that our data doesn't move
                {
                    for (int i = 0; i < semdata.size(); i++)
                    {
                        if (semdata[i].id == id)
                        {
                            game_pos = i;
                            break;
                        }
                    }
                }
                
                if (semdata[game_pos].rand == -2)
                {
                    semdata[game_pos].rand = -1;   //block
                    srand(time(NULL));
                    int r = rand() % 2;
                    if (r == 0)
                    {
                        semdata[game_pos].rand = 0;   //player with smaller index goes first
                    }
                    else
                    {
                        semdata[game_pos].rand = 1;   //player with bigger index goes first
                    }
                }

                while (true)
                {
                    if (semdata[game_pos].id != id)
                    {
                        for (int i = 0; i < semdata.size(); i++)
                        {
                            if (semdata[i].id == id)
                            {
                                GameIsCreated = true;
                                game_pos = i;
                                break;
                            }
                        }
                    }

                    if (semdata[game_pos].rand == 0)  //smaller index's turn (and smaller index goes first)
                    {
                        break;
                    }
                    else if (semdata[game_pos].rand == 1)   //bigger index's turn (and bigger index goes first)
                    {
                        break;
                    }
                }

                if ((semdata[game_pos].rand == 0 and PlayersData[pos].Index < PlayersData[pos].WantPlay) or (semdata[game_pos].rand == 1 and PlayersData[pos].Index > PlayersData[pos].WantPlay))
                {
                    Mark = 'X';   //if rand == 0 then we check if our index is smaller - if it is smaller that means our turn is first. The same with rand == 1
                }
                else
                {
                    Mark = 'O';    //if we are not first we are second
                }

                int turn_number = 0;
                do    //game loop
                {
                    //check if idexes are correct
                    if (semdata[game_pos].id != id)
                    {
                        for (int i = 0; i < semdata.size(); i++)
                        {
                            if (semdata[i].id == id)
                            {
                                GameIsCreated = true;
                                game_pos = i;
                                break;
                            }
                        }
                    }

                    if (PlayersData[pos].Index != Index)
                    {
                        pos = GetPositionInVector(Index, PlayersData);
                    }

                    if (turn_number % 2 == 0)   //when number is even it X's turn
                    {
                        if (Mark == 'X')
                        {
                            semdata[game_pos].Done = false;
                            send(ClientSocket, "T X", std::string("T X").length(), NULL);    //we send T (like turn) and player's mark
                            recv(ClientSocket, recvbuf, 256, NULL);      //wait until receive
                            if (recvbuf[1] == 'X')
                            {
                                semdata[game_pos].x = 4;
                                semdata[game_pos].y = 4;
                                semdata[game_pos].Done = true;
                                PlayersData[pos].win_number++;      //player won, so increase his win_number
                                send(ClientSocket, "WWWX", 4, NULL);
                                sem.notify_all();
                                break;
                            }
                            else if (recvbuf[1] == 'T')
                            {
                                semdata[game_pos].x = 6;
                                semdata[game_pos].y = 6;
                                semdata[game_pos].Done = true;
                                PlayersData[pos].lose_number++;      //player lose, so increase his lose_number
                                sem.notify_all();
                                break;
                            }
                            else
                            {
                                semdata[game_pos].x = recvbuf[0];
                                semdata[game_pos].y = recvbuf[1];
                                semdata[game_pos].Done = true;
                                sem.notify_all();
                            }
                        }
                        else
                        {
                            do
                            {

                                std::unique_lock<std::mutex> lck(semaphore);     //wait for other's player move
                                sem.wait(lck);

                            } while (semdata[game_pos].Done == false);     //beacuse we have one sem for many games we have to have extra verification

                            char message[4];

                            if (semdata[game_pos].x == 4)     //if client send that he won we need to send other client info that he lost
                            {
                                message[0] = 'W';
                                message[1] = 'L';
                                message[2] = 'L';
                                message[3] = 'X';
                                PlayersData[pos].lose_number++;
                                send(ClientSocket, message, 4, NULL);
                                break;
                            }
                            else if (semdata[game_pos].x == 6)
                            {
                                message[0] = 'W';
                                message[1] = 'T';
                                message[2] = 'T';
                                message[3] = 'X';
                                PlayersData[pos].win_number++;
                                send(ClientSocket, message, 4, NULL);
                                break;
                            }
                            else
                            {
                                message[0] = 'W';
                                message[1] = semdata[game_pos].x;
                                message[2] = semdata[game_pos].y;
                                message[3] = 'X';
                                send(ClientSocket, message, 4, NULL);
                            }
                        }
                    }
                    else
                    {
                        if (Mark == 'O')
                        {
                            semdata[game_pos].Done = false;
                            send(ClientSocket, "T O", std::string("T O").length(), NULL);    //we send T (like turn) and player's mark
                            recv(ClientSocket, recvbuf, 256, NULL);      //wait until receive
                            if (recvbuf[1] == 'O')
                            {
                                semdata[game_pos].x = 5;
                                semdata[game_pos].y = 5;
                                semdata[game_pos].Done = true;
                                PlayersData[pos].win_number++;
                                send(ClientSocket, "WWWO", 4, NULL);     //we sent infor that player won
                                sem.notify_all();
                                break;
                            }
                            else if (recvbuf[1] == 'T')
                            {
                                semdata[game_pos].x = 6;
                                semdata[game_pos].y = 6;
                                semdata[game_pos].Done = true;
                                PlayersData[pos].lose_number++;      //player lose, so increase his lose_number
                                sem.notify_all();
                                break;
                            }
                            else
                            {
                                semdata[game_pos].x = recvbuf[0];
                                semdata[game_pos].y = recvbuf[1];
                                semdata[game_pos].Done = true;
                                sem.notify_all();
                            }
                        }
                        else
                        {
                            do
                            {

                                std::unique_lock<std::mutex> lck(semaphore);     //wait for other's player move
                                sem.wait(lck);

                            } while (semdata[game_pos].Done == false);     //beacuse we have one sem for many games we have to have extra verification

                            char message[4];

                            if (semdata[game_pos].x == 5)     //if client send that he won we need to send other client info that he lost
                            {
                                message[0] = 'W';
                                message[1] = 'L';
                                message[2] = 'L';
                                message[3] = 'O';
                                PlayersData[pos].lose_number++;
                                send(ClientSocket, message, 4, NULL);
                                break;
                            }
                            else if (semdata[game_pos].x == 6)
                            {
                                message[0] = 'W';
                                message[1] = 'T';
                                message[2] = 'T';
                                message[3] = 'O';
                                PlayersData[pos].win_number++;
                                send(ClientSocket, message, 4, NULL);
                                break;
                            }
                            else
                            {
                                message[0] = 'W';
                                message[1] = semdata[game_pos].x;
                                message[2] = semdata[game_pos].y;
                                message[3] = 'O';
                                send(ClientSocket, message, 4, NULL);
                            }
                        }
                    }

                    turn_number++;
                    if (turn_number > 8)
                    {
                        send(ClientSocket, "WD" , 2, NULL);    //if there is 9th turn (board is full) we send info that is draw
                        break;
                    }

                } while (true);
            }
        }

        if (recvbuf[0] == '\n')    //when client click "disconnect", he sends \n mark, so we need to end the loop
        {
            break;  //we break loop
        }

    } while (true);

    SaveFile(PlayersData);  //when player disconnect, server saves data and set IsLogged to false. We save data when player disconnect, because after that the data will not change

    if (PlayerRemoveAccount == false)   //if player removed his account we don't set IsLogged = false, because it doesn't exist
    {
        if (PlayersData[pos].Index != Index)
        {
            pos = GetPositionInVector(Index, PlayersData);
        }
        PlayersData[pos].IsLogged = false;
    }
}

int main()
{
    std::vector<Players> PlayersData;    //vector for players
    std::vector<SemData> semdata;      //vector for multiplayer
    //First, server read data from file
    ReadFromFile(PlayersData);

	//connecting
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, 5);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    do   //we accept players
    {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;       //if something unexpected happened, we will not create client thread
        }

        std::thread(ClientThread, ClientSocket, std::ref(PlayersData), std::ref(semdata)).detach();

    } while (true);

    SaveFile(PlayersData);   //when server is turning off it save data - that never happened
}