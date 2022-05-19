/*
* ProcessRegister.cpp
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/

#include <cstring>
#include <cstdlib>
#include <cmath>
#include <chrono>         // std::chrono::seconds
#include <thread>         // std::thread

#include "FunctionTemplates.h"
#include "ProcessRegister.h"

#define NOISE_ENERGY_MULTIPLIER_THRESHOLD 1.2
#define N_COLS_TABLE_OF_VISIBLE_TARGETS 8

// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CProcessRegister  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************


//   ______________________________   public methods   ________________________________________________________

// Constructor 1, config = "TargetTrackingScenario"
CProcessRegister::CProcessRegister(CConfigFileReader* Config)
{
	Init();
	if (!Config->IsFileReady())
	{
		printf("\nError. Config structure is not ready. CProcessRegister::CProcessRegister");
		return;
	}
	pConfig = Config;

	bool b;
	long a;
	float f;
	char name[1200];

	b = pConfig->ReadName("CommonParameters", "Description", name);
	if (!b)
	{
		printf("\nError. Parameter ==Description== in section CommonParameters is absent. CProcessRegister::CProcessRegister");
		return;
	}

	if (strstr(name, "TargetTrackingScenario") == NULL)
	{
		printf("\nError. Wrong format of the config file. Expected: Description = TargetTrackingScenario in section CommonParameters. CProcessRegister::CProcessRegister");
		return;
	}

	b = Init_ScenarioGlobalParams(Config, "ScenarioGlobalParams");
	if (!b) return;
	CleanMainBuffer();

	N_Targets = 0;
	while (true)
	{
		sprintf(name, "TargetParams[%d]", N_Targets);
		b = Config->SearchSection(name);
		if(!b) break;
		N_Targets++;
	}
	if (N_Targets == 0)
	{
		printf("\nError. No one target section in the config file. CProcessRegister::CProcessRegister");
		return;
	}
	Targets = (CSignalSource**)calloc(N_Targets, sizeof(CSignalSource*));
	for (unsigned int t = 0; t < N_Targets; t++)
	{
		Targets[t] = new CSignalSource(t, Config);
		if (!Targets[t]->IsReady()) return;
	}

	TableOfVisibleTargets = new CLargeDataMatrix<float>(N_Targets, N_COLS_TABLE_OF_VISIBLE_TARGETS);
	DataVector = (float*)calloc(N_Channels * 2, sizeof(float));

	// Server params
	b = Config->ReadLong("CommonParameters", "bool_EnableServer", &a);
	if (b)
	{
		b = (a != 0);
	}
	if (b)
	{
		b = Config->ReadLong("CommonParameters", "int_Port", &a);
		if (!b) a = 8080;
		unsigned long port = a;

		b = Config->ReadLong("CommonParameters", "int_MaxSizeDataPacketInBytes", &a);
		if (!b) a = 1344;
		if(a < 80)
		{
			printf("\nError. Too small value in parameter ==int_MaxSizeDataPacketInBytes==, section CommonParameters. Minimal size is 80. CProcessRegister::CProcessRegister");
			return;
		}

		Serv = new CServer3(N_Channels, N_TimeFrames, a, 10, port);
		int ErrCode = Serv->GetErrorCode();
		if (ErrCode > 0)
		{
			printf("\nServer is disabled due to error");
			delete Serv;
			Serv = NULL;
		}
		Serv->Status = 0; // Computations are paused at the beginning until a client message resumes them
	}

	Ready = true;
}




// Constructor 2, config = "SimulationPlan_TargetTracking"
CProcessRegister::CProcessRegister(CConfigFileReader* Config, const char* SectionName)
{
	Init();
	pConfig = Config;
	if (!Config->IsFileReady())
	{
		printf("\nError. Config structure is not ready. CProcessRegister::CProcessRegister");
		return;
	}
	bool b = Init_ScenarioGlobalParams(Config, "ScenarioGlobalParams");
	if (!b) return;

	Ready = true;
}



// Destructor
CProcessRegister::~CProcessRegister()
{
	if ((Targets != NULL) && (!IsTargetsExternal))
	{
		for (unsigned int i = 0; i < N_Targets; i++) if(Targets[i] != NULL) delete Targets[i];
		free(Targets);
	}

	if (MainBuffer != NULL) delete MainBuffer;
	if (TableOfVisibleTargets != NULL) delete TableOfVisibleTargets;
	if (PhaseProc != NULL) delete PhaseProc;
	if (Serv != NULL) delete Serv;
	if (DataVector != NULL) free(DataVector);
}



// Main method
void CProcessRegister::Execute()
{
	if (Serv == NULL) // single thread
	{
		StartComputations();
	}
	else // two threads
	{
		printf("\nServer is ready for data processing");
		std::thread Comp = std::thread(&CProcessRegister::StartComputations, this);
		std::thread Server = std::thread(&CProcessRegister::StartServer, this);
		Comp.join();
		Server.join();
	}
}





// Processes current time frame
void CProcessRegister::ProcessCurrFrame()
{
	const float PowerCutoffThreshold = NoisePower * NOISE_ENERGY_MULTIPLIER_THRESHOLD;
	float f;
	for (unsigned int s = 0; s < N_Targets; s++)
	{
		CSignalSource* Target = Targets[s];
		for (unsigned long c = 0; c < N_Channels; c++)
		{
			float dm = ComputeDistanceInMeters(Target, c);

			float Multiplier = ComputeAmplitudeAttenuation(dm) * sqrt(Target->GetCurrSignalEnergy());
			if ((Multiplier * Multiplier) < NoisePower) continue; // Criterion of large signal attenuation

			unsigned int delay = ComputeDelayInPoints(dm);
			if (delay >= FrameSize) continue; // Criterion of large signal delay

			float* Signal = Target->GetSignalFrame();
			float* DelayedSignal = &Signal[delay];
			float* DataCell = MainBuffer->GetDataCell(c, CurrFrameID);
			for (unsigned long p = 0; p < FrameSize; p += 4)
			{
				DataCell[p] += DelayedSignal[p] * Multiplier;
				DataCell[p + 1] += DelayedSignal[p + 1] * Multiplier;
				DataCell[p + 2] += DelayedSignal[p + 2] * Multiplier;
				DataCell[p + 3] += DelayedSignal[p + 3] * Multiplier;
			}
		}
	} // for (targets: s)

	// Post-processing of all data cells in the current frame
	for (unsigned long c = 0; c < N_Channels; c++)
	{
		float* DataCell = MainBuffer->GetDataCell(c, CurrFrameID);

		// Distortions
		if (EnablePhaseDistortions) PhaseProc->ApplyPhaseDistortions(DataCell);

		// Phase correction
		if (EnablePhaseCorrection) PhaseProc->ApplyPhaseCorrection(DataCell);

		// Compute signal energy
		AuxTemplates::Variance(DataCell, FrameSize, &f);
		f /= (float)FrameSize;

		f -= PowerCutoffThreshold;
		if (f < 0.0f) f = 0.0f;

		MainBuffer->SetAggregatedScore(c, CurrFrameID, f);
	}
}


// Creates output data files
void CProcessRegister::PrepareOutFiles()
{
	bool b;
	char name[1200];
	if (pConfig == NULL) return;
	b = pConfig->ReadName("OutputFiles", "2DTableOfScores_FileName", name);
	if (b)
	{
		if ((MainBuffer != NULL) && (MainBuffer->IsReady()))
		{
			CLargeDataMatrix<float>* Table = MainBuffer->GetMapOfScores();
			if (Table != NULL)
			{
				b = Table->WriteDataIntoFile(name);
				if (!b) printf("\nError. Unable to create file %s.", name);
			}
		}
	}

	b = pConfig->ReadName("OutputFiles", "OutGUI_FileName", name);
	if(b)
	{
		CreateGUIMapTxtFile(name);
	}	
}



// Cleans main internal buffer
void CProcessRegister::CleanMainBuffer()
{
	CurrFrameID = 0;
	MainBuffer->FillBufferWithNoise(NoisePower);
}



// Creates a spatio-temporal map of aggregated values in a form of a text file for the GUI application
bool CProcessRegister::CreateGUIMapTxtFile(const char* GUIMapFileName)
{
	const unsigned int OutputMapContent = 4;
	char BinDataFileName[10] = "NONE";
	const int Attribute = 0;

	FILE* GUIFile = fopen(GUIMapFileName, "w");
	if (GUIFile == NULL)
	{
		printf("\nError. Output GUI map file %s cannot be created", GUIMapFileName);
		return false;
	}

	// Print out header
	fprintf(GUIFile, "\nHEADER_START");
	fprintf(GUIFile, "\nversion=5.1");
	fprintf(GUIFile, "\nMode=%d", OutputMapContent);
	fprintf(GUIFile, "\nBinaryFile=%s", BinDataFileName);
	fprintf(GUIFile, "\nTimeSize=%ld", N_TimeFrames);
	fprintf(GUIFile, "\nSpaceSize=%ld", N_Channels);
	if (OutputMapContent <= 6) fprintf(GUIFile, "\nN_Colours=1");
	else fprintf(GUIFile, "\nN_Colours=10");
	fprintf(GUIFile, "\nHEADER_END");
	fprintf(GUIFile, "\nDATA_START");

	// Print data to the file 
	for (unsigned long t = 0; t < N_TimeFrames; t++) for (unsigned long c = 0; c < N_Channels; c++)
	{
		float Score = MainBuffer->GetAggregatedScore(c, t);
		if (Score > 0.0f) fprintf(GUIFile, "\n%d	%d	%d	%G", t, c, Attribute, Score);
	}

	fprintf(GUIFile, "\nDATA_END");
	fclose(GUIFile);
	return true;
}




// Increments time frame
void CProcessRegister::IncTimeFrame()
{
	for (unsigned int t = 0; t < N_Targets; t++) Targets[t]->IncTimeFrame();
	CurrFrameID++;
}




//   ______________________________   Access to properties   ________________________________________________________

// 
// Gets the flag of readiness
bool CProcessRegister::IsReady()
{
	return Ready;
}




// Gets the main buffer of signals
CSignalBuffer* CProcessRegister::GetMainBuffer()
{
	return MainBuffer;
}



// Gets the array of actual targets at the current state
CSignalSource** CProcessRegister::GetTargets()
{
	return Targets;
}



// Gets the number of targets
unsigned int CProcessRegister::GetNumberOfTargets()
{
	return N_Targets;
}



// Gets the number of time frames
unsigned int CProcessRegister::GetNumberOfTimeFrames()
{
	return N_TimeFrames;
}




// Gets the number of spatial channels
unsigned int CProcessRegister::GetNumberOfChannels()
{
	return N_Channels;
}



// Resets internal targets
void CProcessRegister::ResetTargets(const unsigned int N_Targets, CSignalSource** Targets)
{
	IsTargetsExternal = true;
	this->N_Targets = N_Targets;
	this->Targets = Targets;
	if(TableOfVisibleTargets == NULL) TableOfVisibleTargets = new CLargeDataMatrix<float>(N_Targets, N_COLS_TABLE_OF_VISIBLE_TARGETS);
}




// Gets ID of the current time frame
unsigned int CProcessRegister::GetCurrTimeFrame()
{
	return CurrFrameID;
}




// Gets the frame size
unsigned int CProcessRegister::GetFrameSize()
{
	return FrameSize;
}



// Gets noise power
float CProcessRegister::GetNoisePower()
{
	return NoisePower;
}


// Gets sound velocity
float CProcessRegister::GetSoundVelocityInMetersPerSecond()
{
	return SoundVelocityInMetersPerSecond;
}




// Gets amplitude atenuation coef
float CProcessRegister::GetSoundAmplitudeAttenuationCoef()
{
	return SoundAmplitudeAttenuationCoef;
}



// Gets channel width in meters
float CProcessRegister::GetChannelWidthInMeters()
{
	return ChannelWidthInMeters;
}



// Gets current spatial distribution
CLargeDataVector<float>* CProcessRegister::GetCurrentSpatialDistribution()
{
	CLargeDataMatrix<float> * Map = MainBuffer->GetMapOfScores();
	return Map->GetColumn(CurrFrameID);
}





// Gets the number of currently visible targets for the given Power Cutoff Threshold
unsigned int CProcessRegister::GetNumberOfVisibleTargets(const float PowerCutoffThreshold)
{
	const float alpha = 1.0 / (2.0 * SoundAmplitudeAttenuationCoef * ChannelWidthInMeters);
	unsigned int v = 0;
	for (unsigned int t = 0; t < N_Targets; t++)
	{
		float CenterInMeters = Targets[t]->GetCurrPosX();
		float CenterInChannels = CenterInMeters / ChannelWidthInMeters;
		if ((CenterInChannels >= 0.0) && (CenterInChannels <= (N_Channels - 1)))
		{
			float MaxPower = Targets[t]->GetCurrSignalEnergy();
			if (MaxPower <= PowerCutoffThreshold) continue; // Rejecting small targets
			float OneSideWidthInChannels = log(MaxPower / PowerCutoffThreshold) * alpha;

			TableOfVisibleTargets->ResetValue(v, 0, CenterInChannels);
			TableOfVisibleTargets->ResetValue(v, 1, CenterInChannels - OneSideWidthInChannels);
			TableOfVisibleTargets->ResetValue(v, 2, CenterInChannels + OneSideWidthInChannels);
			TableOfVisibleTargets->ResetValue(v, 3, MaxPower);
			TableOfVisibleTargets->ResetValue(v, 4, Targets[t]->GetCurrPosY());

			// added columns, 2022.05.18
			TableOfVisibleTargets->ResetValue(v, 5, Targets[t]->GetCurrVelocityX());
			TableOfVisibleTargets->ResetValue(v, 6, Targets[t]->GetTargetID());
			TableOfVisibleTargets->ResetValue(v, 7, Targets[t]->GetClassLabel());
			// end of N_COLS_TABLE_OF_VISIBLE_TARGETS
			v++;
		}

		// clean all other cells in the table:
		// for (unsigned int k = v + 1; k < N_Targets; k++) for (unsigned int c = 0; c < N_COLS_TABLE_OF_VISIBLE_TARGETS; c++) TableOfVisibleTargets->ResetValue(k, c, 0.0);
	}
	return v;
}



// Gets the frame duration in seconds
float CProcessRegister::GetFrameDurationInSeconds()
{
	return TimeFrameInSeconds;
}



// Gets a table of currently visible targets
CLargeDataMatrix<float>* CProcessRegister::GetTableOfVisibleTargets()
{
	return TableOfVisibleTargets;
}








//   ______________________________   private methods   ________________________________________________________

// Initializes internal structures
void CProcessRegister::Init()
{
	Targets = NULL;
	MainBuffer = NULL;
	pConfig = NULL;
	PhaseProc = NULL;
	Serv = NULL;
	TableOfVisibleTargets = NULL;
	DataVector = NULL;

	Ready = false;
	IsTargetsExternal = false;
	CurrFrameID = 0;
	N_Channels = 0;
	N_TimeFrames = 0;
	FrameSize = 0;
	N_Targets = 0;
	NoisePower = 1.0f;
	TimeFrameInSeconds = 0.0f;
	ChannelWidthInMeters = 0.0f;
	SoundVelocityInMetersPerSecond = 0.0f;
	SoundAmplitudeAttenuationCoef = 0.0f;
	EnablePhaseDistortions = false;
	EnablePhaseCorrection = false;
}


// Computes the distance in meters from the target the the channel
float CProcessRegister::ComputeDistanceInMeters(CSignalSource* Target, const unsigned long ChannelID)
{
	float X = Target->GetCurrPosX() - ChannelWidthInMeters * ChannelID;
	float Y = Target->GetCurrPosY();
	float D = sqrt(X * X + Y * Y);
	return D;
}


// Computes temporal delay in points in respect to the distance in meters
unsigned int CProcessRegister::ComputeDelayInPoints(float DistanceInMeters)
{
	float DelayInSeconds = DistanceInMeters / SoundVelocityInMetersPerSecond;
	float DelayInPoints = DelayInSeconds * ((float)FrameSize / TimeFrameInSeconds);
	return round(DelayInPoints);
}


// Computes coef of amplitude attenuation in respect to the distance in meters
float CProcessRegister::ComputeAmplitudeAttenuation(float DistanceInMeters)
{
	float coef = exp(-DistanceInMeters * SoundAmplitudeAttenuationCoef);
	return coef;
}






// Starts a separate process for computations
void CProcessRegister::StartComputations()
{
	CurrFrameID = 0;
	if (Serv == NULL)
	{
		printf("\nStart processing");
		for (unsigned long t = 0; t < N_TimeFrames; t++)
		{
			ProcessCurrFrame();
			IncTimeFrame();
		}
	}
	else
	{
		while (Serv->Status == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		for (unsigned long t = 0; t < N_TimeFrames; t++)
		{
			ProcessCurrFrame();
			Serv->CurrProcessedFrameID = t;
			unsigned int Mode = Serv->Mode;
			switch (Mode)
			{
			case 1:
			{
				CLargeDataVector<float>* pData = GetCurrentSpatialDistribution();
				Serv->N_TotalElementsToSend = N_Channels;
				Serv->pDataToSend = pData->GetInternalData();
			}
				break;

			case 2:
			{
				const float PowerCutoffThreshold = NoisePower * NOISE_ENERGY_MULTIPLIER_THRESHOLD;
				unsigned int N_VisibleTargets = GetNumberOfVisibleTargets(PowerCutoffThreshold);
				unsigned int N_Elements = N_VisibleTargets * N_COLS_TABLE_OF_VISIBLE_TARGETS;
				for (unsigned int r = 0; r < N_VisibleTargets; r++) for (unsigned int c = 0; c < N_COLS_TABLE_OF_VISIBLE_TARGETS; c++)
				{
					DataVector[r * N_COLS_TABLE_OF_VISIBLE_TARGETS + c] = TableOfVisibleTargets->GetValue(r, c);
				}
				Serv->N_TotalElementsToSend = N_Elements;
				Serv->pDataToSend = DataVector;

				/*
				if (N_VisibleTargets > 0)
				{
					printf("\nTable of visible targets:");
					TableOfVisibleTargets->PrintData(N_VisibleTargets);
				}
				*/
			}
				break;
			} // switch
			
			IncTimeFrame();

			if (Serv->Status == 0) break;
		}
		Serv->CurrProcessedFrameID = N_TimeFrames;
	}
	printf("\nEnd processing");
}



