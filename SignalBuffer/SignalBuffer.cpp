/*
* SignalBuffer.cpp
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/

#include <cstdlib>
#include <cstdio>
#include "SignalBuffer.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CSignalBuffer  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________

// Constructor with main parameters
CSignalBuffer::CSignalBuffer(const unsigned long N_Channels, const unsigned long N_TimeFrames, const unsigned int FrameSize)
{
	Init();
	this->N_Channels = N_Channels;
	this->N_TimeFrames = N_TimeFrames;
	this->FrameSize = FrameSize;
	const unsigned long Size = FrameSize * N_Channels * N_TimeFrames;

	Buffer = (float*)calloc(Size, sizeof(float));
	if (Buffer == NULL)
	{
		printf("\nError. Unable to allocate a memory buffer of size %ld. CSignalBuffer::CSignalBuffer", Size);
		return;
	}

	MapOfAggregatedScores = new CLargeDataMatrix<float>(N_Channels, N_TimeFrames);

	Ready = true;
}




// Destructor
CSignalBuffer::~CSignalBuffer()
{
	if (Noise_Signal != NULL) free(Noise_Signal);
	if (Buffer != NULL) free(Buffer);
	if (MapOfAggregatedScores != NULL) delete MapOfAggregatedScores;

}


// Reads an external formatted file of white noise
bool CSignalBuffer::ReadFile_WithNoise(const char* NoiseFileName)
{
	FILE* pFile = fopen(NoiseFileName, "r"); // BinDataFile = fopen ( BinaryFileName , "rb" )
	if (pFile == NULL)
	{
		printf("\nError. Cannot open file %s . CSignalBuffer::ReadFile_WithNoise", NoiseFileName);
		return false;
	}
	/*
	#if defined(_WIN32)
	long pos, end;
	pos = ftell(BinDataFile);
	fseek(BinDataFile, 0, SEEK_END);
	end = ftell(BinDataFile);
	fseek(BinDataFile, pos, SEEK_SET);
	end -= StartPos;
	end /= (N_Channels * N_BytesInOneElement);
	N_TimeSamples = end;
#else
	rewind(BinDataFile);
	fseek(BinDataFile, StartPos, SEEK_SET);
	char * buffer = (char*)malloc(N_Channels * N_BytesInOneElement);
	while (!feof(BinDataFile))
	{
		fread(buffer, 1, N_Channels * N_BytesInOneElement, BinDataFile);
		N_TimeSamples++;
	}
	N_TimeSamples--;
	free(buffer);
#endif
	*/

	long n_lines = 0;
	char TmpStr[100];
	while (!feof(pFile))
	{
		fgets(TmpStr, 100, pFile);
		n_lines++;
	}
	n_lines--;
	if (n_lines <= 0)
	{
		printf("\nError. File %s is empty. CSignalBuffer::ReadFile_WithNoise", NoiseFileName);
		fclose(pFile);
		return false;
	}
	N_NoisePoints = n_lines;
	rewind(pFile);
	Noise_Signal = (float*)calloc(N_NoisePoints + 16, sizeof(float));
	float fTmp;
	for (unsigned long i = 0; i < N_NoisePoints; i++)
	{
		fscanf(pFile, "%f", &fTmp);
		Noise_Signal[i] = fTmp;
	}
	for (unsigned long i = 0; i < 16; i++)
	{
		Noise_Signal[i + N_NoisePoints] = Noise_Signal[N_NoisePoints / 2 + i];
	}

	fclose(pFile);
	return true;
}




// Fills the buffer with white noise
void CSignalBuffer::FillBufferWithNoise(const float NoisePower)
{
	float NoiseStdev = sqrt(NoisePower);
	unsigned long Size = N_Channels * N_TimeFrames * FrameSize;
	unsigned long M = (Size >> 2) << 2;
	unsigned long noise_pos = 0;
	if ((fabs(NoisePower) < 0.0001f) || (Noise_Signal == NULL))
	{
		for (unsigned long i = 0; i < M; i += 4)
		{
			Buffer[i] = 0.0f;
			Buffer[i + 1] = 0.0f;
			Buffer[i + 2] = 0.0f;
			Buffer[i + 3] = 0.0f;
			noise_pos += 4;
			if (noise_pos >= N_NoisePoints) noise_pos = 0;
		}
		for (unsigned long i = M; i < Size; i++) Buffer[i] = 0.0f;
	}
	else
	{
		for (unsigned long i = 0; i < M; i += 4)
		{
			Buffer[i] = Noise_Signal[noise_pos] * NoiseStdev;
			Buffer[i + 1] = Noise_Signal[noise_pos + 1] * NoiseStdev;
			Buffer[i + 2] = Noise_Signal[noise_pos + 2] * NoiseStdev;
			Buffer[i + 3] = Noise_Signal[noise_pos + 3] * NoiseStdev;
			noise_pos += 4;
			if (noise_pos >= N_NoisePoints) noise_pos = 0;
		}
		for (unsigned long i = M; i < Size; i++) Buffer[i] = Noise_Signal[noise_pos] * NoiseStdev;
	}
}




//   ______________________________   Access to properties   ________________________________________________________


// Gets the flag of readiness
bool CSignalBuffer::IsReady()
{
	return Ready;
}


// Gets a data cell as a 1D array, length = FrameSize
float* CSignalBuffer::GetDataCell(const unsigned long ChannelID, const unsigned long FrameID)
{
	return &Buffer[(N_Channels * FrameID + ChannelID) * FrameSize];
}


//   ______________________________   private methods   ________________________________________________________


// Initializes internal structures
void CSignalBuffer::Init()
{
	Noise_Signal = NULL;
	Buffer = NULL;
	MapOfAggregatedScores = NULL;
	Ready = false;
	N_Channels = 0;
	N_TimeFrames = 0;
	FrameSize = 0;
	N_NoisePoints = 0;
}




// Gets the value of an aggregated score related to a data cell
float CSignalBuffer::GetAggregatedScore(const unsigned long ChannelID, const unsigned long FrameID)
{
	float a = MapOfAggregatedScores->GetValue(ChannelID, FrameID);
	return a;
}




// Sets the aggregated score related to a data cell
void CSignalBuffer::SetAggregatedScore(const unsigned long ChannelID, const unsigned long FrameID, const float Score)
{
	MapOfAggregatedScores->ResetValue(ChannelID, FrameID, Score);
}




// Gets the internal map of aggregated scores
CLargeDataMatrix<float>* CSignalBuffer::GetMapOfScores()
{
	return MapOfAggregatedScores;
}



// Gets the number of spatial channels
unsigned long CSignalBuffer::GetNumberOfChannels()
{
	return N_Channels;
}


// Gets the number of time frames
unsigned long CSignalBuffer::GetNumberOfTimeFrames()
{
	return N_TimeFrames;
}