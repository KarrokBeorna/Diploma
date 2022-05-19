/*
* SignalSource.h
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/


#ifndef SIGNALSOURCE_SIGNALEMULATOR_H_
#define SIGNALSOURCE_SIGNALEMULATOR_H_


#include "Configs/ConfigFileReader.h"
#include "VectorMatrix/LargeDataVector.h"



// The class represents an emulated target (or a signal source in general), which is described by unique kinematic parameters.
// X and Y coordinates - in meters;
// VelocityX and VelocityY - in meters per time frame
class CSignalSource
{
private:
	bool Ready;	// Flag of readiness
	bool IsSignatureExternal; // Flag: whether the signal signature is an external (true) or internal (false) object
	unsigned int TargetID; // Unique ID of the target
	int ClassLabel; // Class label
	unsigned int FrameSize; // Frame size = data cell size
	float CurrPosX;		// current longitudinal coordinate X in meters
	float CurrPosY;		// current transverse coordinate Y in meters
	float CurrVelocityX;	// Current longitudinal velocity along X in meters per frame
	float CurrVelocityY;	// Current tansverce velocity along Y in meters per frame
	float CurrTargetSignalEnergy;			// Current source signal energy
	float TimeFrameDurationInSeconds; // Duration of time frame in seconds
	unsigned long CurrTargetSignalPos; // Current index in the signal array corresponding to temporal delay
	CLargeDataVector<float> * TargetSignal;					// Normalized array of sound recording of the signal source
	float * TmpDataCell; // Array of a tmp data cell to copy the signal

	void Init(); // Initializes internal structures

public:
	CSignalSource(const unsigned int TargetID, CConfigFileReader* Config); // Constructor 1 - Config "TargetTrackingScenario" 
	CSignalSource(const unsigned int TargetID); // Constructor 2 - Empty
	~CSignalSource(); // Destructor
	void IncTimeFrame(); // Updates all parameters for the next time frame

	// Access to properties
	bool IsReady(); // Gets the flag of readiness
	float GetCurrPosX(); // Gets current position X in meters
	void SetCurrPosX(const float X); // Sets current position X in meters
	float GetCurrPosY();  // Gets current position Y
	void SetCurrPosY(const float Y); // Sets current position Y in meters
	float GetCurrVelocityY();  // Gets current velocity Y in meters per frame
	void SetCurrVelocityY(const float Vy); // Sets current velocity Y in meters per second
	float GetCurrVelocityX();  // Gets current velocity X in meters per second
	void SetCurrVelocityX(const float Vx); // Sets current velocity X in meters per second
	float* GetSignalFrame(); // Gets an array of size = data cell filled with the signal content
	void SetCurrSignalEnergy(const float SignalEnergy); // Sets current signal energy  (variance)
	float GetCurrSignalEnergy(); // Gets current signal energy (variance)
	void ResetSignature(CLargeDataVector<float>* NewSignature); // Resets internal signature to a new vector
	void SetClassLabel(const int ClassLabel); // Sets the class label
	int GetClassLabel(); // Gets the class label
	unsigned int GetTargetID(); // Gets the Target ID
	void SetFrameSize(const unsigned int FrameSize); // Sets the frame size
	float GetFrameDurationInSeconds(); // Gets the frame duration in seconds
};






#endif /*SIGNALSOURCE_SIGNALEMULATOR_H_*/



