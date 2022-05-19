/*
* TargetClassParamSettings.cpp
*
*  Created on: Mar 18, 2022
*      Author: Artem Khlybov
*/


#include "../dirent/dirent.h"
#include "TargetClassParamSettings.h"

#define NMAX_SIGNATURES 100



// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CClassTargetParamRange  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________


// Constructor
CClassTargetParamRange::CClassTargetParamRange(const unsigned int ClassLabel, CConfigFileReader* Config)
{
	this->ClassLabel = ClassLabel;
	Ready = false;
	N_Targets = 0;
	N_Signatures = 0;
	MaxInitVelocityX = 0.0;
	MinInitVelocityX = 0.0;
	MaxInitPosY = 0.0;
	MinInitPosY = 0.0;
	MaxTargetSignalPower = 0.0;
	MinTargetSignalPower = 0.0;
	Signatures = NULL;

	bool b;
	long a;
	float f;
	char SectionName[100];
	char name[1200];

	sprintf(SectionName, "TargetClass[%d]", N_Targets);
	b = Config->SearchSection(SectionName);
	if (!b)
	{
		printf("\nError. Absent section %s in the config file. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}

	// N_Targets
	b = Config->ReadLong(SectionName, "N_Targets", &a);
	if (!b)
	{
		printf("\nError. Parameter =N_Targets= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (a < 1)
	{
		printf("\nError. Parameter =N_Targets= in section %s cannot be zero or negative. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	N_Targets = a;

	// float_MaxInitPosY
	b = Config->ReadFloat(SectionName, "float_MaxInitPosY", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_MaxInitPosY= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =float_MaxInitPosY= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MaxInitPosY = f;

	// float_MinInitPosY
	b = Config->ReadFloat(SectionName, "float_MinInitPosY", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_MinInitPosY= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =float_MinInitPosY= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f > MaxInitPosY)
	{
		printf("\nError. Value of =float_MinInitPosY= in section %s cannot be more than the value of =float_MaxInitPosY=. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MinInitPosY = f;

	// MaxInitVelocityX
	b = Config->ReadFloat(SectionName, "float_MaxInitVelocityX", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_MaxInitVelocityX= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =float_MaxInitVelocityX= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MaxInitVelocityX = f;

	// float_MinInitVelocityX
	b = Config->ReadFloat(SectionName, "float_MinInitVelocityX", &f);
	if (!b)
	{
		printf("\nError. Parameter =MinInitVelocityX= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =MinInitVelocityX= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f > MaxInitVelocityX)
	{
		printf("\nError. Value of =MinInitVelocityX= in section %s cannot be more than the value of =MaxInitVelocityX=. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MinInitVelocityX = f;

	// float_MaxTargetSignalPower
	b = Config->ReadFloat(SectionName, "float_MaxTargetSignalPower", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_MaxTargetSignalPower= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =float_MaxTargetSignalPower= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MaxTargetSignalPower = f;

	// float_MinTargetSignalPower
	b = Config->ReadFloat(SectionName, "float_MinTargetSignalPower", &f);
	if (!b)
	{
		printf("\nError. Parameter =float_MinTargetSignalPower= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f <= 0.0)
	{
		printf("\nError. Value of =float_MinTargetSignalPower= in section %s should be positive. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	if (f > MaxTargetSignalPower)
	{
		printf("\nError. Value of =float_MinTargetSignalPower= in section %s cannot be more than the value of =float_MaxTargetSignalPower=. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	MinTargetSignalPower = f;

	// Signatures
	N_Signatures = 0;
	char LibFileName[1200] = "";
	char FolderName[1200] = "";
	Signatures = (CLargeDataVector<float> **)calloc(NMAX_SIGNATURES, sizeof(CLargeDataVector<float>*));
	for (unsigned int i = 0; i < NMAX_SIGNATURES; i++) Signatures[i] = NULL;
	b = Config->ReadName(SectionName, "SignalSignatureFolder", FolderName);
	if (!b)
	{
		printf("\nError. Parameter =SignalSignatureFolder= in section %s is absent. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	unsigned int count_files = 0;
	DIR* d = opendir(FolderName);
	if (!d)
	{
		printf("\nError. Wrong path to the folder, section %s, parameter =SignalSignatureFolder=. CClassTargetParamRange::CClassTargetParamRange", SectionName);
		return;
	}
	else
	{
		struct dirent* dir;
		while ((dir = readdir(d)) != NULL)
		{
			char* dname = dir->d_name;
			unsigned int len = strlen(dname);
			if (len < 5) continue;
			if ((dname[len - 1] == 't') && (dname[len - 2] == 'x') && (dname[len - 3] == 't') && (dname[len - 4] == '.'))
			{
				unsigned int ll = strlen(FolderName);
				if (ll == 0) strcpy(LibFileName, dname); // sprintf(LibFileName, "%s", d);
				else
				{
					if (FolderName[ll - 1] == '/') sprintf(LibFileName, "%s%s", FolderName, dname);
					else sprintf(LibFileName, "%s/%s", FolderName, dname);
				}

				FILE* tmpFile = fopen(LibFileName, "r");
				if (tmpFile == NULL)
				{
					printf("\nError. Cannot open file %s. CClassTargetParamRange::CClassTargetParamRange", LibFileName);
					return;
				}
				Signatures[N_Signatures] = new CLargeDataVector<float>(tmpFile);
				fclose(tmpFile);
				if (!Signatures[N_Signatures]->IsDataReady())
				{
					printf("\nError. File %s cannot be converted to a vector. CClassTargetParamRange::CClassTargetParamRange", LibFileName);
					closedir(d);
					return;
				}
				N_Signatures++;
				if (N_Signatures == NMAX_SIGNATURES) break; // while
			}
		} // end while ((dir = readdir(d)) != NULL)
		closedir(d);
	}

	Ready = true;
}




// Destructor
CClassTargetParamRange::~CClassTargetParamRange()
{
	if (Signatures != NULL)
	{
		for (unsigned int i = 0; i < NMAX_SIGNATURES; i++) if (Signatures[i] != NULL) delete Signatures[i];
		free(Signatures);
	}
}



//   ______________________________   Access to properties   ________________________________________________________



// Gets the flag of readiness
bool CClassTargetParamRange::IsReady()
{
	return Ready;
}