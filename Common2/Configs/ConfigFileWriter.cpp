
#include <cstring>

#include "ConfigFileWriter.h"





// ************************************************************************************************************
//   ____________________________________   Class CConfigFileWriter  ________________________________________
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________


// Constructor 1: Modify an existing config file
CConfigFileWriter::CConfigFileWriter(const char * ConfigFileName)
{
	char FileName[500];
	sprintf(FileName, "%s", ConfigFileName);
	TextList = new CTextList(FileName);
	Ready = TextList->IsReady(); 
}



// Constructor 2: Create a new config file
CConfigFileWriter::CConfigFileWriter()
{
	TextList = new CTextList();
	Ready = TextList->IsReady(); 
}



// Constructor 3: Copy
CConfigFileWriter::CConfigFileWriter(CConfigFileWriter * ReadyConfig)
{
	Ready = ReadyConfig->IsReady();
	if(!Ready)
	{
		printf("\nError. Existing object of CConfigFileWriter is not ready. Copy constructor.");
		return;
	}
	TextList = new CTextList();
	Ready = TextList->IsReady();
	if(!Ready) return;

	char TmpStr[10000];
	char * pTmpStr = TmpStr;
	ReadyConfig->TextList->Rewind();
	while(true)
	{
		ReadyConfig->TextList->GetCurrStringWithComments(&pTmpStr);
		// TextList->AppendLine(pTmpStr);
		TextList->InsertLineBeforeCurrentLine(pTmpStr);
		TextList->IncLine();
		if(!ReadyConfig->TextList->IncLine()) break;
	}
}





// Constructor 4: Modify an existing config file
CConfigFileWriter::CConfigFileWriter(FILE * ConfigFile)
{
	TextList = new CTextList(ConfigFile);
	Ready = TextList->IsReady();
}





// Destructor
CConfigFileWriter::~CConfigFileWriter()
{
	if(TextList != NULL) delete TextList;
}





//   ______________________________   Properties   ________________________________________________________


// Get the flag of readiness
bool CConfigFileWriter::IsReady()
{
	return Ready;
}



// Save the information to the text file
bool CConfigFileWriter::SaveToFile(const char * FileName)
{
	return TextList->StoreInTextFile(FileName);
}




// Save the information to the text file
bool CConfigFileWriter::SaveToFile(FILE * pTextFile)
{
	return TextList->StoreInTextFile(pTextFile);
}



// Get a flag: Whether the section exists or not
bool CConfigFileWriter::DoesSectionExist(const char * SectionName)
{
	long id = GetLineIDForSection(SectionName);
	if(id == -1) return false;
	else return true;
}




// Creates a new section in the end of the text list
void CConfigFileWriter::CreateNewSection(const char * SectionName)
{
	char String[500];
	sprintf(String, "				Section %s", SectionName);
	if(!DoesSectionExist(SectionName))
	{
		TextList->AppendLine("//-------------------------------------------------------------------------");
		TextList->AppendLine(String);
		TextList->AppendLine("//-------------------------------------------------------------------------");
	}
}





// Append a text line with arbitrary content to the end of the section
void CConfigFileWriter::AppendTextToSection(const char * SectionName, const char * String)
{
	int section_id = GetLineIDForSection(SectionName);
	char TmpStr[500];
	char * pTmpStr = TmpStr;

	if(section_id == -1) // add new string into the section
	{
		CreateNewSection(SectionName);
		section_id = GetLineIDForSection(SectionName);
	}
	TextList->IncLine();	
	while(true)
	{
		bool b = TextList->GetCurrStringWithoutComments(&pTmpStr);
		if(!b) break;
		if((strstr(TmpStr, "DATA_START") != NULL) || (strstr(TmpStr, "Section ") != NULL)) break;
		TextList->IncLine();
	}
	TextList->InsertLineBeforeCurrentLine(String);
}





// Appends a text line to the end of the config file
void CConfigFileWriter::AppendStringToTheEnd(const char * String)
{
	TextList->AppendLine(String);
}





// Delete the parameter from the section
bool CConfigFileWriter::DeleteParam(const char * SectionName, const char * ParamName)
{
	int id = GetLineIDForParam(SectionName, ParamName);
	if(id == -1) return false;
	TextList->DeleteCurrLine();
	return true;
}




