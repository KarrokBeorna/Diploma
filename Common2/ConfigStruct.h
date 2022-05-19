#ifndef CONFIGSTRUCT_H_
#define CONFIGSTRUCT_H_


#include "ConfigFileReader.h"


// This abstract class represents main parameters of a spatial zone. The descendants expand the list of parameters. 


class AConfigStruct_Zone
{
public:	
	unsigned int ZoneID; // ID of the zone within one section
	long ID_StartChannel; // ID of the zone's start channel (0 by default)
	long ID_EndChannel; // ID of the zone's end channel (end channel in the binary file by default)

	AConfigStruct_Zone(CConfigFileReader * ConfigReader, const unsigned int ZoneID); // Constructor 1: Read parameters from a config file
	AConfigStruct_Zone(AConfigStruct_Zone * ZoneParams, const unsigned int ZoneID, const unsigned long StartChannel, const unsigned long EndChannel); // Constructor 2: Create a new zone by explicitly assigning its main parameters
	virtual ~AConfigStruct_Zone(); // Destructor
};




// This abstract class represent a structure of parameters related to one section (i.e. one task of processing).
// The descendants expand the list of parameters

class AConfigStruct
{	
public:	
	unsigned int N_Zones; // Number of zones in the section (0 by default, if there is no explicitly defined zones)
	unsigned int N_Cores; // Number of threads to process the task (may not be the same as for other tasks)
	
	AConfigStruct_Zone ** ZoneParams; // Array of zone parameters

	AConfigStruct * TempConfigStruct; // Temporary structure of parameters used only when threads are dispatched among subzones
	unsigned int * ArrayOfZonesInCores; // Array of id for subzones assigned to threads
	
	AConfigStruct(CConfigFileReader * ConfigReader, char * SectionName); // Constructor 1: Creates the list of parameters by taking them from the given section in the config file 
	AConfigStruct(AConfigStruct * Source, unsigned int LimitNumberOfZones); // Constructor 2: Empty list of parameters
	virtual ~AConfigStruct(); // Destructor
	
	virtual void CreateNewZoneParams(AConfigStruct_Zone * OneZoneParams, const unsigned long StartChannel, const unsigned long EndChannel); // Creates a new spatial zone 
	virtual void CreateTempConfigStruct(const unsigned int NewNumberOfZones); // Creates temporary structure of parameters after division on subzones
	virtual void DispatchBetweenCores(); // Dispatch subzones between threads
};





#endif /*CONFIGSTRUCT_H_*/
