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

void ClientThread(SOCKET ClientSocket, std::vector<Players>& PlayersData)
{
    char sendbuf[256];
    char recvbuf[256];
    memset(sendbuf, NULL, sizeof(sendbuf));    //we clear the memory
    memset(recvbuf, NULL, sizeof(recvbuf));
    int Index = -1;  //Index identify the struct in vector that belongs to out Client (PlayersData[x].Index = Index)
    int pos = -1;    //pos is position in vector where the struct is (PlayersData[pos] -> our data)
    bool PlayerRemoveAccount = false;
    
    sendbuf[0] = '0';  //we send to client message that everything is ready and client thread is created

    send(ClientSocket, sendbuf, 256, NULL);

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
                    send(ClientSocket, "Login", sizeof("Login"), NULL);
                    break;
                }
            }

            if (Index == -1)    //if we didn't find player (and index is still NULL) we send info about failure
            {
                send(ClientSocket, "Error", sizeof("Error"), NULL);
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
                send(ClientSocket, "Error", sizeof("Error"), NULL);    //we send error message (2 players can't have the same nick)
            }
            else
            {
                send(ClientSocket, "Register", sizeof("Register"), NULL);
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
                PlayersData.push_back({Index, nick, password, 0, 0, true});    //we will identify player with index
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

            send(ClientSocket, "CHANGED", sizeof("CHANGED"), NULL);
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

        if (recvbuf[0] == '\n')    //when client click "disconnect", he sends \n mark, so we need to end the loop
        {
            break;  //we break loop
        }

    } while (true);

    SaveFile(PlayersData);  //when player disconnect, server saves data and set IsLogged to false

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

        std::thread(ClientThread, ClientSocket, std::ref(PlayersData)).detach();

    } while (true);

    SaveFile(PlayersData);   //when server is turning off it save data - that never happened
}