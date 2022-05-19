
#include <cstring>

#include "TextList.h"



// ************************************************************************************************************
//   ______________________________________   Class CTextList  ________________________________________
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________


// Constructor 1: Modify an existed config file
CTextList::CTextList(const char * FileName)
{
	FirstLine = NULL;
	CurrLine = NULL;
	Ready = false;
	FILE * pFile = fopen(FileName , "r");
	if(pFile == NULL)
	{
		printf("\nUnable to read a file %s.", FileName);
		return;
	}

	rewind(pFile);
	
	const unsigned int LimitStringSize = 10000;
	char TmpString[LimitStringSize];
	char * pTmpString = TmpString; 
	CTextLine * PrevLine = NULL;
	
	unsigned long count = 0;
	while(!feof(pFile))
	{
		fgets(TmpString, LimitStringSize, pFile);
		count++;
	}
	count--;
	rewind(pFile);
	for(unsigned long i = 0; i < count; i++)
	{
		ReadCurrentLine(pFile, &pTmpString, LimitStringSize);
		CTextLine * ALine = new CTextLine(PrevLine, pTmpString);
		if(PrevLine == NULL) FirstLine = ALine;
		PrevLine = ALine;
	}

	this->N_Lines = count;
	CurrLine = FirstLine;
	Ready = true;
	fclose(pFile);
}




// Constructor 2: Create a new config file
CTextList::CTextList()
{
	N_Lines = 0;
	CurrLine = NULL;
	FirstLine = NULL;
	Ready = true;
	this->AppendLine("");
}





// Constructor 3: Modify an existing config file
CTextList::CTextList(FILE * pFile)
{
	if (pFile == NULL)
	{
		printf("\nUnable to read a file.");
		return;
	}

	rewind(pFile);

	const unsigned int LimitStringSize = 10000;
	char TmpString[LimitStringSize];
	char * pTmpString = TmpString;
	CTextLine * PrevLine = NULL;

	unsigned long count = 0;
	while (!feof(pFile))
	{
		fgets(TmpString, LimitStringSize, pFile);
		count++;
	}
	count--;
	rewind(pFile);
	for (unsigned long i = 0; i < count; i++)
	{
		ReadCurrentLine(pFile, &pTmpString, LimitStringSize);
		CTextLine * ALine = new CTextLine(PrevLine, pTmpString);
		if (PrevLine == NULL) FirstLine = ALine;
		PrevLine = ALine;
	}

	this->N_Lines = count;
	CurrLine = FirstLine;
	Ready = true;
	fclose(pFile);
}


// Destructor
CTextList::~CTextList()
{
	if(FirstLine != NULL)
	{
		CTextLine * ALine = FirstLine; 
		while(ALine != NULL)
		{
			CTextLine * NextLine = ALine->NextLine;
			delete ALine;
			ALine = NextLine;
		}
	}
}




// Read current string from a text file excluding symbols \n and put to the current line of the list
bool CTextList::ReadCurrentLine(FILE * pFile, char ** String, const unsigned int LimitStringSize)
{
	bool b = true;
	if(feof(pFile))
	{
		(*String)[0] = '\0';
		b = false;
	}
	else
	{
		char c;
		fgets(*String, LimitStringSize, pFile); // this option is faster
		for(unsigned int k = 0; k < LimitStringSize; k++)
		{
			c = (*String)[k];
			if(c == '\n') (*String)[k] = '\0';
			if(c == '\0') break;
		}
	}
	return b;
}




// Print out the entire list
void CTextList::PrintOutText()
{
	if(FirstLine != NULL)
	{
		CTextLine * ALine = FirstLine; 
		while(ALine != NULL)
		{
			CTextLine * NextLine = ALine->NextLine;
			printf("\n%s", ALine->String);
			ALine = NextLine;
		}
	}
}



// Append a new line to the end of the list
void CTextList::AppendLine(const char * String)
{
	if(CurrLine == NULL)
	{
		CurrLine = FirstLine;
		if(FirstLine == NULL)
		{
			FirstLine = new CTextLine(NULL, String);
			CurrLine = FirstLine;
		}
	}

	CTextLine * ALine = CurrLine;
	CTextLine * NextLine = CurrLine->NextLine;
	while(NextLine != NULL)
	{
		ALine = NextLine;
		NextLine = ALine->NextLine;
	}
	CTextLine * NewLine = new CTextLine(ALine, String);
	CurrLine = NewLine;

	N_Lines++;
}



// Save information in a text file
bool CTextList::StoreInTextFile(FILE * TextFile)
{
	if(TextFile == NULL)
	{
		printf("\nUnable to create a text file in CTextList::StoreInTextFile.");
		return false;
	}

	if(FirstLine != NULL)
	{
		CTextLine * ALine = FirstLine;
		while(ALine != NULL)
		{
			CTextLine * NextLine = ALine->NextLine;
			fprintf(TextFile, "%s\n", ALine->String);
			ALine = NextLine;
		}
	}
	return true;
}




