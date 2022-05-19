/*
* WinSockServer.cpp
*
*  Created on: Feb 04, 2022
*      Author: Artem Khlybov
*/

#include <cstring>
#include "WinSockServer.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CWinSockServer  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________

// Constructor
CWinSockServer::CWinSockServer(const unsigned int BuffSize, const char* Port)
{
    Ready = false;
    ErrorCode = 0;
    RecieveBuffer = NULL;

	RecieveBufferSize = BuffSize;
	RecieveBuffer = (char*)calloc(BuffSize, sizeof(char));
    strcpy(this->Port, Port);


    WSADATA wsaData;

    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("\nWSAStartup failed with error: %d", iResult);
        ErrorCode = 2;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, Port, &hints, &result);
    if (iResult != 0)
    {
        printf("\ngetaddrinfo failed with error: %d", iResult);
        WSACleanup();
        ErrorCode = 3;
        return;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("\nsocket failed with error: %ld", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        ErrorCode = 4;
        return;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("\nbind failed with error: %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        ErrorCode = 5;
        return;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("\nlisten failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        ErrorCode = 6;
        return;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("\naccept failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        ErrorCode = 7;
        return;
    }

    // No longer need server socket
    closesocket(ListenSocket);
    Ready = true;
}




// Destructor
CWinSockServer::~CWinSockServer()
{
    if (RecieveBuffer != NULL) free(RecieveBuffer);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
    {
        printf("\nshutdown failed with error: %d", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        ErrorCode = 20;
    }
    else // cleanup
    {
        closesocket(ClientSocket);
        WSACleanup();
    }
}




// Send a packet of data
void CWinSockServer::SendData(const char * Data)
{
    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, RecieveBuffer, RecieveBufferSize, 0);
        if (iResult > 0) {
            printf("\nBytes received: %d", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, RecieveBuffer, iResult, 0);
            if (iSendResult == SOCKET_ERROR) 
            {
                printf("\nsend failed with error: %d", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                Ready = false;
                ErrorCode = 20;
                return;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("\nConnection closing...");
        else {
            printf("\nrecv failed with error: %d", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            Ready = false;
            ErrorCode = 21;
            return;

        }

    } while (iResult > 0);

}