// Starts a separate process for data transfer
void CProcessRegister::StartServer()
{
	if (Serv != NULL) Serv->RunPortListening();
}




// Initializes constant parameters from the config file
bool CProcessRegister::Init_ScenarioGlobalParams(CConfigFileReader* pConfig, const char* SectionName)
{
	bool b;
	long a;
	float f;
	char name[1200];

	if (!pConfig->SearchSection(SectionName))
	{
		printf("\nError. Absent section =%s= in the config file. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}

	b = pConfig->ReadLong(SectionName, "N_FrameSize", &a);
	if (!b)
	{
		printf("\nError. Parameter =N_FrameSize= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (a < 512)
	{
		printf("\nError. Too small value of =N_FrameSize= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	FrameSize = a;

	b = pConfig->ReadLong(SectionName, "N_Channels", &a);
	if (!b)
	{
		printf("\nError. Parameter =N_Channels= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (a < 10)
	{
		printf("\nError. Too small value of =N_Channels= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	N_Channels = a;

	b = pConfig->ReadLong(SectionName, "N_TimeFrames", &a);
	if (!b)
	{
		printf("\nError. Parameter =N_TimeFrames= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (a < 10)
	{
		printf("\nError. Too small value of =N_TimeFrames= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	N_TimeFrames = a;

	b = pConfig->ReadFloat(SectionName, "float_NoisePower", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_NoisePower= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (f < 0.0f)
	{
		printf("\nError. Value of parameter =float_NoisePower= in section %s cannot be negative. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	NoisePower = f;

	b = pConfig->ReadLong(SectionName, "int_PulseRepetitionRateInHertz", &a);
	if (!b)
	{
		printf("\nError. Parameter =int_PulseRepetitionRateInHertz= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (a < 100)
	{
		printf("\nError. Too small value of =int_PulseRepetitionRateInHertz= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	f = (float)FrameSize / (float)a;
	TimeFrameInSeconds = f;

	b = pConfig->ReadFloat(SectionName, "float_ChannelWidthInMeters", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_ChannelWidthInMeters= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (f < 0.01)
	{
		printf("\nError. Too small value of =float_ChannelWidthInMeters= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	ChannelWidthInMeters = f;

	b = pConfig->ReadFloat(SectionName, "float_SoundVelocityInMetersPerSecond", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_SoundVelocityInMetersPerSecond= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (f < 100.0)
	{
		printf("\nError. Too small value of =float_SoundVelocityInMetersPerSecond= in section %s. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	SoundVelocityInMetersPerSecond = f;

	b = pConfig->ReadFloat(SectionName, "float_SoundAmplitudeAttenuationCoef", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_SoundAmplitudeAttenuationCoef= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}
	if (f < 0.0)
	{
		printf("\nError. Value of =float_SoundAmplitudeAttenuationCoef= cannot be negative. CProcessRegister::Init_ScenarioGlobalParams");
		return false;
	}
	SoundAmplitudeAttenuationCoef = f;

	b = pConfig->ReadLong(SectionName, "bool_EnablePhaseDistortions", &a);
	if (b)
	{
		b = (a != 0);
		EnablePhaseDistortions = b;
	}

	b = pConfig->ReadLong(SectionName, "bool_EnablePhaseCorrection", &a);
	if (b)
	{
		b = (a != 0);
		EnablePhaseCorrection = b;
	}

	PhaseProc = new CPhaseProc(FrameSize);
	if (!PhaseProc->IsReady())
	{
		printf("\nError. Object PhaseProc is not ready. CProcessRegister::Init_ScenarioGlobalParams");
		return false;
	}

	MainBuffer = new CSignalBuffer(N_Channels, N_TimeFrames, FrameSize);
	if (!MainBuffer->IsReady()) return false;

	b = pConfig->ReadName(SectionName, "NoiseSamplesFileName", name);
	if (!b)
	{
		printf("\nError. Parameter =NoiseSamplesFileName= in section %s is absent. CProcessRegister::Init_ScenarioGlobalParams", SectionName);
		return false;
	}

	b = MainBuffer->ReadFile_WithNoise(name);
	if (!b) return false;

	

	return true;
}


