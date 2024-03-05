#pragma once

//network data
WSADATA wsaData;
SOCKET ServerSocket = INVALID_SOCKET;
struct addrinfo* result = NULL, * ptr = NULL, hints;
int iResult;
char recvbuf[256];
char sendbuf[256];
char lastrecv[256];
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
        ServerSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ServerSocket == INVALID_SOCKET) {
            //printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ServerSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ServerSocket);
            ServerSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ServerSocket == INVALID_SOCKET) {
        //printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;   // no error = successful connection
}

void ServerMessages(bool& ConnectStatus)      //here we will receive messages from server
{
    do
    {
        memset(recvbuf, NULL, sizeof(recvbuf));   //we clear the memory

        recv(ServerSocket, recvbuf, 256, NULL);    //we receive

        if (ConnectStatus == false)      //When the connect status is false (we are not online) we break the loop
        {
            break;
        }

        if (std::string(recvbuf) == "Login" or std::string(recvbuf) == "Error" or std::string(recvbuf) == "Register")      //that if is connected with login and register
        {
            WFS.notify_one();
            strcpy_s(lastrecv, recvbuf);    //we copy the message from server, so other functions can read them, and we don't have to worry about that memset in 83 line
        }

        if (recvbuf[0] == '0')    //we use it when we connect with server
        {
            WFS.notify_one();     //we notify the main client that we got message from server
        }

    } while (true);
}

void Disconnect(bool& IsConnected)
{
    sendbuf[0] = '\n';        //we send that specyfic mark so server will know that we disconnect
    send(ServerSocket, sendbuf, 256, NULL);
    shutdown(ServerSocket, closesocket(ServerSocket));     //we shut down the socket to unlock every blocked socket function (such as receive)
    IsConnected = false;
}

int LoginOrRegister(char Option, std::string NickAndPassword, bool& IsConnected)
{
    NickAndPassword = Option + std::string(" ") + NickAndPassword;   //we add the info so we have: [L or C](spacebar)nick(spacebar)password

    int sendResult = send(ServerSocket, NickAndPassword.c_str(), NickAndPassword.length(), NULL);    //we send message to the server
    if (sendResult < 0)   //if something went wrong we call disconnect function
    {
        Disconnect(IsConnected);
        return 2;     //we have to send info to Client.cpp that we lost connection
    }

    std::unique_lock<std::mutex> lck(WaitForServer);           //and we wait for answer
    WFS.wait(lck);

    //we read last receive message from server
    if (std::string(lastrecv) == "Login" or std::string(lastrecv) == "Register")    //if login or register operation is success we return true, otherwise we return false
    {
        memset(lastrecv, NULL, sizeof(lastrecv));    //when we use the lastrecv we clean. We don't have to worry that we clean the next message from server,
        return 0;                                // because server sends messages only when we ask him about something (there's no race condition) - at least in that moment
    }
    else if (std::string(lastrecv) == "Error")
    {
        memset(lastrecv, NULL, sizeof(lastrecv));
        return 1;
    }
}

void DeleteAccounts()
{
    send(ServerSocket, "DELETE", std::string("DELETE").length(), NULL);
}