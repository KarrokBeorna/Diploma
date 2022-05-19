#ifndef CONFIGFILEWRITER_H_
#define CONFIGFILEWRITER_H_

#include <cstdio>
#include "TextList/TextList.h"


// The class supports modification and creation of configuration files by forming a formatted text list of parameters.  


class CConfigFileWriter
{
private:
	bool Ready; // Flag of readiness
	CTextList * TextList; // internal object of the text list
	
	bool SetCurrLineID(const unsigned int ID); // Put the pointer to the current line to another line in accordance with line id
	long GetLineIDForSection(const char * SectionName); // Get id of the line, which contains start of the section
	long GetLineIDForParam(const char * SectionName, const char * ParamName); // Get id of the line corresponded to the parameter; order of id starts from the section beginning
	long GetLineIDForParam(const char * SectionName, const char * ParamName, const unsigned int ZoneID); // Get id of the line corresponded to the zone parameter; order of id starts from the section beginning
	
public:
	CConfigFileWriter(const char * ConfigFileName); // Constructor 1: Modify an existing config file
	CConfigFileWriter(); // Constructor 2: Create a new config file
	CConfigFileWriter(CConfigFileWriter * ReadyConfig); // Constructor 3: Copy
	CConfigFileWriter(FILE * ConfigFile); // Constructor 4: Modify an existing config file
	~CConfigFileWriter(); // Destructor
	
	bool IsReady(); // Get the flag of readiness
	bool SaveToFile(const char * FileName); // Save the information to the text file
	bool SaveToFile(FILE * pTextFile); // Save the information to the text file
	bool DoesSectionExist(const char * SectionName); // Get a flag: Whether the section exists or not
	void CreateNewSection(const char * SectionName); // Creates a new section in the end of the text list
	void AppendTextToSection(const char * SectionName, const char * String); // Append a text line with arbitrary content to the end of the section
	void AppendStringToTheEnd(const char * String); // Appends a text line to the end of the config file
	void SetStrParam(const char * SectionName, const char * ParamName, const char * Value); // Set or reset the parameter's value in the section
	bool DeleteParam(const char * SectionName, const char * ParamName); // Delete the parameter from the section
	bool DeleteSection(const char * SectionName); // Delete the entire section
	bool AppendDataFragmentToSection(const char * SectionName, FILE * pDataFile); // Append a data fragment to the end of the section. Note: each section may contain only one data fragment
	bool DoesDataFragmentExist(const char * SectionName); // Gets a flag: whether a data fragment exists in the section 
	void DeleteDataFragment(const char * SectionName); // Delete the data frafment from the section, if it exists
	void PrintOutTextList(); // Print out the text list on the console 
	bool SetCursorToLine(const char * SectionName, const char * SearchString); // Finds defined line in a section and sets the cursor to this line. Output = false, if the search string doesn't exist
	void InsertNewLineIntoCurrentPos(const char * String); // Inserts a new line into the current position
	bool GetCurrentString(char ** OutStr); // Gets the content of current string (including comments)
	bool RenameSection(const char * OldName, const char * NewName); // Changes the name of a certain section
	void AppendAnotherConfig(CConfigFileWriter * ExistingConfig); // Appends contents of another config structure to the end of this config structure

	void SetFloat(const char * SectionName, const char * ParamName, const float Value); // Set or reset a float-point parameter in the section
	void SetFloat(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const float Value); // Set or reset a float-point parameter in the certain zone of the section
	void SetLong(const char * SectionName, const char * ParamName, const long Value); // Set or reset a long integer parameter in the section
	void SetLong(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const long Value); // Set or reset a long integer parameter in the certain zone of the section
	void SetName(const char * SectionName, const char * ParamName, const char * Value); // Set or reset a string parameter, which is quoted inside "",  in the section
	void SetName(const char * SectionName, const char * ParamName, const unsigned int ZoneID, const char * Value); // Set or reset a string parameter, which is quoted inside "", in the certain zone of the section.
};


#endif /*CONFIGFILEWRITER_H_*/
