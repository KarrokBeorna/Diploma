/*
* SignalSource.cpp
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/

#include <cstdlib>
#include "SignalSource.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CSignalBuffer  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************

//   ______________________________   public methods   ________________________________________________________


// Constructor 1 - Config "TargetTrackingScenario"
CSignalSource::CSignalSource(const unsigned int TargetID, CConfigFileReader* Config)
{
	Init();

	long a;
	float f;
	bool b;
	b = Config->ReadLong("ScenarioGlobalParams", "N_FrameSize", &a);
	if (!b)
	{
		printf("\nError. Parameter =N_FrameSize= is absent in the section ScenarioGlobalParams. CSignalSource::CSignalSource");
		return;
	}
	FrameSize = a;
	this->TargetID = TargetID;

	// TimeFrameDurationInSeconds
	b = Config->ReadLong("ScenarioGlobalParams", "int_PulseRepetitionRateInHertz", &a);
	if (!b)
	{
		printf("\nError. Parameter =int_PulseRepetitionRateInHertz= is absent in the section ScenarioGlobalParams. CSignalSource::CSignalSource");
		return;
	}
	TimeFrameDurationInSeconds = (float)FrameSize / (float)a;

	char SearchString[100];
	sprintf(SearchString, "TargetParams[%d]", TargetID);
	b = Config->SearchSection(SearchString);
	if (!b)
	{
		printf("\nError. Section %s is absent in the config file. CSignalSource::CSignalSource", SearchString);
		return;
	}

	b = Config->ReadLong(SearchString, "int_TargetClass", &a);
	if (!b)
	{
		printf("\nError. Parameter int_TargetClass is absent in the config file, section %s. CSignalSource::CSignalSource", SearchString);
		return;
	}
	if (a < 0)
	{
		printf("\nError. Wrong value (%d) of the parameter int_TargetClass is absent in the config file, section %s. CSignalSource::CSignalSource", a, SearchString);
		return;
	}
	ClassLabel = a;

	b = Config->ReadFloat(SearchString, "float_InitPosX", &f);
	if (!b)
	{
		printf("\nError. Parameter float_InitPosX is absent in the config file, section %s. CSignalSource::CSignalSource", SearchString);
		return;
	}
	CurrPosX = f;

	b = Config->ReadFloat(SearchString, "float_InitPosY", &f);
	if (!b) f = 0.0f;
	CurrPosY = f;

	b = Config->ReadFloat(SearchString, "float_InitVelocityX", &f);
	if (!b)
	{
		printf("\nError. Parameter float_InitVelocityX is absent in the config file, section %s. CSignalSource::CSignalSource", SearchString);
		return;
	}
	CurrVelocityX = f;

	b = Config->ReadFloat(SearchString, "float_InitVelocityY", &f);
	if (!b) f = 0.0f;
	CurrVelocityY = f;

	b = Config->ReadFloat(SearchString, "float_InitTargetSignalPower", &f);
	if (!b)
	{
		printf("\nError. Parameter float_InitTargetSignalPower is absent in the config file, section %s. CSignalSource::CSignalSource", SearchString);
		return;
	}
	CurrTargetSignalEnergy = f;

	char str[1200];
	char* pstr = str;
	b = Config->ReadName(SearchString, "SignalSignatureFileName", pstr);
	if (!b)
	{
		printf("\nError. Parameter SignalSignatureFileName is absent in the config file, section %s. CSignalSource::CSignalSource", SearchString);
		return;
	}

	FILE* pFile = fopen(str, "r");
	if (pFile == NULL)
	{
		printf("\nError. Cannot open file %s . CSignalSource::CSignalSource", str);
		return;
	}

	TargetSignal = new CLargeDataVector<float>(pFile);
	if (!TargetSignal->IsDataReady())
	{
		printf("\nError. Unable to read data from the file %s. CSignalSource::CSignalSource", SearchString);
		return;
	}
	fclose(pFile);

	TmpDataCell = (float*)calloc(FrameSize * 2, sizeof(float));

	IncTimeFrame();
	CurrPosX -= CurrVelocityX * TimeFrameDurationInSeconds; // Velocity in meters per second; we need meters per time frame
	CurrPosY -= CurrVelocityY * TimeFrameDurationInSeconds;

	Ready = true;
}



// Constructor 2 - Empty
CSignalSource::CSignalSource(const unsigned int TargetID)
{
	Init();
	this->TargetID = TargetID;
}



// Destructor
CSignalSource::~CSignalSource()
{
	if ((TargetSignal != NULL) && (!IsSignatureExternal)) delete TargetSignal;
	if (TmpDataCell != NULL) free(TmpDataCell);
}




// Updates all parameters for the next time frame
void CSignalSource::IncTimeFrame()
{
	unsigned long TargetSignalSize = TargetSignal->GetSize();
	float* DataBuffer = TargetSignal->GetInternalData();
	for (unsigned int i = 0; i < FrameSize; i++)
	{
		if (CurrTargetSignalPos >= TargetSignalSize) CurrTargetSignalPos = 0;
		TmpDataCell[i] = DataBuffer[CurrTargetSignalPos];
		CurrTargetSignalPos++;
	}
	unsigned int k = CurrTargetSignalPos;
	for (unsigned int i = 0; i < FrameSize; i++)
	{
		if (k >= TargetSignalSize) k = 0;
		TmpDataCell[i + FrameSize] = DataBuffer[k];
		CurrTargetSignalPos++;
	}

	CurrPosX += CurrVelocityX * TimeFrameDurationInSeconds; // Velocity in meters per second; we need meters per time frame
	CurrPosY += CurrVelocityY * TimeFrameDurationInSeconds;
}


//   ______________________________   Access to properties   ________________________________________________________

// Gets the flag of readiness
bool CSignalSource::IsReady()
{
	return Ready;
}


// Gets current position X in meters
float CSignalSource::GetCurrPosX()
{
	return CurrPosX;
}


// Sets current position X in meters
void CSignalSource::SetCurrPosX(const float X)
{
	CurrPosX = X;
}


// Gets current position Y in meters
float CSignalSource::GetCurrPosY()
{
	return CurrPosY;
}


// Sets current position Y in meters
void CSignalSource::SetCurrPosY(const float Y)
{
	CurrPosY = Y;
}


// Gets current velocity Y in meters per second
float CSignalSource::GetCurrVelocityY()
{
	return CurrVelocityY;
}

// Sets current velocity Y in meters per second
void CSignalSource::SetCurrVelocityY(const float Vy)
{
	CurrVelocityY = Vy;
}


// Gets current velocity X in meters per second
float CSignalSource::GetCurrVelocityX()
{
	return CurrVelocityX;
}

// Sets current velocity X in meters per second
void CSignalSource::SetCurrVelocityX(const float Vx)
{
	CurrVelocityX = Vx;
}



// Updates all parameters for the next time frame
float* CSignalSource::GetSignalFrame()
{
	return TmpDataCell;
}


// Sets current signal energy  (variance)
void CSignalSource::SetCurrSignalEnergy(const float SignalEnergy)
{
	CurrTargetSignalEnergy = SignalEnergy;
}



// Gets current signal energy (variance)
float CSignalSource::GetCurrSignalEnergy()
{
	return CurrTargetSignalEnergy;
}



// Resets internal signature to a new vector
void CSignalSource::ResetSignature(CLargeDataVector<float>* NewSignature)
{
	IsSignatureExternal = true;
	TargetSignal = NewSignature; // to avoid data realloc
	Ready = true;

	// if (TargetSignal != NULL) delete TargetSignal;
	// TargetSignal = new CLargeDataVector<float>(NewSignature);
}


// Sets the class label
void CSignalSource::SetClassLabel(const int ClassLabel)
{
	this->ClassLabel = ClassLabel;
}



// Gets the class label
int CSignalSource::GetClassLabel()
{
	return ClassLabel;
}


// Gets the Target ID
unsigned int CSignalSource::GetTargetID()
{
	return TargetID;
}



// Sets the frame size
void CSignalSource::SetFrameSize(const unsigned int FrameSize)
{
	this->FrameSize = FrameSize;
	if (TmpDataCell != NULL) free(TmpDataCell);
	TmpDataCell = (float*)calloc(FrameSize * 2, sizeof(float));
}




// Gets the frame duration in seconds
float CSignalSource::GetFrameDurationInSeconds()
{
	return TimeFrameDurationInSeconds;
}


//   ______________________________   private methods   ________________________________________________________


// Initializes internal structures
void CSignalSource::Init()
{
	TargetSignal = NULL;
	TmpDataCell = NULL;
	Ready = false;
	IsSignatureExternal = false;
	TargetID = 0;
	FrameSize = 0;
	ClassLabel = -1;
	CurrPosX = 0.0f;
	CurrPosY = 0.0f;
	CurrTargetSignalEnergy = 0.0f;
	CurrTargetSignalPos = 0;
	TimeFrameDurationInSeconds = 1.0;
}

