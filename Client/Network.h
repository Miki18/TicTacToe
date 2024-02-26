#pragma once

//network data
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo* result = NULL, * ptr = NULL, hints;
int iResult;
char recvbuf[256];
#define DEFAULT_PORT "27015"
//////

extern std::mutex WaitForServer;
extern std::condition_variable WFS;   //extern mutex

int ConnectToServer(std::string IP)   //return 1 if not able to connect     return 0 if connect to server successfully
{
    //Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        //printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));    //We connect with TCP (IPPROTO_TCP)
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //Resolve the server address and port
    iResult = getaddrinfo(IP.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        //printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            //printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        //printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;   // no error = successful connection
}

void ServerMessages()
{
    memset(recvbuf, ' ', sizeof(recvbuf));   //we clear the memory (or to be excact - we set all memory in recvbuf to spacebar)
    do
    {
        recv(ConnectSocket, recvbuf, 256, NULL);

        if (recvbuf[0] == '0')
        {
            WFS.notify_one();
        }

    } while (true);
}