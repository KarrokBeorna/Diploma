/*
* TestScripts.h
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov
*/


#ifndef TESTCRIPTS_SIGNALEMULATOR_H_
#define TESTCRIPTS_SIGNALEMULATOR_H_

#include <cstdio>
#include "Configs/ConfigFileReader.h"
#include "FunctionTemplates.h"
#include "ProcessRegister/ProcessRegister.h"


int SCRIPT_ConfigRun0(char* ConfigFileName)
{
	CConfigFileReader* Config = new CConfigFileReader(ConfigFileName);
	if (!Config->IsFileReady())
	{
		printf("\nError. Config structure is not ready.");
		delete Config;
		return 1;
	}

	bool b;
	char name[1200];
	b = Config->ReadName("CommonParameters", "Description", name);
	if (!b)
	{
		printf("\nError. Parameter ==Description== in section CommonParameters is absent.");
		delete Config;
		return 11;
	}

	if (strstr(name, "TargetTrackingScenario") == NULL)
	{
		printf("\nError. Parameter =Description= in section CommonParameters should be TargetTrackingScenario.");
		delete Config;
		return 12;
	}

	CProcessRegister* A = new CProcessRegister(Config);
	if (!A->IsReady())
	{
		printf("\nError. Object of CProcessRegister is not ready. Halted");
		delete A;
		delete Config;
		return 13;
	}
	
	A->Execute();
	A->PrepareOutFiles();

	delete A;
	delete Config;
	return 0;
}



int SCRIPT_ConfigRun0()
{
	char tmpstring[1200] = "Config TTScenario_0.txt";
	// char tmpstring[1200] = "F:/WORK/Workspaces1/SignalEmulator/Configs/Config TTScenario_0.txt";
	char* ptmp = tmpstring;
	return SCRIPT_ConfigRun0(ptmp);
}






#endif /*TESTCRIPTS_SIGNALEMULATOR_H_*/


