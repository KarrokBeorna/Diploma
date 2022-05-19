/*
* Server3.cpp
*
*  Created on: May 16, 2022
*      Author: Artem Khlybov
*/

#include <cstring>
#include <cstdlib>
#include <chrono>         // std::chrono::seconds
#include <thread>         // std::thread

#include "Messages.h"
#include "Server3.h"

#define _ENABLE_DEBUGINFO 
#define _HEADER_SIZE_IN_BYTES 12

// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CServer3  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________

// Constructor
CServer3::CServer3(const unsigned int N_Channels, const unsigned int N_Frames, const unsigned int SendPacketSize, const unsigned int RecievePacketSize, const unsigned long Port)
{
	Ready = false;
    this->N_Frames = N_Frames;
    this->N_Channels = N_Channels;
    Status = 0;
    Mode = 0;
    pDataToSend = NULL;
    ErrorCode = CSimpleSocket::CSocketError::SocketSuccess;
    FrameID = 0;
    CurrProcessedFrameID = 0;
    this->SendPacketSize = SendPacketSize;
    this->RecievePacketSize = RecievePacketSize;
    this->iPort = Port;
    N_TotalElementsToSend = 0;

	SendDataPacketBuffer = (uint8*)calloc(SendPacketSize, sizeof(char));

    pClientSocket = NULL;
    if (!ListenSocket.Initialize())
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Initialize failed with error: %d, CServer3::CServer3", ErrorCode);
        return;
    }

	Ready = true;
}




// Destructor
CServer3::~CServer3()
{
	if (SendDataPacketBuffer != NULL) free(SendDataPacketBuffer);
    if(pClientSocket != NULL) delete pClientSocket;
}





// Server starts listening the port
void CServer3::RunPortListening()
{
    bool b = ListenSocket.Listen(NULL, iPort);
    if (!b)
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Listen failed with error: %d, CServer3::RunPortListening", ErrorCode);

        return;
    }

    pClientSocket = ListenSocket.Accept();
    if (pClientSocket == NULL)
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Accept failed with error: %d, CServer3::RunPortListening", ErrorCode);
        return;
    }

    b = true;
    while (b)
    {
        unsigned int RecievedBytePacketSize = pClientSocket->Receive(RecievePacketSize);
        if (RecievedBytePacketSize == 0)
        {
            ErrorCode = ListenSocket.GetSocketError();
            printf("\npClientSocket->Receive failed with error: %d, CServer3::RunPortListening", ErrorCode);
            return;
        }

        uint8* RecieveDataPacketBuffer = pClientSocket->GetData();
        uint8 Msg; 
        
        memcpy(&Msg, &RecieveDataPacketBuffer[0], sizeof(uint8));

        if (Msg == PAKOSAM_CLIENT_REQUEST_METADATA) SendMetadata();

        if (Msg == PAKOSAM_CLIENT_DISCONNECT) break; // TODO: In future: it is not necessary to shut down the server when the client disconnects

        if (Msg == PAKOSAM_CLIENT_REQUEST_DATA_FRAME)
        {
            if (Status == 0) continue; // TODO: instead the server must send a message that the data frame is not available
            unsigned int Parameter;
            memcpy(&Parameter, &RecieveDataPacketBuffer[1], sizeof(unsigned int));
            if (Parameter > 0) Mode = Parameter;
            while (pDataToSend == NULL)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            bool bb = SendCurrDataFrame(N_TotalElementsToSend, pDataToSend);

            if (!bb)
            {
                Status = 0; // Stops computations
                SendMessageToConnected(PAKOSAM_SERVER_PROCESSING_STOPPED);
            }
            else IncFrame();
        }

        if (Msg == PAKOSAM_CLIENT_REQUEST_SERVER_STATUS) SendStatus();

        if (Msg == PAKOSAM_CLIENT_RUN_SERVER_ONLINE)
        {
            if (Status == 0)
            {
                Mode = 1;
                Status = 1;
                SendMessageToConnected(PAKOSAM_SERVER_STARTS_PROCESSING);
                printf("\nServer starts processing");
            }
        }
        if (Msg == PAKOSAM_CLIENT_RUN_SERVER_OFFLINE)
        {
            if (Status == 0)
            {
                Status = 2;
                SendMessageToConnected(PAKOSAM_SERVER_STARTS_PROCESSING);
                printf("\nServer starts processing");
            }
        }
        if (Msg == PAKOSAM_CLIENT_STOP_PROCESSING)
        {
            if (Status != 0)
            {
                Status = 0;
                printf("\nServer stops processing");
            }
        }
    }
    ShutdownConnections();
}





//   ______________________________   access to properties   ________________________________________________________



// Gets the error code
int CServer3::GetErrorCode()
{
    return ErrorCode;
}







//   ______________________________   private methods   ________________________________________________________



