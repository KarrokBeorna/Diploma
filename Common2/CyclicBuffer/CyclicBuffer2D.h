/*
* CyclicBuffer2D.h
*
*  Created on: Feb 28, 2022
*      Author: Artem Khlybov
*/


#ifndef CYCLICBUFFER2D_MULTITHREADTEST_H_
#define CYCLICBUFFER2D_MULTITHREADTEST_H_


#include "CyclicBuffer1D.h"

template<typename T>
class CCyclicBuffer2D: public CCyclicBuffer1D<T>
{
private:
	int CurrID;
	unsigned int DepthInFrames;
	unsigned int N_LinesPerFrame;
	unsigned int N_PointsPerLine;

	T ** Buffer; // Main buffer
	unsigned int GetSize() = 0;

public:
	// Cunstructor
	CCyclicBuffer2D(const unsigned int DepthInFrames, const unsigned int N_LinesPerFrame, const unsigned int N_PointsPerLine)
	{
		Buffer = NULL;
		CurrID = 0;
		this->DepthInFrames = DepthInFrames;
		this->N_LinesPerFrame = N_LinesPerFrame;
		this->N_PointsPerLine = N_PointsPerLine;
		const unsigned long Size = N_LinesPerFrame * N_PointsPerLine;
		for (unsigned int n = 0; n < DepthInFrames; n++)
		{
			Buffer = (T**)calloc(DepthInFrames, sizeof(T*));
			Buffer[n] = (T*)calloc(Size, sizeof(T));
		}
	}



#endif // CYCLICBUFFER2D_MULTITHREADTEST_H_

