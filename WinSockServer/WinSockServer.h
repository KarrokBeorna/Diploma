/*
* WinSockServer.h
*
*  Created on: Feb 04, 2022
*      Author: Artem Khlybov
*/


#ifndef WINSOCKSERVER_SIGNALEMULATOR_H_
#define WINSOCKSERVER_SIGNALEMULATOR_H_

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

// #define DEFAULT_BUFLEN 512
// #define DEFAULT_PORT "27015"




// This class implements WinSock2 server
class CWinSockServer
{
private:
    bool Ready; // Flag of readiness
    unsigned int ErrorCode; // Error code
    unsigned int RecieveBufferSize; // Size of the receiving buffer
    char Port[10]; // Port as a string
    int iResult; // Info about result recieving a packet
    int iSendResult; // Info about result of sending a packet 
    SOCKET ListenSocket; // Socket for listening the port
    SOCKET ClientSocket; // Client socket

    char* RecieveBuffer; // Buffer for receiving data

protected:
    // Message types send by the server
    enum PAKOSAM_SERVER_MSG
    {
        PAKOSAM_SERVER_SENDING_METADATA = 0,
        PAKOSAM_SERVER_SENDING_DATA_FRAME = 1,
        PAKOSAM_SERVER_STOPPED = 2,
        PAKOSAM_SERVER_KEEP_WAITING = 3,
        PAKOSAM_SERVER_NEW_FRAME_READY = 4,
        PAKOSAM_SERVER_DATA_FRAME_END = 5

        /*
        SERVER_STATUS = 0,			///< sent by the server when the status of the server has changed
        SERVER_ACQ_RESULTS = 1,			///< client has started an acquire run
        SERVER_CMD_SUCCESS = 2,			///< is sent when certain client commands have been completed successfully
        SERVER_CMD_FAILURE = 3,			///< sent when certain client commands have failed
        SERVER_CRN_DATA = 4,			///< new packet of CRN data
        SERVER_CALC_TS_RESULTS = 5,	///< temperature and strain measurement has been competed
        SERVER_KEEP_ALIVE = 10			///< sent when there is no other information to forward
        */
    };

    // Message types send by the client
    enum PAKOSAM_CLIENT_MSG
    {
        PAKOSAM_CLIENT_REQUEST_METADATA = 0,
        PAKOSAM_CLIENT_REQUEST_DATA_FRAME = 1,
        PAKOSAM_CLIENT_REQUEST_DATA_FRAME_AGAIN = 2,
        PAKOSAM_CLIENT_DISCONNECT = 3
        /*
        CLIENT_STRING = 1,			///< catch all for issuing simple commands to the server
        CLIENT_CRN_UPLOAD = 2,			///< CRN Acquire data
        CLIENT_KEEP_ALIVE = 10			///< sent when there is no other information to forward
        */
    };

public:
    CWinSockServer(const unsigned int BuffSize, const char* Port); // Constructor
    ~CWinSockServer(); // Destructor
    void 
    void SendDataFrame(const char* Data); // Send a packet of data
    void SendMetadata(const char* Data); // Send metadata
};




/*
int CreateServer(const unsigned int BuffSize, const char * Port)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    
    char* recvbuf = (char*)calloc(BuffSize, sizeof(char));

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
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
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) 
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) 
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) 
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) 
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, BuffSize, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    free(recvbuf);

    return 0;
}
*/




#endif /*WINSOCKSERVER_SIGNALEMULATOR_H_*/


