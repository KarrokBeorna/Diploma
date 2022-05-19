
#include <cstring>
#include <cstdlib>
#include "ConfigFileReader.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CConfigFileReader  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________



// Constructor 1 (opens the file)
CConfigFileReader::CConfigFileReader(const char * FileName)
{
	Ready = false;
	strcpy(this->ConfigFileName, FileName);
	pFile = fopen(ConfigFileName, "r"); // pFile = fopen( ConfigFileName , "rb" ); // TODO: Check correctness. "rb" is not appropriate when a file section is read; returned back to "r" 
	if(pFile == NULL)
	{
		printf("\nIniFile %s doesn't exist.", FileName);
		return;
	}
	Ready = true;
	IsFileExternal = false;
}





// Constructor 2 (gets the opened file)
CConfigFileReader::CConfigFileReader(FILE * pConfigFile)
{
	Ready = false;
	this->pFile = pConfigFile;
	if(pFile == NULL)
	{
		printf("\nIniFile doesn't exist.");
		return;
	}
	Ready = true;
	IsFileExternal = true;
}




// Constructor 3 (ready object of ConfigFileWriter)
CConfigFileReader::CConfigFileReader(CConfigFileWriter * ConfigWriter)
{
	pFile = NULL;
	strcpy(ConfigFileName, "");
	Ready = ConfigWriter->IsReady();
	if(!Ready)
	{
		printf("\nInput structure of CConfigFileWriter is not ready.");
		return;
	}
	pFile = tmpfile();
	ConfigWriter->SaveToFile(pFile);

	if(pFile == NULL)
	{
		printf("\nInput structure of CConfigFileWriter cannot be saved in a temporary file.");
		Ready = false;
		return;
	}
	rewind(pFile);
	IsFileExternal = false;
}






// Destructor
CConfigFileReader::~CConfigFileReader()
{
	if((Ready)&&(!IsFileExternal)) fclose(pFile);
}



// Prints out the content
void CConfigFileReader::PrintOutContent()
{
	if(pFile == NULL) return;
	rewind(pFile);
	char Str[10000];
	char * pStr = Str;

	unsigned long count = 0;
	while(!feof(pFile))
	{
		fgets(Str, 10000, pFile);
		count++;
	}
	count--;
	rewind(pFile);

	for(unsigned long i = 0; i < count; i++)
	{
		GetCurrentLineWithComments(&pStr, 10000);
		printf("\n%s", Str);
	}
	printf("\n");
}


//   ______________________________   Properties   ________________________________________________________



// Gets the flag: whether the config file is ready
bool CConfigFileReader::IsFileReady()
{
	return Ready;
}



// Puts the file pointer to the beginning of the chosen section
bool CConfigFileReader::SearchSection(const char * SectionName)
{
	char String[1200];
	char * pString = String;
	char SearchString[1200];
	sprintf(SearchString, "Section %s", SectionName);
	rewind(pFile);

	while(!feof(pFile))
	{
		GetCurrentLine(&pString);
		if(strlen(pString) == 0) continue;
		if(strstr(String, SearchString) == NULL) continue;
		else break;
	}
	if(feof(pFile)) return false;
	else return true;
}



// Gets a common parameter of string type
bool CConfigFileReader::ReadStr(const char * SectionName, const char * ParamName, char ** OutString)
{
	char SearchString[1200];
	char * pString = SearchString;
	SearchSection(SectionName);
	// char StopString[] = "EndSection"; 
	char StopString[] = "Section";
	bool flag = true;
	
	while(flag)
	{
		this->GetCurrentLine(&pString);
		if(strstr(SearchString, ParamName) != NULL)
		{
			char FalseString[1200];
			sprintf(FalseString, "%s[", ParamName);
			if(strstr(SearchString, FalseString) != NULL) continue;
			else break;
		}
		if(strstr(SearchString, StopString) != NULL) flag = false;
		if(feof(pFile)) flag = false;
	}
	if(flag == false)
	{
		(*OutString)[0] = '\0';
		return false;
	}
	
	char * pch = strrchr(SearchString, '=');
	strcpy((*OutString), pch + 1);
	return true;
}





// Gets a common parameter of a file name
bool CConfigFileReader::ReadName(const char * SectionName, const char * ParamName, char * OutString)
{
	char SearchString[1200];
	char * pString = SearchString;
	SearchSection(SectionName);
	// char StopString[] = "EndSection"; 
	char StopString[] = "Section";
	bool flag = true;
	
	while(flag)
	{
		this->GetCurrentLine(&pString);
		if(strstr(SearchString, ParamName) != NULL) break;
		if(strstr(SearchString, StopString) != NULL) flag = false;
		if(feof(pFile)) flag = false;
	}
	if(flag == false)
	{
		OutString[0] = '\0';
		return false;
	}
	
	char delim = '"';
	char * pch = NULL;
	for(unsigned int k = 0; k < strlen(SearchString); k++) if(SearchString[k] == delim)
	{
		pch = &SearchString[k + 1];
		break;
	}
	if(pch == NULL)
	{
		OutString[0] = '\0';
		return false;
	}
	
	bool b = true;
	unsigned int k = 0;
	while(b)
	{
		char tmp = pch[k];
		if((tmp == delim) || (tmp == '\n') || (tmp == '\0'))
		{
			OutString[k] = '\0';
			break;
		}
		OutString[k] = tmp;
		k++;
	}

	return true;
}





