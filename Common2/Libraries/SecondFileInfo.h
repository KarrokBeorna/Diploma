#ifndef SECONDFILEINFO_H_
#define SECONDFILEINFO_H_


#include <iostream>
#include <fstream>
#include <string>

#include "GeneralSettings.h"


// These two methods read a text file of data and return the numbers of columns and rows.

bool GetFileInfo1(const char * FileName, unsigned long * Lines, unsigned int * Columns, bool CheckColumns = true); // Reads a line of data using getline()
bool GetFileInfo2(FILE * pFile, unsigned long * Lines, unsigned int * Columns, bool CheckColumns = true); // Reads a line of data using fgets()


#endif /*SECONDFILEINFO_H_*/
