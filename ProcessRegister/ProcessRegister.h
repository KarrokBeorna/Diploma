/*
* ProcessRegister.h
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/


#ifndef PROCESSREGISTER_SIGNALEMULATOR_H_
#define PROCESSREGISTER_SIGNALEMULATOR_H_

#include "Configs/ConfigFileReader.h"
#include "../SignalSource/SignalSource.h"
#include "../SignalBuffer/SignalBuffer.h"
#include "../PhaseProcessing/PhaseProc.h"
#include "../Server/Server3.h"


// The class represents a register of targets. It performs main operations regarding to filling the signal buffer with target signals
// and background noise.
class CProcessRegister
{
private:
	bool Ready;	// Flag of readiness
	bool IsTargetsExternal; //Flag: whether the targets are external (true) or internal (false)
	unsigned long N_Channels;				// Number of channels
	unsigned long N_TimeFrames;				// Number of time frames
	unsigned int FrameSize;					// Frame size = cell size (2^n)
	unsigned int CurrFrameID; // ID of the currently processed time frame

	float TimeFrameInSeconds;				// Duration of one frame in seconds
	float ChannelWidthInMeters;				// Spatial width of one channel in meters
	float SoundVelocityInMetersPerSecond;	// Velocity of sound in meters per second
	float SoundAmplitudeAttenuationCoef;	// Sound amplitude attenuation coef in 1/m
	float NoisePower;	// Noise variance in one data cell
	bool EnablePhaseDistortions; // Flag enabling phase distortions in the signal
	bool EnablePhaseCorrection; // Flag enabling correction procedure to suppress phase distortions
	CConfigFileReader* pConfig; // Reference to the external config
	unsigned int N_Targets; // Number of targets 
	CSignalSource** Targets; // Array of targets
	CSignalBuffer* MainBuffer; // Main data buffer
	CLargeDataMatrix<float>* TableOfVisibleTargets; // Table of currently visible targets
	CPhaseProc* PhaseProc; // Phase processing operations
	CServer3* Serv; // TCP server
	float* DataVector;
	
	void Init(); // Initializes internal structures
	float ComputeDistanceInMeters(CSignalSource * Target, const unsigned long ChannelID); // Computes the distance in meters from the target the the channel
	unsigned int ComputeDelayInPoints(float DistanceInMeters); // Computes temporal delay in points in respect to the distance in meters
	float ComputeAmplitudeAttenuation(float DistanceInMeters); // Computes coef of amplitude attenuation in respect to the distance in meters
	void StartComputations(); // Starts a separate process for computations
	void StartServer(); // Starts a separate process for data transfer
	bool Init_ScenarioGlobalParams(CConfigFileReader* pConfig, const char* SectionName); // Initializes constant parameters from the config file

public:
	CProcessRegister(CConfigFileReader* Config); // Constructor 1, config = "TargetTrackingScenario"
	CProcessRegister(CConfigFileReader* Config, const char * SectionName); // Constructor 2, config = "SimulationPlan_TargetTracking"
	~CProcessRegister(); // Destructor
	// void PreparationBeforeLaunch(); // Preparations with internal data without memory allocation
	void Execute(); // Main method
	void ProcessCurrFrame(); // Processes current time frame
	void PrepareOutFiles(); // Creates output data files
	void CleanMainBuffer(); // Cleans main internal buffer
	bool CreateGUIMapTxtFile(const char* GUIMapFileName); // Creates a spatio-temporal map of aggregated values in a form of a text file for the GUI application
	void IncTimeFrame(); // Increments time frame

	// Access to properties
	bool IsReady(); // Gets the flag of readiness
	CSignalBuffer* GetMainBuffer(); // Gets the main buffer of signals
	CSignalSource** GetTargets(); // Gets the array of actual targets at the current state
	unsigned int GetNumberOfTargets(); // Gets the number of targets
	unsigned int GetNumberOfTimeFrames(); // Gets the number of time frames
	unsigned int GetNumberOfChannels(); // Gets the number of spatial channels
	void ResetTargets(const unsigned int N_Targets, CSignalSource** Targets); // Resets internal targets
	unsigned int GetCurrTimeFrame(); // Gets ID of the current time frame
	unsigned int GetFrameSize(); // Gets the frame size
	float GetNoisePower(); // Gets noise power
	float GetSoundVelocityInMetersPerSecond(); // Gets sound velocity
	float GetSoundAmplitudeAttenuationCoef(); // Gets amplitude atenuation coef
	float GetChannelWidthInMeters(); // Gets channel width in meters
	CLargeDataVector<float>* GetCurrentSpatialDistribution(); // Gets a table of currently visible targets for the given Power Cutoff Threshold
	CLargeDataMatrix<float>* GetTableOfVisibleTargets(); // Gets a table of currently visible targets
	unsigned int GetNumberOfVisibleTargets(const float PowerCutoffThreshold); // Gets the number of currently visible targets for the given Power Cutoff Threshold
	float GetFrameDurationInSeconds(); // Gets the frame duration in seconds
};





#endif /*PROCESSREGISTER_SIGNALEMULATOR_H_*/