// Gets a common parameter of long int type
bool CConfigFileReader::ReadLong(const char * SectionName, const char * ParamName, long * OutValue)
{
	char SearchString[1200];
	char * pString = SearchString;
	bool Res = ReadStr(SectionName, ParamName, &pString);
	if(!Res) return false;	
	long value = 0;
	try
	{
		sscanf(SearchString, "%ld", &value);
	}
	catch(...)
	{
		Res = false;
	}
	*OutValue = value;
	return Res;
}




// Gets a common parameter of float-point type
bool CConfigFileReader::ReadFloat(const char * SectionName, const char * ParamName, float * OutValue)
{
	char SearchString[1200];
	char * pString = SearchString;
	bool Res = ReadStr(SectionName, ParamName, &pString);
	if(!Res) return false;
	float value = 0.0;
	try
	{
		sscanf(SearchString, "%f", &value);
	}
	catch(...)
	{
		Res = false;
	}
	*OutValue = value;
	return Res;
}





// Gets the number of spatial zones corresponded within the given section
unsigned int CConfigFileReader::GetNumberOfZones(const char * SectionName)
{
	unsigned int res = 1;
	long value; 
	bool b = ReadLong(SectionName, "N_Zones", &value);
	if(value < 0) value = 1;
	if(!b) value = 1;
	res = value;
	return res;
}





// Gets a zone parameter of string type
bool CConfigFileReader::ReadStr(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char ** OutString)
{
	char SearchString[1200];
	sprintf(SearchString, "%d", ZoneID);
	
	char NewParamName[1200];
	strcpy(NewParamName, ParamName);
	strcat(NewParamName, "[");
	strcat(NewParamName, SearchString);
	strcat(NewParamName, "]");
	
	char * pString = SearchString;
	SearchSection(SectionName);
	// char StopString[] = "EndSection";
	char StopString[] = "Section"; 
	bool flag = true;
	
	while(flag)
	{
		this->GetCurrentLine(&pString);
		if(strstr(SearchString, NewParamName) != NULL) break;
		if(feof(pFile)) flag = false;
		if(strstr(SearchString, StopString) != NULL) flag = false;
	}
	if(flag == false)
	{
		(*OutString)[0] = '\0';
		return false;
	}
	
	char * pch = strrchr(SearchString, '=');
	strcpy((*OutString), pch + 1);
	return true;
}




// Gets a zone parameter of a file name
bool CConfigFileReader::ReadName(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char * OutString)
{
	char SearchString[1200];
	sprintf(SearchString, "%d", ZoneID);
	
	char NewParamName[1200];
	strcpy(NewParamName, ParamName);
	strcat(NewParamName, "[");
	strcat(NewParamName, SearchString);
	strcat(NewParamName, "]");
	
	char * pString = SearchString;
	SearchSection(SectionName);
	// char StopString[] = "EndSection";
	char StopString[] = "Section"; 
	bool flag = true;
	
	while(flag)
	{
		this->GetCurrentLine(&pString);
		if(strstr(SearchString, NewParamName) != NULL) break;
		if(feof(pFile)) flag = false;
		if(strstr(SearchString, StopString) != NULL) flag = false;
	}
	if(flag == false)
	{
		OutString[0] = '\0';
		return false;
	}
	
	char delim = '"';
	char * pch = NULL;
	for(unsigned int k = 0; k < strlen(SearchString); k++) if(SearchString[k] == delim)
	{
		pch = &SearchString[k + 1];
		break;
	}
	if(pch == NULL)
	{
		OutString[0] = '\0';
		return false;
	}
	
	bool b = true;
	unsigned int k = 0;
	while(b)
	{
		char tmp = pch[k];
		if((tmp == delim) || (tmp == '\n') || (tmp == '\0'))
		{
			OutString[k] = '\0';
			break;
		}
		OutString[k] = tmp;
		k++;
	}

	return true;
}




// Gets a zone parameter of long type
bool CConfigFileReader::ReadLong(const char * SectionName, const char * ParamName, const unsigned int ZoneID, long * OutValue)
{
	char SearchString[1200];
	char * pString = SearchString;
	bool Res = ReadStr(SectionName, ParamName, ZoneID, &pString);
	if(!Res) return false;
	long value = 0;
	try
	{
		sscanf(SearchString, "%ld", &value);
	}
	catch(...)
	{
		Res = false;
	}
	*OutValue = value;
	return Res;	
}




