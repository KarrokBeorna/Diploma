/*
* Main.cpp
*
*  Created on: Jan 11, 2022
*      Author: Artem Khlybov & Dmitriy Savelev
*/



#include <cstdio>
#include "TestScripts.h"


int main(int argc, char** argv)
{
/*
#if defined(_WIN32)
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
#endif
*/
	int out = 0;


	// SCRIPT_Test2();   return 0;
	// SCRIPT_Test1();   return 0;

	if (argc <= 1) out = SCRIPT_ConfigRun0();
	else out = SCRIPT_ConfigRun0(argv[1]);


	if (out == 0) printf("\n\n_completed successfully_____");
	else  printf("\n\n_completed with error %d_____", out);
	printf("\n");
	// Sleep(1000); // #include <windows.h>
	return out;
}

