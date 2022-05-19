/*
* CyclicBuffer1D.h
*
*  Created on: Feb 28, 2022
*      Author: Artem Khlybov
*/


#ifndef CYCLICBUFFER1D_MULTITHREADTEST_H_
#define CYCLICBUFFER1D_MULTITHREADTEST_H_

#include <cstdio>


template<typename T>
class CCyclicBuffer1D
{
private:
	int CurrID;
	unsigned int DepthInFrames;
	unsigned int N_PointsPerFrame;
	unsigned long N_Frames_Processed;

	T** Buffer; // Main buffer

public:
	bool FlagRun;

	// Constructor
	CCyclicBuffer1D(const unsigned int DepthInFrames, const unsigned int N_PointsPerFrame)
	{
		Buffer = NULL;
		CurrID = 0;
		N_Frames_Processed = 0;
		FlagRun = true;
		this->DepthInFrames = DepthInFrames;
		this->N_PointsPerFrame = N_PointsPerFrame;
		Buffer = (T**)calloc(DepthInFrames, sizeof(T*));
		for (unsigned int n = 0; n < DepthInFrames; n++)
		{
			Buffer[n] = (T*)calloc(N_PointsPerFrame, sizeof(T));
		}
	}

	// Destructor
	virtual ~CCyclicBuffer1D()
	{
		if (Buffer != NULL)
		{
			for (unsigned int n = 0; n < DepthInFrames; n++) free(Buffer[n]);
			free(Buffer);
		}
		Buffer = NULL;
	}

	// Increments the current subbuffer
	void IncID()
	{
		CurrID++;
		if (CurrID >= DepthInFrames) CurrID = 0;
		N_Frames_Processed++;
	}

	// Gets the current subbuffer
	T* GetCurrBuffer() { return Buffer[CurrID]; }


	// Gets the previous subbuffer
	T* GetPrevBuffer()
	{
		int PrevID = GetPrevID();
		return Buffer[PrevID];
	}

	int GetCurrID()
	{
		return CurrID;
	}

	int GetPrevID()
	{
		int PrevID = CurrID - 1;
		if (PrevID < 0) PrevID = DepthInFrames - 1;
		return PrevID;
	}

	unsigned int GetDepth()
	{
		return DepthInFrames;
	}

	T* GetBuffer(const unsigned int BuffID) { return Buffer[BuffID]; }


	unsigned int GetSize()
	{
		return N_PointsPerFrame;
	}
	

	unsigned long GetNumberOfProcessedFrames()
	{
		return N_Frames_Processed;
	}

};




#endif // CYCLICBUFFER1D_MULTITHREADTEST_H_

