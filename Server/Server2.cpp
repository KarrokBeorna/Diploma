/*
* Server2.cpp
*
*  Created on: Feb 22, 2022
*      Author: Artem Khlybov
*/

#include <cstring>
#include <cstdlib>
#include <chrono>         // std::chrono::seconds
#include <thread>         // std::thread

#include "Messages.h"
#include "Server2.h"

#define _ENABLE_DEBUGINFO 
#define _HEADER_SIZE_IN_BYTES 12

// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CServer2  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________

// Constructor
CServer2::CServer2(CLargeDataMatrix<float>* DataToSend, const unsigned int SendPacketSize, const unsigned int RecievePacketSize, const unsigned long Port)
{
	Ready = false;
    Status = 0;
    pDataToSend = DataToSend;
    ErrorCode = CSimpleSocket::CSocketError::SocketSuccess;
    FrameID = 0;
    CurrProcessedFrameID = 0;
    this->SendPacketSize = SendPacketSize;
    this->RecievePacketSize = RecievePacketSize;
    this->iPort = Port;

    if (!pDataToSend->IsDataReady())
    {
        printf("\nDataset is not ready");
        ErrorCode = 2;
        return;
    }
    N_Channels = pDataToSend->GetNumberOfRows();

	SendDataPacketBuffer = (uint8*)calloc(SendPacketSize, sizeof(char));

    pClientSocket = NULL;
    if (!ListenSocket.Initialize())
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Initialize failed with error: %d, CServer2::CServer2", ErrorCode);
        return;
    }

	Ready = true;
}




// Destructor
CServer2::~CServer2()
{
	if (SendDataPacketBuffer != NULL) free(SendDataPacketBuffer);
    if(pClientSocket != NULL) delete pClientSocket;
}





