#ifndef TEXTLIST_H_
#define TEXTLIST_H_

#include <cstdio>

#include "TextLine.h"


// This class represent a two-directional list of strings used to modify or create a config file.


class CTextList
{
private:
	bool Ready; // Flag: is ready (may be not ready, if the path to the existed config file is wrong) 
	unsigned int N_Lines; // Number of lines in the list
	CTextLine * FirstLine; // Pointer to the first line
	CTextLine * CurrLine;  // Pointer to the current line
	
public:
	CTextList(const char * FileName); // Constructor 1: Modify an existed config file
	CTextList(); // Constructor 2: Create a new config file
	CTextList(FILE * pFile); // Constructor 3: Modify an existed config file
	~CTextList(); // Destructor
	
	bool ReadCurrentLine(FILE * pFile, char ** String, const unsigned int LimitStringSize); // Read current string from a text file excluding symbols \n and put to the current line of the list
	void PrintOutText(); // Print out the entire list
	void AppendLine(const char * String); // Append a new line to the end of the list
	bool StoreInTextFile(const char * FileName); // Save information in a text file
	bool StoreInTextFile(FILE * TextFile); // Save information in a text file
	bool IsReady(); // Get flag of readiness
	void Rewind(); // Rewind current line to the beginning
	bool IncLine(); // Increment pointer of the current line to the next line
	bool DecLine(); // Decrement pointer of the current line to the previous line
	bool GetCurrStringWithoutComments(char ** pString); // Get a string containing in the current line excluding comments
	bool GetCurrStringWithComments(char ** pString); // Get a string containing in the current line including comments
	void ResetStrInCurrLine(const char * NewString); // Reset string data in the current line
	void InsertLineBeforeCurrentLine(const char * String); // Insert a new line above the current line
	void InsertLineAfterCurrentLine(const char * String); // Insert a new line below the current line
	bool DeleteCurrLine(); // Delete the current line
	void DeleteLines(const unsigned int StartLineID, const unsigned int EndLineID); // Delete lines from the start line to the end line; identifiers are determined by the ordering sequence of lines
};





#endif /*TEXTLIST_H_*/
