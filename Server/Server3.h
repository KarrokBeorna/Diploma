/*
* Server3.h
*
*  Created on: May 16, 2022
*      Author: Artem Khlybov
*/

#ifndef SERVER3_SIGNALEMULATOR_H_
#define SERVER3_SIGNALEMULATOR_H_

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#include "PassiveSocket.h"
#include "VectorMatrix/LargeDataMatrix.h"


// This class represents a TCP server 
class CServer3
{
private:
	bool Ready; // Flag of readiness
	unsigned int N_Channels; // Number of spatial channels
	unsigned int FrameID; // ID of the current frame
	unsigned int N_Frames; // Total number of frames
	int ErrorCode; // Error code
	unsigned int SendPacketSize; // Length in bytes of the data packet to send
	unsigned int RecievePacketSize; // Length in bytes of the recieve data packet
	unsigned long iPort; // Port as an integer
	CPassiveSocket ListenSocket; // Socket for listening the port
	CActiveSocket * pClientSocket; // Client socket
	uint8* SendDataPacketBuffer; // Data buffer for sending one frame

	void PrepareDataPacketForSending(float* DataArray, const unsigned int FrameID, const unsigned int StartElement, const unsigned short N_ElementsToSend); // Fetch one data packet from the array to send, transorm them to binary format and and fill the send buffer
	void ShutdownConnections(); // Shuts down all sockets
	bool SendCurrDataFrame(); // Sends current data frame
	void IncFrame(); // Go to the next frame
	void SendMetadata(); // Sends metadata to the client
	void SendStatus(); // Sends server status to the client
	bool SendMessageToConnected(const unsigned char Msg); // Sends a message to connected abonents

public:
	struct SServerFlags
	{
		std::mutex mtx_FrameID;
		std::condition_variable cv_FrameID;
		uint8 Status; // Server status: 0 - no computations, 1 - computations running online, 2 - computations running offline
		uint8 Mode; // Mode of info to send. 1 = Energy of spatial channels. 2 = Table of tracked targets
		unsigned int CurrProcessedFrameID; // ID of the currently processed frame
		unsigned int N_TotalElementsToSend; // Number of elements to send in the current frame
		float* pDataToSend; // External <float> array to send 
	} PublicParams;

//	uint8 Status; // Server status: 0 - no computations, 1 - computations running online, 2 - computations running offline
//	uint8 Mode; // Mode of info to send. 1 = Energy of spatial channels. 2 = Table of tracked targets
//	unsigned int CurrProcessedFrameID; // ID of the currently processed frame
//	unsigned int N_TotalElementsToSend; // Number of elements to send in the current frame
//	float* pDataToSend; // External <float> array to send 

	CServer3(const unsigned int N_Channels, const unsigned int N_Frames, const unsigned int SendPacketSize, const unsigned int RecievePacketSize, const unsigned long Port); // Constructor
	virtual ~CServer3(); // Destructor
	void RunPortListening(); // Server starts listening the port

	// Properties
	int GetErrorCode(); // Gets the error code
};



#endif /*SERVER2_SIGNALEMULATOR_H_*/