// Gets a zone parameter of float-point type
bool CConfigFileReader::ReadFloat(const char * SectionName, const char * ParamName, const unsigned int ZoneID, float * OutValue)
{
	char SearchString[1200];
	char * pString = SearchString;
	bool Res = ReadStr(SectionName, ParamName, ZoneID, &pString);
	if(!Res) return false;
	float value = 0.0;
	try
	{
		sscanf(SearchString, "%f", &value);
	}
	catch(...)
	{
		Res = false;
	}
	*OutValue = value;
	return Res;
}





// Gets a zone parameter of long type. If it is absent, the common parameter havig the same name is extracted. If it fails, the default value is the output.
long CConfigFileReader::ReadLong_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const long DefaultValue)
{
	long Res = DefaultValue; 
	long a;
	bool b = ReadLong(SectionName, ParamName, ZoneID, &a);
	if(b) Res = a;
	else
	{
		b = ReadLong(SectionName, ParamName, &a);
		if(b) Res = a;
	}
	return Res;
}





// Gets a zone parameter of float type. If it is absent, the common parameter havig the same name is extracted. If it fails, the default value is the output.
float CConfigFileReader::ReadFloat_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const float DefaultValue)
{
	float Res = DefaultValue; 
	float a;
	bool b = ReadFloat(SectionName, ParamName, ZoneID, &a);
	if(b) Res = a;
	else
	{
		b = ReadFloat(SectionName, ParamName, &a);
		if(b) Res = a;
	}
	return Res;
}




// Gets a zone parameter of string type. If it is absent, the common parameter havig the same name is extracted. 
void CConfigFileReader::ReadString_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char ** OutString)
{
	bool b = ReadStr(SectionName, ParamName, ZoneID, OutString);
	if(!b) b = ReadStr(SectionName, ParamName, OutString);
}




// Gets a zone parameter of the file name. If it is absent, the common parameter havig the same name is extracted.
void CConfigFileReader::ReadName_Zone(const char * SectionName, const char * ParamName, const unsigned int ZoneID, char * OutString)
{
	bool b = ReadName(SectionName, ParamName, ZoneID, OutString);
	if(!b) b = ReadName(SectionName, ParamName, OutString);
	if(!b) OutString[0] = '\0';
}




// Sets the file pointer to the beginning of the data insertion in the given section. Output = number of lines in the data insertion.
unsigned long CConfigFileReader::GetData(const char * SectionName, fpos_t * pos)
{
	unsigned long N_Lines = 0;
	bool b = SearchSection(SectionName);
	if(!b) return N_Lines;
	const unsigned int StrSize = 10000; 
	char SearchString[StrSize];
	char * pString = SearchString;
	
	while(!feof(pFile))
	{
		GetCurrentLine(&pString, StrSize);
		if(pString == NULL) continue;
		if (strlen(pString) == 0) continue;
		if(strstr(pString, "DATA_START") == NULL) continue;
		else break;
	}
	if(feof(pFile)) return N_Lines;
	else
	{
		fgetpos(pFile, pos);
		
		while(!feof(pFile))
		{
			GetCurrentLine(&pString, StrSize);
			N_Lines++;
			if(pString == NULL) continue;
			if(strstr(pString, "DATA_END") == NULL) continue;
			else break;
		}
		N_Lines--;
		return N_Lines;
	}
	// fpos_t pos;
    // fgetpos (pFile, &pos);
	// fsetpos (pFile, &pos);
}



// Gets a pointer to the config file
FILE * CConfigFileReader::GetFile()
{
	return pFile;
}



// Gets the name of the config file
char * CConfigFileReader::GetFileName()
{
	return ConfigFileName;
}





// Copies a section to a new config file
bool CConfigFileReader::CopySectionToConfigWriter(const char * OriginalSectionName, const char * NewSectionName, CConfigFileWriter * ConfigWriter)
{
	if(!ConfigWriter->IsReady()) return false;
	bool b = SearchSection(OriginalSectionName);
	if(!b) return false;

	const unsigned int StrSize = 10000;
	char SearchString[StrSize];
	char * pString = SearchString;

	b = true;
	fpos_t pos;
	while(true)
	{
		fgetpos (pFile, &pos);
		b = GetCurrentLine(&pString, StrSize);
		if((!b) || (strlen(pString) != 0))
		{
			fsetpos(pFile, &pos);
			break;
		}
	}
	ConfigWriter->CreateNewSection(NewSectionName);

	unsigned long count = 0;
	b = true;
	while(b)
	{
		b = GetCurrentLine(&pString, StrSize);
		if(!b) break;
		if(strlen(pString) == 0)
		{
			ConfigWriter->AppendStringToTheEnd("");
			continue;
		}

		// fgets(pString, StrSize, pFile);

		if(strstr(pString, "Section ") != NULL) break;
		count++;
	}
	if(feof(pFile)) count--;

	fsetpos(pFile, &pos);
	for(unsigned long i = 0; i < count; i++)
	{
		GetCurrentLine(&pString, StrSize);
		if(strlen(pString) == 0)
		{
			ConfigWriter->AppendStringToTheEnd("");
			continue;
		}
		ConfigWriter->AppendStringToTheEnd(pString);
	}
	ConfigWriter->AppendStringToTheEnd("");
	return true;
}