// Delete the entire section
bool CConfigFileWriter::DeleteSection(const char * SectionName)
{
	int sectionid = GetLineIDForSection(SectionName);
	if(sectionid == -1) return false;
	bool b = TextList->DeleteCurrLine();
	if(!b) return false;
	char TmpStr[500];
	char * pTmpStr = TmpStr; 
	while(b)
	{
		b = TextList->GetCurrStringWithoutComments(&pTmpStr);
		if(!b) break;
		if(strstr(TmpStr, "Section ") != NULL) break;
		b = TextList->DeleteCurrLine();
	}
	return true;
}




// Append a data fragment to the end of the section. Note: each section may contain only one data fragment
bool CConfigFileWriter::AppendDataFragmentToSection(const char * SectionName, FILE * pDataFile)
{
	const unsigned int LimitSize = 15000;
	if(pDataFile == NULL) return false;
	int section_id = GetLineIDForSection(SectionName);
	if(section_id == -1)
	{
		CreateNewSection(SectionName);
		section_id = GetLineIDForSection(SectionName);
	}
	if(DoesDataFragmentExist(SectionName)) DeleteDataFragment(SectionName);

	AppendTextToSection(SectionName, "DATA_START");
	rewind(pDataFile);
	char OneLine[LimitSize];
	char * pOneLine = OneLine; 
	while(true)
	{
		// fgets(OneLine, 15000, pDataFile);
		bool b = TextList->ReadCurrentLine(pDataFile, &pOneLine, LimitSize);
		if(!b) break;
		if(strlen(pOneLine) == 0) continue;
		TextList->InsertLineAfterCurrentLine(pOneLine);
	}
	TextList->InsertLineAfterCurrentLine("DATA_END");
	return true;
}



// Gets a flag: whether a data fragment exists in the section
bool CConfigFileWriter::DoesDataFragmentExist(const char * SectionName)
{
	int section_id = GetLineIDForSection(SectionName);
	if(section_id == -1) return false;
	TextList->IncLine();
	char TmpStr[500];
	char * pTmpStr = TmpStr;
	while(true)
	{
		bool b = TextList->GetCurrStringWithoutComments(&pTmpStr);
		if(!b) break;
		if(strstr(TmpStr, "DATA_START") != NULL) return true;
		if(strstr(TmpStr, "Section ") != NULL) break;
		TextList->IncLine();
	}
	return false;
}



// Delete the data frafment from the section, if it exists
void CConfigFileWriter::DeleteDataFragment(const char * SectionName)
{
	bool b = DoesDataFragmentExist(SectionName);
	if(!b) return;
	
	char TmpStr[500];
	char * pTmpStr = TmpStr; 
	while(b)
	{
		b = TextList->GetCurrStringWithoutComments(&pTmpStr);
		if(!b) break;
		if(strstr(TmpStr, "DATA_END") != NULL)
		{
			TextList->DeleteCurrLine();
			break;
		}
		if(strstr(TmpStr, "Section ") != NULL) break;
		b = TextList->DeleteCurrLine();
	}
}



// Print out the text list on the console
void CConfigFileWriter::PrintOutTextList()
{
	TextList->PrintOutText();
	printf("\n");
}





// Finds defined line in a section and sets the cursor to this line. Output = false, if the search string doesn't exist
bool CConfigFileWriter::SetCursorToLine(const char * SectionName, const char * SearchString)
{
	int section_id = GetLineIDForSection(SectionName);
	if(section_id == -1) return false;

	char TmpStr[500];
	char * pTmpStr = TmpStr;
	TextList->IncLine();
	bool succ = false;
	while(true)
	{
		bool b = TextList->GetCurrStringWithComments(&pTmpStr);
		if(!b) break;
		if((strstr(TmpStr, "DATA_START") != NULL) || (strstr(TmpStr, "Section ") != NULL)) break;
		if(strstr(TmpStr, SearchString) != NULL)
		{
			succ = true;
			break;
		}
		TextList->IncLine();
	}
	return succ;
}




// Inserts a new line into the current position
void CConfigFileWriter::InsertNewLineIntoCurrentPos(const char * String)
{
	TextList->InsertLineAfterCurrentLine(String);
}




