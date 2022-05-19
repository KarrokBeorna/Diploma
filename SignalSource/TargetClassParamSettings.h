/*
* TargetClassParamSettings.h
*
*  Created on: Mar 18, 2022
*      Author: Artem Khlybov
*/


#ifndef TARGETCLASSPARAMSETTINGS_SIGNALEMULATOR_H_
#define TARGETCLASSPARAMSETTINGS_SIGNALEMULATOR_H_

#include "VectorMatrix/LargeDataVector.h"
#include "Configs/ConfigFileReader.h"


// The class contains limits and settings for a particular class of targets. All info is provided by a
// config file of type "SimulationPlan_TargetTracking"
class CClassTargetParamRange
{
public:
	bool Ready; // Flag of readiness
	int ClassLabel; // Class label
	unsigned int N_Targets; // Number of targets in this class
	unsigned int N_Signatures; // Number of available signal signatures
	float MaxInitVelocityX; // Max abs init velocity X (along the cable)
	float MinInitVelocityX; // Min abs init velocity X (along the cable)
	float MaxInitPosY; // Max distance Y (transverse to the cable) from the target to the cable 
	float MinInitPosY; // Min distance Y (transverse to the cable) from the target to the cable 
	float MaxTargetSignalPower; // Max signal power
	float MinTargetSignalPower; // Min signal power
	CLargeDataVector<float>** Signatures; // Array of available signal signatures

	CClassTargetParamRange(const unsigned int ClassLabel, CConfigFileReader* Config); // Constructor
	~CClassTargetParamRange(); // Destructor

	// Properties
	bool IsReady(); // Gets the flag of readiness
};







#endif /*TARGETCLASSPARAMSETTINGS_SIGNALEMULATOR_H_*/
