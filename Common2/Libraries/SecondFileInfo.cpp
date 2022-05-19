
#include "SecondFileInfo.h"

// Reads a line of data using getline()
bool GetFileInfo1(const char * FileName, unsigned long * Lines, unsigned int * Columns, bool CheckColumns )
{
	 bool out = true;
	 std::ifstream infile;
	 infile.open( FileName );
	 if( !infile.is_open() )
	 {
		  printf("\nThe file %s doesn't exist\n", FileName);
		  return false;
	 }
	 unsigned int cols = 0;
	 unsigned long lines = 0;
	 
	 // now find the number of classes counting the class labels
	 std::string line;
	 while( ! infile.eof() )
	 {
	  getline( infile, line );
	  lines++;
	 }
	 if (lines > 0) lines--;
	 infile.clear();
	 infile.seekg( 0, std::ios::beg );	// Reset to the start of the file
	 getline( infile, line );
	 
	 unsigned int temp = line.size();
	 if(temp == 1)
	 {
		 getline( infile, line );
		 temp = line.size();
	 }

	 for(unsigned int i = 0; i < temp; i++)
	 {
		 if( line[ i ] == Delimiter ) cols++;
	 }
	 
	 cols++;
	 if( temp > 1 )
	 {
		 if( ( line[ temp - 2 ] == Delimiter ) || ( line[ temp - 1 ] == Delimiter ) )
		 {
			 cols--;
		 }
	 }
	 infile.close();
	 
	 if( ( lines == 0 ) || 
		 ( ( CheckColumns ) && ( cols < 2 ) ) )
	 {
	  printf("\nThe library file %s is empty or has a wrong format.", FileName);
	  printf("\nLines = %ld; Columns = %d.", lines, cols);
	  printf("\n");
	  out = false;
	 }
	 *Lines = lines;
	 *Columns = cols;
	 
	 return out;
}




// Reads a line of data using getline()
bool GetFileInfo2(FILE * pFile, unsigned long * Lines, unsigned int * Columns, bool CheckColumns )
{
	 bool out = true;

	 if(pFile == NULL)
	 {
		  printf("\nFile to be opened doesn't exist\n");
		  return false;
	 }
	 rewind(pFile);

	 unsigned int cols = 0;
	 unsigned long lines = 0;

	 // now find the number of classes counting the class labels
	 char string[100000];
	 unsigned long temp = 0;
	 while( !feof(pFile) )
	 {
		 fgets(string , 100000 , pFile);
		 temp = strlen(string);
		 if(temp > 0)
		 {
			 if((string[0] == '\r') || (string[0] == '\n')) continue;
			 else lines++;
		 }
	 }
	 rewind(pFile);

	 while( !feof(pFile) )
	 {
		 fgets(string , 100000 , pFile);
		 temp = strlen(string);
		 if(temp > 0)
		 {
			 if((string[0] == '\r') || (string[0] == '\n')) continue;
			 else break;
		 }
	 }

	 for(unsigned long i = 0; i < temp; i++)
	 {
		 if( string[i] == Delimiter ) cols++;
	 }
	 cols++;
	 if(temp > 1)
	 {
		 if((string[temp-2] == Delimiter) || (string[temp-1] == Delimiter)) cols--;
	 }
	 if(temp > 2)
	 {
		 if((string[temp-2] == '\r') && (string[temp-3] == Delimiter)) cols--;
	 }
	 else return false;

	 if( ( lines == 0 ) || ( ( CheckColumns ) && ( cols < 2 ) ) )
	 {
		 printf("\nThe library file is empty or has a wrong format.");
		 printf("\nLines = %ld; Columns = %d.", lines, cols);
		 printf("\n");
		 out = false;
	 }
	 *Lines = lines;
	 *Columns = cols;

	 return out;
}