// Gets the content of current string (including comments)
bool CConfigFileWriter::GetCurrentString(char ** OutStr)
{
	return TextList->GetCurrStringWithComments(OutStr);
}




// Changes the name of a certain section
bool CConfigFileWriter::RenameSection(const char * OldName, const char * NewName)
{
	bool succ = false;
	char SearchString[500];
	char ListString[500];
	char * pListString = ListString;
	sprintf(SearchString, "Section %s", OldName);

	TextList->Rewind();
	while(true)
	{
		bool b = TextList->GetCurrStringWithoutComments(&pListString);
		if(b)
		{
			if((strlen(pListString) == 0) || (strstr(pListString, SearchString) == NULL))
			{
				TextList->IncLine();
				continue;
			}
			else
			{
				succ = true;
				break;
			}
		}
		else break;
	}
	if(succ)
	{
		TextList->DeleteCurrLine();
		sprintf(ListString, "				Section %s", NewName);
		TextList->InsertLineBeforeCurrentLine(ListString);
	}
	return succ;
}




// Appends contents of another config structure to the end of this config structure
void CConfigFileWriter::AppendAnotherConfig(CConfigFileWriter * ExistingConfig)
{
	char String[10000];
	char * pStr = String;
	TextList->AppendLine("");
	ExistingConfig->TextList->Rewind();
	bool b = true;
	while (b)
	{
		b = ExistingConfig->TextList->GetCurrStringWithComments(&pStr);
		if (!b) break;
		TextList->InsertLineAfterCurrentLine(pStr);
	}
}




// Set or reset the parameter's value in the section
void CConfigFileWriter::SetStrParam(const char * SectionName, const char * ParamName, const char * Value)
{
	long id = GetLineIDForParam(SectionName, ParamName);
	char NewString[500];
	sprintf(NewString, "%s=%s", ParamName, Value);
	char TmpStr[500];
	char * pTmpStr = TmpStr;
	
	if(id == -1) // add new string into the section
	{
		int section_id = GetLineIDForSection(SectionName);
		if(section_id == -1)
		{
			CreateNewSection(SectionName);
			section_id = GetLineIDForSection(SectionName);
		}
		TextList->IncLine();
		while(true)
		{
			bool b = TextList->GetCurrStringWithoutComments(&pTmpStr);
			if(!b) break;
			if((strstr(TmpStr, "DATA_START") != NULL) || (strstr(TmpStr, "Section ") != NULL)) break;
			TextList->IncLine();
		}
		TextList->InsertLineBeforeCurrentLine(NewString);
	}
	else // reset string
	{
		TextList->ResetStrInCurrLine(NewString);
	}
}



// Set or reset a float-point parameter in the section
void CConfigFileWriter::SetFloat(const char * SectionName, const char * ParamName, const float Value)
{
	char ValueStr[150];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%f", Value);
	// SetStrParam(SectionName, ParamName, pValueStr);

	int id = GetLineIDForParam(SectionName, ParamName);
	char NewString[500];
	sprintf(NewString, "%s=%s", ParamName, pValueStr);
	char TmpStr[500];
	char * pTmpStr = TmpStr;

	if(id == -1) // add new string into the section
	{
		int section_id = GetLineIDForSection(SectionName);
		if(section_id == -1)
		{
			CreateNewSection(SectionName);
			section_id = GetLineIDForSection(SectionName);
		}
		TextList->IncLine();
		while(true)
		{
			bool b = TextList->GetCurrStringWithoutComments(&pTmpStr);
			if(!b) break;
			if((strstr(TmpStr, "DATA_START") != NULL) || (strstr(TmpStr, "Section ") != NULL)) break;
			TextList->IncLine();
		}
		TextList->InsertLineBeforeCurrentLine(NewString);
	}
	else // reset string
	{
		TextList->DeleteCurrLine();
		TextList->InsertLineBeforeCurrentLine(NewString);
	}
}




// Set or reset a float-point parameter in the certain zone of the section
void CConfigFileWriter::SetFloat(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const float Value)
{
	char ValueStr[150];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%f", Value);
	
	char ParamString[500];
	char * pParamString = ParamString;
	sprintf(ParamString, "%s[%d]", ParamName, ZoneID);
	SetStrParam(SectionName, pParamString, pValueStr); 
}





