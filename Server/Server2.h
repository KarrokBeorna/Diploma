/*
* Server2.h
*
*  Created on: Feb 22, 2022
*      Author: Artem Khlybov
*/

#ifndef SERVER2_SIGNALEMULATOR_H_
#define SERVER2_SIGNALEMULATOR_H_


#include "PassiveSocket.h"
#include "VectorMatrix/LargeDataMatrix.h"


// This class represents a TCP server 
class CServer2
{
private:
	bool Ready; // Flag of readiness
	unsigned int N_Channels; // Number of spatial channels
	unsigned int FrameID; // ID of the current frame
	int ErrorCode; // Error code
	unsigned int SendPacketSize; // Length in bytes of the data packet to send
	unsigned int RecievePacketSize; // Length in bytes of the recieve data packet
	unsigned long iPort; // Port as an integer
	CLargeDataMatrix<float>* pDataToSend; // Input dataset for sending
	CPassiveSocket ListenSocket; // Socket for listening the port
	CActiveSocket * pClientSocket; // Client socket
	uint8* SendDataPacketBuffer; // Data buffer for sending one frame

	void PrepareDataPacketForSending(float* DataArray, const unsigned int FrameID, const unsigned int StartChannel, const unsigned short N_ChannelsToSend); // Fetch one data packet from the array to send, transorm them to binary format and and fill the send buffer
	void ShutdownConnections(); // Shuts down all sockets
	bool SendCurrDataFrame(); // Sends current data frame
	void IncFrame(); // Go to the next frame
	void SendMetadata(); // Sends metadata to the client
	void SendStatus(); // Sends server status to the client
	bool SendMessageToConnected(const unsigned short Msg); // Sends a message to connected abonents

public:
	uint8 Status; // Server status: 0 - no computations, 1 - computations running online, 2 - computations running offline
	unsigned int CurrProcessedFrameID; // ID of the currently processed frame

	CServer2(CLargeDataMatrix<float>* DataToSend, const unsigned int SendPacketSize, const unsigned int RecievePacketSize, const unsigned long Port); // Constructor
	virtual ~CServer2(); // Destructor
	void RunPortListening(); // Server starts listening the port

	// Properties
	int GetErrorCode(); // Gets the error code
};



#endif /*SERVER2_SIGNALEMULATOR_H_*/