// Fetch one data packet from the array to send, transorm them to binary format and and fill the send buffer
void CServer3::PrepareDataPacketForSending(float * DataArray, const unsigned int FrameID, const unsigned int StartElement, const unsigned short N_ElementsToSend)
{
    const uint8 Msg = PAKOSAM_SERVER_SENDING_DATA_FRAME;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(uint8)); // 0 + 1 = 1
    memcpy(&SendDataPacketBuffer[1], &Mode, sizeof(uint8)); // 1 + 1 = 2
    memcpy(&SendDataPacketBuffer[2], &FrameID, sizeof(unsigned int)); // 2 + 4 = 6
    memcpy(&SendDataPacketBuffer[6], &StartElement, sizeof(unsigned int)); // 6 + 4 = 10
    memcpy(&SendDataPacketBuffer[10], &N_ElementsToSend, sizeof(unsigned short)); // 10 + 2 = 12
    // header length = _HEADER_SIZE_IN_BYTES
    
    const unsigned int N_bytes = sizeof(float);
    float* DataArray1 = &DataArray[StartElement];
    for (unsigned int i = 0; i < N_ElementsToSend; i++) memcpy(&SendDataPacketBuffer[_HEADER_SIZE_IN_BYTES + i * N_bytes], &DataArray1[i], N_bytes);


#ifdef _ENABLE_DEBUGINFO
    printf("\n  Frame %d, Channels[%d ... %d]", FrameID, StartElement, StartElement + N_ElementsToSend);
    // If printing out the content of the sent packets needed
    /*
    for (unsigned short int i = 0; i < N_ChannelsToSend; i++)
    {
        printf("\n  %lE", DataArray1[i]);
    }
    */
#endif // _ENABLE_DEBUGINFO
    
}






// Shuts down all sockets
void CServer3::ShutdownConnections()
{
    printf("\nServer closes the connection");
    try
    {
        if (!pClientSocket->Close())
        {
            ErrorCode = pClientSocket->GetSocketError();
            printf("\npClientSocket->Close failed with error: %d, CServer3::ShutdownConnections", ErrorCode);
        }
    }
    catch (...)
    {     }
}






// Sends one data frame
// N_TotalElements = N_Channels;
// VectorToSend = pDataToSend->GetColumn(FrameID)->GetInternalData();
bool CServer3::SendCurrDataFrame(const unsigned int N_TotalElements, float * VectorToSend)
{
    if(FrameID >= N_Frames) // Sending a message about stop sending the data
    {
        // SendMessageToConnected(PAKOSAM_SERVER_PROCESSING_STOPPED);
        // Status = 0; // Stops computations
        printf("\nAll data were sent");
        FrameID = 0;
        return false;
    }

    // Delay is necessary: the server waits for the data processing
    while (CurrProcessedFrameID == FrameID)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /*
    if (VectorToSend == NULL)
    {
        printf("\nError. Data array to send is not assigned. CServer3::SendCurrDataFrame");
        ErrorCode = 55;
        return false;
    }
    */

#ifdef _ENABLE_DEBUGINFO
    printf("\n__________Sending Frame %ld___________", FrameID);
#endif
    const unsigned int N_chars = sizeof(float);
    unsigned short int N_ElementsToSend = (SendPacketSize - _HEADER_SIZE_IN_BYTES) / N_chars; // _HEADER_SIZE_IN_BYTES s for the packet's header
    unsigned int CurrStartChannel = 0;

    bool b = true;
    while (b)
    {
        if (CurrStartChannel + N_ElementsToSend >= N_TotalElements)
        {
            N_ElementsToSend = N_TotalElements - CurrStartChannel; // TODO: Send error message to the client 
            b = false;
        }
        PrepareDataPacketForSending(VectorToSend, FrameID, CurrStartChannel, N_ElementsToSend);
        
        if (!pClientSocket->Send(SendDataPacketBuffer, SendPacketSize))
        {
            ErrorCode = pClientSocket->GetSocketError();
            printf("\npClientSocket->Send failed with error: %d, CServer3::SendDataFrame", ErrorCode);
        }

        CurrStartChannel += N_ElementsToSend;
    }

    // Sending a message about ending the frame
    if(!SendMessageToConnected(PAKOSAM_SERVER_DATA_FRAME_END)) return false;

    return true;
}





// Go to the next frame
void CServer3::IncFrame()
{
    FrameID++;
}





// Sends metadata to the client
void CServer3::SendMetadata()
{
#ifdef _ENABLE_DEBUGINFO
    printf("\nSending metadata");
#endif

    // unsigned int N_chars = sizeof(unsigned short int);
    const uint8 Msg = PAKOSAM_SERVER_SENDING_METADATA;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(uint8)); // 0 + 1 = 1
    memcpy(&SendDataPacketBuffer[1], &N_Channels, sizeof(unsigned int)); // 1 + 4 = 5
    memcpy(&SendDataPacketBuffer[5], &N_Frames, sizeof(unsigned int)); // 5 + 4 = 9
    // header size is 9 bytes: 1 + 4 + 4

    if (!pClientSocket->Send(SendDataPacketBuffer, 9))  
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer3::SendMetadata", ErrorCode);
    }
}


// Sends server status to the client
void CServer3::SendStatus()
{
#ifdef _ENABLE_DEBUGINFO
    printf("\nSending server status");
#endif

    const unsigned short Msg = PAKOSAM_SERVER_SENDING_STATUS;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(uint8));

    memcpy(&SendDataPacketBuffer[1], &Status, sizeof(uint8));
    // header size is 2 bytes: 1 + 1

    if (!pClientSocket->Send(SendDataPacketBuffer, 4))
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer3::SendMetadata", ErrorCode);
    }
}



// Sends a message to connected abonents
bool CServer3::SendMessageToConnected(const uint8 Msg)
{
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(uint8));
    bool b = true;
    if (!pClientSocket->Send(SendDataPacketBuffer, 1))
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer3::SendMessageToConnected", ErrorCode);
        b = false;
    }
    return b;
}
