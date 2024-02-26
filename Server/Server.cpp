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

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 256

class Gracze
{
	public:
		int line_in_file;  //we save line in file for easier access
		std::string nick;
		std::string password;
		int win_number;
		int lose_number;
};

void ClientThread(SOCKET ClientSocket)
{
    char sendbuf[256];
    memset(sendbuf, ' ', sizeof(sendbuf));    //we clear the memory
    
    sendbuf[0] = '0';  //we send to client message that everything is ready and client thread is created

    int returned;

    returned = send(ClientSocket, sendbuf, 256, NULL);
}

int main()
{
	std::fstream data;
	data.open("Data.txt", std::ios::in | std::ios::out | std::ios::app);   //we open file. If file doesn't exist we create a new file.

	//First, server read data from file
	int line_number = 0;
	std::string line;
	while (!data.eof())    //data schema is: nick(string)[spacebar]password(string)[spacebar]win_number(integer)[spacebar]lose_number(integer)[end line]
	{
		if (std::getline(data, line).fail())     //eof detects the end of file (data ends). But when file is empty (for example we just created the file) eof will not work.
		{										//That's why we need to check if that first getline was correct (if we aren't trying to read from empty file).
			break;							   //.fail() return true when something bad happen and then we use break to stop the whole read file procedure
		}

		int line_position = 0;
		while (line[line_position] != char(32))   //read nick - we read till we got spacebar
		{
			std::cout << line[line_position];
			line_position++;                     //we move one positon forward
		}

		line_position++;                //we move to the next position (we don't want to read space)

		while (line[line_position] != char(32))  //read password - we read till we got spacebar
		{
			std::cout << line[line_position];
			line_position++;
		}

		line_position++;

		while (line[line_position] != char(32))    //read number of wins - we read till we got spacebar
		{
			std::cout << line[line_position];
			line_position++;
		}

		line_position++;

		while (line_position <= line.length())    //read number of losses - because this is the last data in out line, we read till we reach the end of line
		{
			std::cout << line[line_position];
			line_position++;
		}
		std::cout << "" << std::endl;

        line_number++;                        //we increase the line number
	}

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

    do
    {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;       //if something unexpected happened, we will not create client thread
        }

        ClientThread(ClientSocket);

    } while (true);
}