// Save information in a text file
bool CTextList::StoreInTextFile(const char * FileName)
{
	FILE * pFile = fopen(FileName , "w");
	if(pFile == NULL)
	{
		printf("\nUnable to create a file %s.", FileName);
		return false;
	}
	bool b = StoreInTextFile(pFile);
	fclose(pFile);
	return b;
}




// Get flag of readiness
bool CTextList::IsReady()
{
	return this->Ready;
}



// Rewind current line to the beginning
void CTextList::Rewind()
{
	CurrLine = FirstLine;
}



// Increment pointer of the current line to the next line
bool CTextList::IncLine()
{
	if(CurrLine != NULL)
	{
		CurrLine = CurrLine->NextLine;
		return true;
	}
	else return false;
}


// Decrement pointer of the current line to the previous line
bool CTextList::DecLine()
{
	if(CurrLine != NULL)
	{
		if(CurrLine->PrevLine == NULL) CurrLine = FirstLine;
		else CurrLine = CurrLine->PrevLine; 
		return true;
	}
	else return false;
}



// Decrement pointer of the current line to the previous line
bool CTextList::GetCurrStringWithoutComments(char ** pString)
{
	if(CurrLine == NULL)
	{
		*pString[0] = '\0';
		return false;
	}
	else sprintf(*pString, "%s", CurrLine->String);   

	char * pch = strstr(*pString, "//");
	if(pch != NULL) strcpy(pch, "");
	return true;
}



// Get a string containing in the current line including comments
bool CTextList::GetCurrStringWithComments(char ** pString)
{
	if(CurrLine == NULL)
	{
		*pString[0] = '\0';
		return false;
	}
	else
	{
		strcpy(*pString, CurrLine->String);
		return true;   
	}
}



// Reset string data in the current line
void CTextList::ResetStrInCurrLine(const char * NewString)
{
	// if(CurrLine != NULL) sprintf(CurrLine->String, "%s", NewString);
	if(CurrLine != NULL) strcpy(CurrLine->String, NewString);
}





// Insert a new line above the current line
void CTextList::InsertLineBeforeCurrentLine(const char * String)
{
	if(CurrLine == NULL)
	{
		CTextLine * Line = FirstLine;
		while(true)
		{
			if(Line->NextLine == NULL)
			{
				CurrLine = Line;
				break;
			}
			else Line = Line->NextLine;
		}
		CTextLine * NewLine = new CTextLine(CurrLine, String);
		CurrLine = NewLine;
	}
	else
	{
		CTextLine * PrevLine = CurrLine->PrevLine;
		CTextLine * NewLine;
		if(PrevLine == NULL)
		{
			NewLine = new CTextLine(NULL, String);
			FirstLine = NewLine; 
		}
		else 
		{
			NewLine = new CTextLine(PrevLine, String);
		}
		NewLine->NextLine = CurrLine;
		CurrLine->PrevLine = NewLine;
		CurrLine = NewLine; 
	}
	N_Lines++;
}




// Insert a new line below the current line
void CTextList::InsertLineAfterCurrentLine(const char * String)
{
	if(CurrLine == NULL) return;
	CTextLine * NextLine = CurrLine->NextLine;
	CTextLine * NewLine = new CTextLine(CurrLine, String);
	NewLine->NextLine = NextLine;
	if(NextLine!= NULL) NextLine->PrevLine = NewLine; 
	CurrLine = NewLine;
	N_Lines++;
}




// Delete the current line
bool CTextList::DeleteCurrLine()
{
	if(CurrLine == NULL) return false;
	bool res = true;
	CTextLine * PrevLine = CurrLine->PrevLine;
	CTextLine * NextLine = CurrLine->NextLine;
	if(CurrLine == FirstLine) FirstLine = NextLine;
	if(PrevLine != NULL) PrevLine->NextLine = NextLine;
	if(NextLine != NULL) NextLine->PrevLine = PrevLine;
	delete CurrLine;
	if(NextLine != NULL) CurrLine = NextLine;
	else
	{
		CurrLine = PrevLine;
		res = false;
	}
	N_Lines--;
	return res;
}



// Delete lines from the start line to the end line; identifiers are determined by the ordering sequence of lines
void CTextList::DeleteLines(const unsigned int StartLineID, const unsigned int EndLineID)
{
	CTextLine * Line = FirstLine;
	unsigned int lineid = 0;
	while(true)
	{
		if(lineid == StartLineID) break;
		if(Line->NextLine == NULL) break;
		else
		{
			Line = Line->NextLine;
			lineid++;
		}
	}
	if(lineid == StartLineID)
	{
		CurrLine = Line;
		while(true)
		{
			if(lineid == EndLineID) break;
			bool b = DeleteCurrLine();
			if(!b) break;
			lineid++;
		}
	}
}










