#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_



#include <cstdio>
#include "ConfigFileWriter.h"

// The class provides a common interface to get access to parameters stored in the configuration file.
// It is supposed that the configuration file consists of independent sections. Each section may contain
// one or several spatial zones. Parameters inside the section can be defined either for all zones or only
// for one zone. Each zone has parameters of start and end channels. 
// Parameters of the zones are numbered in ascending mode starting from 0; for example, ID_StartChannel[0].
// Common parameters are not numbered. 
// The number of zones must be pointed out for each section supporting spatial processing.
// If a section relates to a multi-threading algorithm, then the number of cores should be explicitly written done.
// Comments follow after the sign "//".
// Note: spatial zones should not overlap, but gaps between them are allowable. 
// If the end channel is unknown in advance, it can be set to 0. during the program execution, the real number of channels
// is substituted (fused or raw channels, depending on the context).
// Each section should start with the keyword "Section". 
// To get access to a zone parameter, please include ZoneID; otherwise a common parameter will be extracted.
// There are three types of parameters: long integer, float, and string (file name). 
// The latter one should be bracketed by "".


class CConfigFileReader
{
private:
	char ConfigFileName[1200]; // Filename of the configuration file
	FILE * pFile; // Pointer to the corresponded file 
	bool Ready;	// Flag: is the file ready
	bool IsFileExternal;  // Flag: whether the config file is opened inside the class or it is external.
	
	bool GetCurrentLine(char ** Line, const unsigned int StrSize = 1200); // Get one line in the config file without comments. Returns the flag of success
	bool GetCurrentLineWithComments(char ** Line, const unsigned int StrSize = 1200); // Get one line in the config file with comments. Returns the flag of success

	bool ReadArrayOfStrings(const char * SectionName, const char * ParamName, unsigned int * N_Strings, char *** OutStrings); // NOT OPERATIONAL YET
	bool ReadArrayOfLongs(const char * SectionName, const char * ParamName, unsigned int * N_Values, long ** OutValues);  // NOT OPERATIONAL YET
	bool ReadArrayOfFloats(const char * SectionName, const char * ParamName, unsigned int * N_Values, float ** OutValues);  // NOT OPERATIONAL YET

public:
	CConfigFileReader(const char * FileName); // Constructor 1 (opens the file)
	CConfigFileReader(FILE * pConfigFile); // Constructor 2 (gets the opened file)
	CConfigFileReader(CConfigFileWriter * ConfigWriter); // Constructor 3 (ready object of ConfigFileWriter)
	~CConfigFileReader(); // Destructor
	
	void PrintOutContent(); // Prints out the content

	// Properties
	bool IsFileReady(); // Gets the flag: whether the config file is ready
	bool SearchSection(const char * SectionName); // Puts the file pointer to the beginning of the chosen section 
	// bool IsSectionEnabled(const char * SectionName); // Gets the flag: whether the section is enabled or not
	bool ReadStr(const char * SectionName, const char * ParamName, char ** OutString); // Gets a common parameter of string type 
	bool ReadName(const char * SectionName, const char * ParamName, char * OutString); // Gets a common parameter of a file name
	bool ReadLong(const char * SectionName, const char * ParamName, long * OutValue); // Gets a common parameter of long int type
	bool ReadFloat(const char * SectionName, const char * ParamName, float * OutValue); // Gets a common parameter of float-point type

	unsigned int GetNumberOfZones(const char * SectionName); // Gets the number of spatial zones corresponded within the given section
	bool ReadStr(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char ** OutString); // Gets a zone parameter of string type
	bool ReadName(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char * OutString); // Gets a zone parameter of a file name
	bool ReadLong(const char * SectionName, const char * ParamName, const unsigned int ZoneID, long * OutValue); // Gets a zone parameter of long type
	bool ReadFloat(const char * SectionName, const char * ParamName, const unsigned int ZoneID, float * OutValue); // Gets a zone parameter of float-point type

	long ReadLong_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const long DefaultValue); // Gets a zone parameter of long type. If it is absent, the common parameter havig the same name is extracted. If it fails, the default value is the output. 
	float ReadFloat_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const float DefaultValue); // Gets a zone parameter of float type. If it is absent, the common parameter havig the same name is extracted. If it fails, the default value is the output.
	void ReadString_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char ** OutString); // Gets a zone parameter of string type. If it is absent, the common parameter havig the same name is extracted. 
	void ReadName_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char * OutString); // Gets a zone parameter of the file name. If it is absent, the common parameter havig the same name is extracted.
	
	unsigned long GetData(const char * SectionName, fpos_t * pos); // Sets the file pointer to the beginning of the data insertion in the given section. Output = number of lines in the data insertion. 
	FILE * GetFile(); // Gets a pointer to the config file
	char * GetFileName(); // Gets the name of the config file
	bool CopySectionToConfigWriter(const char * OriginalSectionName, const char * NewSectionName, CConfigFileWriter * ConfigWriter); // Copies a section to a new config file
};



#endif /*CONFIGFILEREADER_H_*/

