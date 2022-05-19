
#include <cstdlib>
#include "ConfigStruct.h"





// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class AConfigStruct_Zone  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



// ______________________________   public methods   ________________________________________________________



// Constructor 1: Read parameters from a config file
AConfigStruct_Zone::AConfigStruct_Zone(CConfigFileReader * ConfigReader, const unsigned int ZoneID)
{
	this->ZoneID = ZoneID;
}




// Constructor 2: Create a new zone by explicitly assigning its main parameters
AConfigStruct_Zone::AConfigStruct_Zone(AConfigStruct_Zone * ZoneParams, const unsigned int ZoneID, const unsigned long StartChannel, const unsigned long EndChannel)
{
	this->ZoneID = ZoneID;
	ID_StartChannel = StartChannel;
	ID_EndChannel = EndChannel;
}





// Destructor
AConfigStruct_Zone::~AConfigStruct_Zone()
{
	// nothing
}





// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class AConfigStruct  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________



// Constructor 1: Creates the list of parameters by taking them from the given section in the config file
AConfigStruct::AConfigStruct(CConfigFileReader * ConfigReader, char * SectionName)
{
	N_Zones = 0;
	ZoneParams = NULL;

	long a;
	N_Cores = 1; // default
	bool b = ConfigReader->ReadLong(SectionName, "N_Cores", &a);
	if(b) N_Cores = a;
	
	N_Zones = 1; // default
	b = ConfigReader->ReadLong(SectionName, "N_Zones", &a);
	if(b) N_Zones = a;
	
	ArrayOfZonesInCores = NULL;
	TempConfigStruct = NULL;
	
}





// Constructor 2: Empty list of parameters
AConfigStruct::AConfigStruct(AConfigStruct * Source, unsigned int LimitNumberOfZones)
{
	N_Zones = 0;
	ZoneParams = NULL;
	N_Cores = Source->N_Cores;
	if((N_Cores < 1) || (N_Cores > 100)) N_Cores = 1;
	ArrayOfZonesInCores = NULL;
	TempConfigStruct = NULL;
}



// Destructor
AConfigStruct::~AConfigStruct()
{
	for(unsigned int i = 0; i < N_Zones; i++) delete ZoneParams[i];
	if(ZoneParams != NULL) free(ZoneParams);
	
	if(ArrayOfZonesInCores != NULL) free(ArrayOfZonesInCores);
	if(TempConfigStruct != NULL) delete TempConfigStruct;
}






// Creates a new spatial zone
void AConfigStruct::CreateNewZoneParams(AConfigStruct_Zone * OneZoneParams, const unsigned long StartChannel, const unsigned long EndChannel)
{
	this->ZoneParams[N_Zones] = new AConfigStruct_Zone(OneZoneParams, N_Zones, StartChannel, EndChannel);
	N_Zones++;
}




// Creates temporary structure of parameters after division on subzones
void AConfigStruct::CreateTempConfigStruct(const unsigned int NewNumberOfZones)
{
	TempConfigStruct = new AConfigStruct(this, NewNumberOfZones);
}




// Dispatch subzones between threads
void AConfigStruct::DispatchBetweenCores()
{
	CreateTempConfigStruct(N_Cores * N_Zones);
	ArrayOfZonesInCores = (unsigned int * ) calloc(N_Cores * N_Zones, sizeof(unsigned int));
	
	unsigned long N_ChannelsToProcess = 0;
	for(unsigned int z = 0; z < N_Zones; z++)
	{
		N_ChannelsToProcess += (ZoneParams[z]->ID_EndChannel + 1 - ZoneParams[z]->ID_StartChannel);
	}
	unsigned long Delta = N_ChannelsToProcess / N_Cores;
	printf("\nChannels to process: %ld . Delta=%ld", N_ChannelsToProcess, Delta);
	
	unsigned int ZoneID = 0;
	unsigned int CoreID = 0;
	unsigned int SubZoneID = 0;
	unsigned long N_CoreChannels = 0;
	unsigned long SubZoneStartChannel = ZoneParams[0]->ID_StartChannel;
	while(true)
	{
		AConfigStruct_Zone * OneZoneParams = ZoneParams[ZoneID];
		unsigned long SubZoneEndChannel = OneZoneParams->ID_EndChannel;
		unsigned long ZoneSize = SubZoneEndChannel + 1 - SubZoneStartChannel;
		if(ZoneSize <= Delta)
		{
			TempConfigStruct->CreateNewZoneParams(OneZoneParams, SubZoneStartChannel, SubZoneEndChannel);
			ArrayOfZonesInCores[TempConfigStruct->N_Zones - 1] = CoreID;
			SubZoneID++;
			ZoneID++;
			if(ZoneID >= N_Zones) break;
			N_CoreChannels += ZoneSize;
			SubZoneStartChannel = (ZoneParams[ZoneID])->ID_StartChannel;
			if(ZoneSize == Delta)
			{
				N_CoreChannels = 0;
				CoreID++;
				if(CoreID >= N_Cores) break;
				SubZoneID = 0;
			}
		}
		else
		{
			SubZoneEndChannel = SubZoneStartChannel + Delta - N_CoreChannels - 1;
			TempConfigStruct->CreateNewZoneParams(OneZoneParams, SubZoneStartChannel, SubZoneEndChannel);
			ArrayOfZonesInCores[TempConfigStruct->N_Zones - 1] = CoreID;
			CoreID++;
			if(CoreID >= N_Cores) break;
			N_CoreChannels = 0;
			SubZoneID = 0;
			SubZoneStartChannel = SubZoneEndChannel + 1;
		}
	}
	
	(TempConfigStruct->ZoneParams[TempConfigStruct->N_Zones - 1])->ID_EndChannel = (ZoneParams[N_Zones - 1])->ID_EndChannel;
	
	printf("\nOld zones:");
	for(unsigned int k = 0; k < N_Zones; k++)
	{
		AConfigStruct_Zone * OneZoneParams = ZoneParams[k];
		printf("\n %d: [%ld, %ld].", k, OneZoneParams->ID_StartChannel, OneZoneParams->ID_EndChannel);
	}
	printf("\n");
	
	printf("\nNew zones:");
	for(unsigned int k = 0; k < TempConfigStruct->N_Zones; k++)
	{
		AConfigStruct_Zone * OneZoneParams = TempConfigStruct->ZoneParams[k];
		printf("\n %d: [%ld, %ld]. Core=%d", k, OneZoneParams->ID_StartChannel, OneZoneParams->ID_EndChannel, ArrayOfZonesInCores[k]);
	}
	printf("\n");
}