//   ______________________________   private methods   ________________________________________________________

// Get one line in the config file without comments. Returns the flag of success
bool CConfigFileReader::GetCurrentLine(char ** Line, const unsigned int StrSize)
{
	bool b = GetCurrentLineWithComments(Line, StrSize);
	if(!b) (*Line)[0] = '\0';
	else
	{
		char * pch = strstr(*Line, "//");
		if(pch != NULL) strcpy(pch, "");
	}
	return b;
}




// Get one line in the config file with comments. Returns the flag of success
bool CConfigFileReader::GetCurrentLineWithComments(char ** Line, const unsigned int StrSize)
{
	bool b = true;
	if(feof(pFile))
	{
		b = false;
		(*Line)[0] = '\0';
	}
	else
	{
		char c;
		fgets(*Line, StrSize, pFile); // this option is faster
		for(unsigned int k = 0; k < StrSize; k++)
		{
			c = (*Line)[k];
			if(c == '\n') (*Line)[k] = '\0';
			if(c == '\0') break;
		}
	}
	return b;
}






// NOT OPERATIONAL YET
bool CConfigFileReader::ReadArrayOfStrings(const char * SectionName, const char * ParamName, unsigned int * N_Strings, char *** OutStrings)
{
	char SearchString[1200];
	char * pString = SearchString;
	(*N_Strings) = 0;
	
	bool Res = ReadStr(SectionName, ParamName, 0, &pString);
	if(Res)
	{
		unsigned int count = 1;
		bool b = true;
		while(b)
		{
			b = ReadStr(SectionName, ParamName, count, &pString);
			count++;
		}
		count--;
		(*OutStrings) = (char **) calloc(count, sizeof(char *));
		(*N_Strings) = count;
		char * str;
		for(unsigned int i = 0; i < count; i++)
		{
			ReadStr(SectionName, ParamName, count, &str);
			unsigned int len = strlen(str) + 1; 
			(*OutStrings)[i] = new char[len];
			strcpy((*OutStrings)[i], str);
		}
	}
	else
	{
		(*N_Strings) = 0;
		(*OutStrings) = NULL;
	}
	return Res;
}




// NOT OPERATIONAL YET
bool CConfigFileReader::ReadArrayOfLongs(const char * SectionName, const char * ParamName, unsigned int * N_Values, long ** OutValues)
{
	char SearchString[1200];
	char * pString = SearchString;
	(*N_Values) = 0;
	
	bool Res = ReadStr(SectionName, ParamName, 0, &pString);
	if(Res)
	{
		unsigned int count = 1;
		bool b = true;
		while(b)
		{
			b = ReadStr(SectionName, ParamName, count, &pString);
			count++;
		}
		count--;
		(*OutValues) = (long *) calloc(count, sizeof(long));
		(*N_Values) = count;
		long a;
		for(unsigned int i = 0; i < count; i++)
		{
			ReadLong(SectionName, ParamName, count, &a);
			(*OutValues)[i] = a;
		}
	}
	else
	{
		(*N_Values) = 0;
		(*OutValues) = NULL;
	}
	return Res;
}



// NOT OPERATIONAL YET
bool CConfigFileReader::ReadArrayOfFloats(const char * SectionName, const char * ParamName, unsigned int * N_Values, float ** OutValues)
{
	char SearchString[1200];
	char * pString = SearchString;
	(*N_Values) = 0;
	
	bool Res = ReadStr(SectionName, ParamName, 0, &pString);
	if(Res)
	{
		unsigned int count = 1;
		bool b = true;
		while(b)
		{
			b = ReadStr(SectionName, ParamName, count, &pString);
			count++;
		}
		count--;
		(*OutValues) = (float *) calloc(count, sizeof(float));
		(*N_Values) = count;
		float a;
		for(unsigned int i = 0; i < count; i++)
		{
			ReadFloat(SectionName, ParamName, count, &a);
			(*OutValues)[i] = a;
		}
	}
	else
	{
		(*N_Values) = 0;
		(*OutValues) = NULL;
	}
	return Res;
}


