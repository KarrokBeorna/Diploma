/*
* SignalBuffer.h
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/

#ifndef SIGNALBUFFER_SIGNALEMULATOR_H_
#define SIGNALBUFFER_SIGNALEMULATOR_H_

#include "VectorMatrix/LargeDataMatrix.h"


// The class represents the main buffer of signals, which has three dimensions: spatial channels, time frames, points in a frame

class CSignalBuffer
{
private:
	bool Ready;	// Flag of readiness
	unsigned long N_Channels;				// Number of channels
	unsigned long N_TimeFrames;				// Number of time frames
	unsigned int FrameSize;					// Frame size = cell size (2^n)
	unsigned long N_NoisePoints;			// Number of noise points in the array of white noise 
	float* Noise_Signal;					// White noise array
	float* Buffer;							// main array with data
	CLargeDataMatrix<float>* MapOfAggregatedScores; // 2D spatio-temporal distribution of cell energies (or other aggregated score values like SNR). Rows = channels, columns = time frames

	void Init();	// Initializes internal structures

public:
	CSignalBuffer(const unsigned long N_Channels, const unsigned long N_TimeFrames, const unsigned int FrameSize); // Constructor with main parameters
	~CSignalBuffer(); // Destructor
	bool ReadFile_WithNoise(const char * NoiseFileName); // Reads an external formatted file of white noise
	void FillBufferWithNoise(const float NoisePower = 0.0f); // Fills the buffer with white noise or with zeros

		// Access to properties
	bool IsReady(); // Gets the flag of readiness
	float* GetDataCell(const unsigned long ChannelID, const unsigned long FrameID); // Gets a data cell as a 1D array, length = FrameSize
	float GetAggregatedScore(const unsigned long ChannelID, const unsigned long FrameID); // Gets the value of an aggregated score related to a data cell
	void SetAggregatedScore(const unsigned long ChannelID, const unsigned long FrameID, const float Score); // Sets the aggregated score related to a data cell
	CLargeDataMatrix<float>* GetMapOfScores(); // Gets the internal map of aggregated scores
	unsigned long GetNumberOfChannels(); // Gets the number of spatial channels
	unsigned long GetNumberOfTimeFrames(); // Gets the number of time frames
};




#endif /*SIGNALBUFFER_SIGNALEMULATOR_H_*/



