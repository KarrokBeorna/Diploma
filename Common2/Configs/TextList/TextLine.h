#ifndef TEXTLINE_H_
#define TEXTLINE_H_


// The class describes one line in the text list. There are three fields: a text string and two pointers to the 
// previous and next lines.

class CTextLine
{
public:
	char * String; // Content of the line in a form of dynamic string
	CTextLine * PrevLine;  // Pointer to the previous line
	CTextLine * NextLine;  // Pointer to the next line
	
	CTextLine(CTextLine * PrevLine, const char * String); // Constructor
	~CTextLine(); // Destructor
	
	void RewriteLine(const char * NewString); // Rewrite the content of the line with a new string
};


#endif /*TEXTLINE_H_*/