// Set or reset a long integer parameter in the section
void CConfigFileWriter::SetLong(const char * SectionName, const char * ParamName, const long Value)
{
	char ValueStr[150];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%ld", Value);
	SetStrParam(SectionName, ParamName, pValueStr); 
}




// Set or reset a long integer parameter in the certain zone of the section
void CConfigFileWriter::SetLong(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const long Value)
{
	char ValueStr[150];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%ld", Value);
	
	char ParamString[500];
	char * pParamString = ParamString;
	sprintf(ParamString, "%s[%d]", ParamName, ZoneID);
	SetStrParam(SectionName, pParamString, pValueStr); 
}





// Set or reset a string parameter, which is quoted inside "",  in the section
void CConfigFileWriter::SetName(const char * SectionName, const char * ParamName, const char * Value)
{
	char delim = '"';
	char ValueStr[500];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%c%s%c", delim, Value, delim);
	SetStrParam(SectionName, ParamName, pValueStr); 	
}




// Set or reset a string parameter, which is quoted inside "", in the certain zone of the section.
void CConfigFileWriter::SetName(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const char * Value)
{
	char delim = '"';
	char ValueStr[500];
	char * pValueStr = ValueStr;
	sprintf(ValueStr, "%c%s%c", delim, Value, delim);

	char ParamString[500];
	char * pParamString = ParamString;
	sprintf(ParamString, "%s[%d]", ParamName, ZoneID);
	SetStrParam(SectionName, pParamString, pValueStr); 	
}






//   ______________________________   private methods   ________________________________________________________



// Put the pointer to the current line to another line in accordance with line id
bool CConfigFileWriter::SetCurrLineID(const unsigned int ID)
{
	unsigned int count = 0;
	bool succ = true;
	TextList->Rewind();
	while(true)
	{
		bool b = TextList->IncLine();
		if(!b)
		{
			succ = false;
			break;
		}
		count++;
	}
	return succ; 
}




// Get id of the line, which contains start of the section
long CConfigFileWriter::GetLineIDForSection(const char * SectionName)
{
	char SearchString[500];
	char ListString[10000];
	char * pListString = ListString; 
	sprintf(SearchString, "Section %s", SectionName);

	bool succ = false;
	long count = 0;
	TextList->Rewind();
	while(true)
	{
		bool b = TextList->GetCurrStringWithoutComments(&pListString);
		if(b)
		{
			if((strlen(pListString) == 0) || (strstr(pListString, SearchString) == NULL))
			{
				count++;
				TextList->IncLine();
				continue;
			}
			else
			{
				succ = true; 
				break;
			}
		}
		else break;
	}
	if(!succ) return -1;
	else return count;
}




// Get id of the line corresponded to the parameter; order of id starts from the section beginning
long CConfigFileWriter::GetLineIDForParam(const char * SectionName, const char * ParamName)
{
	long id = GetLineIDForSection(SectionName);
	if(id == -1) return -1;

	TextList->IncLine();
	char ListString[500];
	char WrongString[500];
	sprintf(WrongString, "%s[", ParamName);
	char * pListString = ListString;
	bool succ = false;
	long count = 0;

	while(true)
	{
		bool b = TextList->GetCurrStringWithoutComments(&pListString);
		if(b)
		{
			if(strlen(pListString) == 0)
			{
				TextList->IncLine();
				count++;
				continue;
			}
			if((strstr(pListString, "DATA_START") != NULL) || (strstr(pListString, "Section ") != NULL)) break;
			if((strstr(pListString, "=") != NULL) && (strstr(pListString, ParamName) != NULL) && (strstr(pListString, WrongString) == NULL)) 
			{
				succ = true; 
				break;
			}
			TextList->IncLine();
			count++;
		}
		else break;
	}
	
	if(!succ) return -1;
	else return count;
}




// Get id of the line corresponded to the zone parameter; order of id starts from the section beginning
long CConfigFileWriter::GetLineIDForParam(const char * SectionName, const char * ParamName, const unsigned int ZoneID)
{
	char ParamString[500];
	sprintf(ParamString, "%s[%d]", ParamName, ZoneID);
	return GetLineIDForParam(SectionName, ParamString); 
}





