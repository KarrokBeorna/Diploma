
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "TextLine.h"



// ************************************************************************************************************
//   ______________________________________   Class CTextLine  ________________________________________
// ************************************************************************************************************



//   ______________________________   public methods   ________________________________________________________



// Constructor
CTextLine::CTextLine(CTextLine * PrevLine, const char * String)
{
	this->PrevLine = PrevLine;
	this->NextLine = NULL;
	
	if(PrevLine != NULL) PrevLine->NextLine = this;
	
	unsigned int Strlen = strlen(String);
	this->String = (char *) calloc(Strlen + 1, sizeof(char));
	for(unsigned int i = 0; i < Strlen; i++) this->String[i] = String[i];
	this->String[Strlen] = '\0';
	// sprintf(this->String, "%s", String);
	// printf("\n%s", this->String);
}



// Destructor
CTextLine::~CTextLine()
{
	if(PrevLine != NULL) PrevLine->NextLine = NextLine;
	if(NextLine != NULL) NextLine->PrevLine = PrevLine;
	free(String);
}




// Rewrite the content of the line with a new string
void CTextLine::RewriteLine(const char * NewString)
{
	unsigned int Strlen = strlen(NewString);
	free(String);
	String = (char *) calloc(Strlen + 1, sizeof(char));
	for(unsigned int i = 0; i < Strlen; i++) this->String[i] = NewString[i];
	this->String[Strlen] = '\0';
}