// Server starts listening the port
void CServer2::RunPortListening()
{
    bool b = ListenSocket.Listen(NULL, iPort);
    if (!b)
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Listen failed with error: %d, CServer2::RunPortListening", ErrorCode);

        return;
    }

    pClientSocket = ListenSocket.Accept();
    if (pClientSocket == NULL)
    {
        ErrorCode = ListenSocket.GetSocketError();
        printf("\nListenSocket.Accept failed with error: %d, CServer2::RunPortListening", ErrorCode);
        return;
    }

    b = true;
    while (b)
    {
        if (!pClientSocket->Receive(RecievePacketSize))
        {
            ErrorCode = ListenSocket.GetSocketError();
            printf("\npClientSocket->Receive failed with error: %d, CServer2::RunPortListening", ErrorCode);
            return;
        }

        uint8* RecieveDataPacketBuffer = pClientSocket->GetData();
        unsigned short Msg; // = *(unsigned short*)byteArray;

        memcpy(&Msg, &RecieveDataPacketBuffer[0], sizeof(unsigned short));

        if (Msg == PAKOSAM_CLIENT_REQUEST_METADATA) SendMetadata();

        if (Msg == PAKOSAM_CLIENT_DISCONNECT) break; // TODO: In future: it is not necessary to shut down the server when the client disconnects

        if (Msg == PAKOSAM_CLIENT_REQUEST_DATA_FRAME)
        {
            if (Status == 0) continue; // TODO: instead the server must send a message that the data frame is not available
            bool bb = SendCurrDataFrame();
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
int CServer2::GetErrorCode()
{
    return ErrorCode;
}



//   ______________________________   private methods   ________________________________________________________



// Fetch one data packet from the array to send, transorm them to binary format and and fill the send buffer
void CServer2::PrepareDataPacketForSending(float * DataArray, const unsigned int FrameID, const unsigned int StartChannel, const unsigned short N_ChannelsToSend)
{
    const unsigned short Msg = PAKOSAM_SERVER_SENDING_DATA_FRAME;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(unsigned short));
    memcpy(&SendDataPacketBuffer[2], &StartChannel, sizeof(unsigned int));
    memcpy(&SendDataPacketBuffer[6], &N_ChannelsToSend, sizeof(unsigned short));
    memcpy(&SendDataPacketBuffer[8], &FrameID, sizeof(unsigned int));
    // header length = _HEADER_SIZE_IN_BYTES

    const unsigned int N_bytes = sizeof(float);
    float* DataArray1 = &DataArray[StartChannel]; 
    for (unsigned int i = 0; i < N_ChannelsToSend; i++) memcpy(&SendDataPacketBuffer[_HEADER_SIZE_IN_BYTES + i * N_bytes], &DataArray1[i], N_bytes);


#ifdef _ENABLE_DEBUGINFO
    printf("\n  Frame %d, Channels[%d ... %d]", FrameID, StartChannel, StartChannel + N_ChannelsToSend);
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
void CServer2::ShutdownConnections()
{
    printf("\nServer closes the connection");
    try
    {
        if (!pClientSocket->Close())
        {
            ErrorCode = pClientSocket->GetSocketError();
            printf("\npClientSocket->Close failed with error: %d, CServer2::ShutdownConnections", ErrorCode);
        }
    }
    catch (...)
    {     }
}






// Sends one data frame
bool CServer2::SendCurrDataFrame()
{
    if(FrameID >= pDataToSend->GetNumberOfCols()) // Sending a message about stop sending the data
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
    

#ifdef _ENABLE_DEBUGINFO
    printf("\n__________Sending Frame %ld___________", FrameID);
#endif
    const unsigned int N_chars = sizeof(float);
    CLargeDataVector<float>* DataVector = pDataToSend->GetColumn(FrameID);
    unsigned short int N_ElementsToSend = (SendPacketSize - _HEADER_SIZE_IN_BYTES) / N_chars; // _HEADER_SIZE_IN_BYTES s for the packet's header
    unsigned int CurrStartChannel = 0;

    bool b = true;
    while (b)
    {
        if (CurrStartChannel + N_ElementsToSend >= N_Channels)
        {
            N_ElementsToSend = N_Channels - CurrStartChannel; // TODO: Send error message to the client 
            b = false;
        }
        PrepareDataPacketForSending(DataVector->GetInternalData(), FrameID, CurrStartChannel, N_ElementsToSend);

        if (!pClientSocket->Send(SendDataPacketBuffer, SendPacketSize))
        {
            ErrorCode = pClientSocket->GetSocketError();
            printf("\npClientSocket->Send failed with error: %d, CServer2::SendDataFrame", ErrorCode);
        }

        CurrStartChannel += N_ElementsToSend;
    }

    // Sending a message about ending the frame
    if(!SendMessageToConnected(PAKOSAM_SERVER_DATA_FRAME_END)) return false;

    return true;
}





// Go to the next frame
void CServer2::IncFrame()
{
    FrameID++;
}





// Sends metadata to the client
void CServer2::SendMetadata()
{
#ifdef _ENABLE_DEBUGINFO
    printf("\nSending metadata");
#endif

    // unsigned int N_chars = sizeof(unsigned short int);
    const unsigned short Msg = PAKOSAM_SERVER_SENDING_METADATA;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(unsigned short));

    memcpy(&SendDataPacketBuffer[2], &N_Channels, sizeof(unsigned int));

    const unsigned int N_Frames = pDataToSend->GetNumberOfCols();
    memcpy(&SendDataPacketBuffer[6], &N_Frames, sizeof(unsigned int));
    // header size is 10 bytes: 2 + 4 + 4

    if (!pClientSocket->Send(SendDataPacketBuffer, 10))  
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer2::SendMetadata", ErrorCode);
    }
}


// Sends server status to the client
void CServer2::SendStatus()
{
#ifdef _ENABLE_DEBUGINFO
    printf("\nSending server status");
#endif

    const unsigned short Msg = PAKOSAM_SERVER_SENDING_STATUS;
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(unsigned short));

    memcpy(&SendDataPacketBuffer[2], &Status, sizeof(unsigned int));
    // header size is 4 bytes: 2 + 2

    if (!pClientSocket->Send(SendDataPacketBuffer, 4))
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer2::SendMetadata", ErrorCode);
    }
}



// Sends a message to connected abonents
bool CServer2::SendMessageToConnected(const unsigned short Msg)
{
    memcpy(&SendDataPacketBuffer[0], &Msg, sizeof(unsigned short));
    bool b = true;
    if (!pClientSocket->Send(SendDataPacketBuffer, 2))
    {
        ErrorCode = pClientSocket->GetSocketError();
        printf("\npClientSocket->Send failed with error: %d, CServer2::SendMessageToConnected", ErrorCode);
        b = false;
    }
    return b;
}
