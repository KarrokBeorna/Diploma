
// #include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "../FunctionTemplates.h"
#include "GeneralSettings.h"
#include "ExtLibrary.h"  



// ************************************************************************************************************
// ************************************************************************************************************
//   ______________________________   Class CCoordinates  _____________________________________________________
// ************************************************************************************************************
// ************************************************************************************************************

//_________________________________   public methods  ______________________________________


// Constructor
CCoordinates::CCoordinates(const unsigned long LibSize)
{
	this->LibSize = LibSize;
	Frames = (unsigned long *) calloc(LibSize, sizeof(unsigned long));
	Channels = (unsigned long *) calloc(LibSize, sizeof(unsigned long));
}


// Destructor
CCoordinates::~CCoordinates()
{
	free(Frames);
	free(Channels);
}



// Reallocate the internal arrays if necessary
void CCoordinates::Realloc(const unsigned long NewLibSize)
{
	if(LibSize == NewLibSize) return;
	LibSize = NewLibSize;
	Frames = (unsigned long *) realloc(Frames, sizeof(unsigned long) * LibSize);
	Channels = (unsigned long *) realloc(Channels, sizeof(unsigned long) * LibSize);
}


// Substitute dummy values to the arrays
void CCoordinates::AutoFillCoordinates()
{
	unsigned long n_channels = ( unsigned long )floor(sqrt( (float) LibSize));
	unsigned long z = 0;
	unsigned long f = 0;
	while(z < LibSize)
	{
		
		for(unsigned long i = 0; i < n_channels; i++)
		{
			Frames[z] = f;
			Channels[z] = i;
			z++;
			if(z >= LibSize) break;
		}
		f++;
	}
}


// Get a particular frame value related to some sample ID
unsigned long CCoordinates::GetFrame(const unsigned long ID)
{
	return Frames[ID];
}



// Get a particular channel value related to some sample ID
unsigned long CCoordinates::GetChannel(const unsigned long ID)
{
	return Channels[ID];
}


// Set a new frame value related to some sample ID
void CCoordinates::SetFrame(const unsigned long ID, const unsigned long Frame)
{
	Frames[ID] = Frame;
}


// Set a new channel value related to some sample ID
void CCoordinates::SetChannel(const unsigned long ID, const unsigned long Channel)
{
	Channels[ID] = Channel;
}


// Get the size of the internal library (i.e. the number of samples)
unsigned long CCoordinates::GetLibSize()
{
	return this->LibSize;
}



// ************************************************************************************************************
// ************************************************************************************************************
//   ______________________________   Class CExtLibrary  _____________________________________________________
// ************************************************************************************************************
// ************************************************************************************************************

//_________________________________   public methods  ______________________________________



// Constructor 1 (data file name)
CExtLibrary::CExtLibrary(const char * FileName)
{
	ClassLabels = NULL;	
	LibMatrix = NULL;
	Coords = NULL;
	DataFormat = 0;
	LibSize = 0;
	VectorSize = 0;
	IsReady = false;
	
	if(!ReassignLibrary(FileName))
	{
		printf("\nUnable to open the library file %s. Halted...", FileName);
		return;
	}
	IsReady = true;
}



// Constructor 2 (empty library)
CExtLibrary::CExtLibrary(const unsigned long LibSize, const unsigned int VectorSize)
{
	ClassLabels = NULL;	
	LibMatrix = NULL;
	Coords = NULL;
	DataFormat = 0;
	this->LibSize = 0;
	this->VectorSize = 0;
	IsReady = false;

	if(LibSize <= 1)
	{
		printf("\nIncorrect library size (%ld)", LibSize);
		return;
	}
	if(VectorSize <= 1)
	{
		printf("\nIncorrect vector size in the library (%d)", VectorSize);
		return;
	}

	this->LibSize = LibSize;
	this->VectorSize = VectorSize;
	
	ClassLabels = (long *) calloc(LibSize, sizeof(long));	
	LibMatrix = (float *) calloc(LibSize * VectorSize, sizeof(float));
	
	Coords = new CCoordinates(LibSize);
	this->WithLabels = false;	
	this->WithCoordinates = false;	
	DataFormat = 1;
	IsReady = true;
}





// Destructor
CExtLibrary::~CExtLibrary()
{
	if(Coords != NULL) delete Coords;
}




// This function gets access to a particular data vector via the number in the internal matrix
float * CExtLibrary::GetInternalLine(const unsigned long LineNumber)
{
	float * Vector;
	if(LineNumber >= LibSize)
	{
		Vector = NULL;
		printf("\nWrong line number (out of the library size)");
	}
	else
	{
		Vector = &LibMatrix[LineNumber * VectorSize];
	}
	return Vector;
}


// Print out the library's content
void CExtLibrary::PrintOutLibrary()
{
	if(!this->IsReady)
	{
		printf("\nUnable to print out the library's content: the library is not ready.");
		return;
	}
	if(strlen(LibFileName) > 0) printf("\nPrinting out the library created on file %s", LibFileName);
	else printf("\nPrinting out the library");

	for(unsigned long r = 0; r < LibSize; r++)
	{
		if(this->WithCoordinates) printf("F: %ld	C: %ld	", Coords->GetFrame(r), Coords->GetChannel(r));
		if(this->WithLabels) printf("L: %ld	", ClassLabels[r]);
		for(unsigned int c = 0; c < VectorSize; c++)
		{
			printf("%.4f	", this->LibMatrix[c + r * VectorSize]);
		}
		printf("\n");
	}
	printf("\n");
}




// Update one value in the matrix corresponded to a particular line and row
void CExtLibrary::ResetLibValue(const unsigned long LineID, const unsigned int FeatureID, const float NewValue)
{
	LibMatrix[LineID * VectorSize + FeatureID] = NewValue;
}



// Update one value in the matrix corresponded to a particular line and row
void CExtLibrary::ResetClassLabel(const unsigned long LineID, const long NewValue)
{
	ClassLabels[LineID] = NewValue;
}


// Reset frame value at a given line
void CExtLibrary::ResetFrameValue(const unsigned long LineID, const unsigned long NewValue)
{
	Coords->SetFrame(LineID, NewValue); 
}


// Reset channel value at a given line
void CExtLibrary::ResetChannelValue(const unsigned long LineID, const unsigned long NewValue)
{
	Coords->SetChannel(LineID, NewValue); 
}


// Get the frame value for a given sample ID
unsigned long CExtLibrary::GetFrameID(const unsigned long LineID)
{
	return Coords->GetFrame(LineID);
}


// Get the channel value for a given sample ID
unsigned long CExtLibrary::GetChannelID(const unsigned long LineID)
{
	return Coords->GetChannel(LineID);
}


// Create a new instance of the object by copying it
CExtLibrary * CExtLibrary::CopyLibrary(const unsigned int ReducedIn_N_Times)
{
	unsigned long NewLibSize = LibSize;
	
	unsigned int inc = 1;
	if((ReducedIn_N_Times > 1)&&(ReducedIn_N_Times * 100 < LibSize)) 
	{
		inc = ReducedIn_N_Times; 
		NewLibSize = LibSize / inc;		
	}
	
	CExtLibrary * NewLibrary = new CExtLibrary(NewLibSize, VectorSize);
	unsigned long z = 0;
	for(unsigned long i = 0; i < LibSize; i += inc)
	{
		for(unsigned int n = 0; n < VectorSize; n++)
		{
			float temp = LibMatrix[n + i * VectorSize];
			NewLibrary->ResetLibValue(z,n,temp);
		}
		long ClassLabel = this->ClassLabels[i];
		NewLibrary->ResetClassLabel(z,ClassLabel);
		
		unsigned long frame = this->Coords->GetFrame(i);
		NewLibrary->ResetFrameValue(z,frame);

		unsigned long channel = this->Coords->GetChannel(i);
		NewLibrary->ResetChannelValue(z, channel);

		z++;
	}
	NewLibrary->WithLabels = this->WithLabels;
	NewLibrary->NumberOfClasses = this->NumberOfClasses;
	
	return NewLibrary;
}



// Calculate the number of classes again. If there is no class labels, the number of classes is supposed = 1
unsigned long CExtLibrary::RecalculateNumberOfClasses()
{
	NumberOfClasses = FindNumberOfClasses();
	return NumberOfClasses;
}



// Set a predefined number of classes, if it is known
void CExtLibrary::SetNumberOfClasses(const unsigned long PredefinedNumberOfClasses)
{
	NumberOfClasses = PredefinedNumberOfClasses;
}



// Set the format of the library data file (0 - old format; 1 - new data format with characters)
void CExtLibrary::SetDataFormat(const unsigned int Format)
{
	DataFormat = Format;
	if( (DataFormat < 0) || (DataFormat > 1) ) DataFormat = 0;
}




// Write the library into a text file
bool CExtLibrary::WriteLibraryIntoFile(const char * OutputFile)
{
	FILE * pFile = fopen ( OutputFile , "w" ); // creating the output file
	if(pFile == NULL) 
	{
		printf("\nFile %s doesn't exist", OutputFile);
		return false;
	}
	
	for(unsigned long i = 0; i < LibSize; i++)
	{
		switch (DataFormat)
		{
			case 0:  // by default  -- old format
				{
					if(WithLabels)
					{
						fprintf(pFile, "%ld", this->GetLabel(i));
						fprintf(pFile, Delim);
					}
					for(unsigned v = 0; v < VectorSize-1; v++)
					{
						fprintf(pFile, Precision, this->GetLibValue(i, v));
						fprintf(pFile, Delim);
					}
					fprintf(pFile, Precision, this->GetLibValue(i, VectorSize-1));
					break;
				}
				
			default:  // new format
				{
					if(WithCoordinates)
					{
						// fprintf(pFile, "f%ld	c%ld	", this->GetFrameID(i), this->GetChannelID(i));
						fprintf(pFile, "f%ld", this->GetFrameID(i));
						fprintf(pFile, Delim);
						fprintf(pFile, "f%ld", this->GetChannelID(i));
						fprintf(pFile, Delim);
					}
					if(WithLabels)
					{
						// fprintf(pFile, "l%ld	", this->GetLabel(i));
						fprintf(pFile, "l%ld", this->GetLabel(i));
						fprintf(pFile, Delim);
					}
					for(unsigned v = 0; v < VectorSize-1; v++)
					{
						// fprintf(pFile, "d%f	", this->GetLibValue(i, v));
						fprintf(pFile, "d");
						fprintf(pFile, Precision, this->GetLibValue(i, v));
						fprintf(pFile, Delim);
					}
					// fprintf(pFile, "d%f", this->GetLibValue(i, VectorSize-1));
					fprintf(pFile, "d");
					fprintf(pFile, Precision, this->GetLibValue(i, VectorSize-1));
				}
		}
		fprintf(pFile, "\n");
	}
	fclose(pFile);
	
	return true;
}




// Substitute dummy values to the coordinates
void CExtLibrary::AutoFillCoordinates()
{
	Coords->AutoFillCoordinates();
	this->WithCoordinates = true;
}




// Random reordering the data instances
void CExtLibrary::ShuffleData()
{
	float * D = (float *) calloc(VectorSize, sizeof(float));
	
	for(unsigned long i = 0; i < LibSize; i++)
	{
		unsigned long NewPos = rand() % LibSize;
		for(unsigned int k = 0; k < VectorSize; k++)
		{
			D[k] = LibMatrix[i * VectorSize + k];
			LibMatrix[i * VectorSize + k] = LibMatrix[NewPos * VectorSize + k];
			LibMatrix[NewPos * VectorSize + k] = D[k];
		}
		
		if(WithCoordinates)
		{
			unsigned long TempFrame = Coords->GetFrame(i);
			unsigned long TempChannel = Coords->GetChannel(i);
			ResetFrameValue(i, Coords->GetFrame(NewPos));
			ResetFrameValue(NewPos, TempFrame);
			ResetChannelValue(i, Coords->GetChannel(NewPos));
			ResetChannelValue(NewPos, TempChannel);
		}
		if(WithLabels)
		{
			long TempLabel = ClassLabels[i];
			ClassLabels[i] = ClassLabels[NewPos];
			ClassLabels[NewPos] = TempLabel;
		}
	}
	printf("\nLibrary %s was shuffled.", this->LibFileName);
	printf("\n");
}




//_________________________________   private methods  ______________________________________


// Determine the format of the input data file and find other information about the file
int CExtLibrary::GetFileFormat(const char * FileName, unsigned long * DataLines, unsigned int * DataCols, bool * WithCoordinates, bool * WithLabels)
{
	// 1. Check the presence of the file 
	FILE * pFile = fopen ( FileName , "r" );
	if (pFile == NULL)
	{
		printf("\nThe file %s doesn't exist\n", FileName);
		return -1;
	}
	
	long lines = 0;
	int cols = 0;
	bool WithL = false, WithC = false; 
	
	// 2. Find the number of lines 
	rewind(pFile);
	char string[100000];
	while (!feof(pFile))
	{
		fgets(string , 100000 , pFile);
		lines++;
	}
	lines--;
	if(lines <= 0)
	{
		printf("\nThe file %s is empty\n", FileName);
		fclose(pFile);
		return -1;
	}
	*DataLines = lines; 
	

	// 5. Is it old or new format?
	rewind(pFile);
	fgets (string , 100000 , pFile);
	int Format = 0;
	unsigned int temp = strlen(string);
	if(temp < 2)
	{
		fgets (string , 100000 , pFile);
		temp = strlen(string);
	}
	for(unsigned int i = 0; i < temp; i++) if(string[i] == 'd')
	{
		Format = 1;
		break;
	}
	
	switch (Format)
	{
		case 1:		// this is New Format
			{
				// 5. Find the number of data columns
				for(unsigned int i = 0; i < temp; i++) if(string[i] == 'd') cols++;
				if(cols == 0)
				{
					printf("\nThe file %s doesn't contain data cells\n", FileName);
					fclose(pFile);
					return -1;
				}
				*DataCols = cols;
				
				// 4. Do ClassLabels exist 
				for(unsigned int i = 0; i < temp; i++) if(string[i] == 'l') WithL = true;
				
				// 5. Do Coordinates exist 
				bool fr = false;
				bool ch = false;
				for(unsigned int i = 0; i < temp; i++)
				{
					if(string[i] == 'f') fr = true;
					if(string[i] == 'c') ch = true;
				}
				if(fr && ch) WithC = true;
			
				break;
			}
		default:		// this is Old Format
			{
				WithC = false;
				rewind(pFile);
				float D;
				fscanf (pFile, "%G", &D);
				if(D == floor(D)) WithL = true;
				if(D < -2)  WithL = false;
				
				const char Delimiter = '	';
				
				cols = 1;
				for(unsigned int i = 0; i < temp; i++) if(string[i] == Delimiter) cols++;
				if(temp>2)
				{
					if(string[temp-1] == Delimiter) cols--;
					else
					{
						if(string[temp-2] == Delimiter) cols--;
					}
				}
				// printf("\n");
				// printf("\n%c%c%c", string[temp-2], string[temp-1], string[temp]);
				// printf("\n");
				if(WithL) cols--;
			}
	}
	
	*DataCols = cols;
	*WithLabels = WithL;
	*WithCoordinates = WithC; 

	fclose(pFile);
	return Format;
}






// Reallocate all internal arrays for modified data file
bool CExtLibrary::ReassignLibrary(const char * FileName)
{
	// 1. Get preliminary info from the file
	unsigned long lines = 0;
	unsigned int cols = 0;
	DataFormat = GetFileFormat(FileName, &lines, &cols, &WithCoordinates, &WithLabels); 
	if(DataFormat == -1) return false;
	strcpy(LibFileName, FileName);
	bool NeedLabelsRealloc = false;
	if(lines != LibSize) NeedLabelsRealloc = true;
	
	ReallocLibrary(lines, cols);
	
	// 2. Fill internal arrays
	FILE * pFile = fopen ( FileName , "r" );
	if(pFile == NULL)
	{
		printf("\nUnable to open the library file %s. Halted...", FileName);
		return false;
	}
	
	char string[100000];
	char part[100000];
	long iTmp;
	float dTmp;
	rewind(pFile);
	
	switch (DataFormat)
	{
		case 1:	// New Format
			{
				for(unsigned long lines = 0; lines < this->LibSize; lines++)
				{
					fgets(string , 100000 , pFile);
					cols = 0;
					char * pch = strchr(string,'d');
					strcpy(part, pch);
					char * tmp = strtok(part, " 	");
					while (tmp != NULL)
					{
						dTmp = atof(&tmp[1]);
						tmp = strtok (NULL, " 	");
			 			LibMatrix[cols + lines * VectorSize] = dTmp;
						cols++;
					}
					
					pch = strchr(string,'f');
					if(pch != NULL)
					{
						strcpy(part, pch);
						char * tmp = strtok(part, " 	");
						iTmp = atoi(&tmp[1]);
						Coords->SetFrame(lines, iTmp); 
					}

					pch = strchr(string,'c');
					if(pch != NULL)
					{
						strcpy(part, pch);
						char * tmp = strtok(part, " 	");
						iTmp = atoi(&tmp[1]);
						Coords->SetChannel(lines, iTmp); 
					}

					pch = strchr(string,'l');
					if(pch != NULL)
					{
						strcpy(part, pch);
						char * tmp = strtok(part, " 	");
						iTmp = atoi(&tmp[1]);
						this->ClassLabels[lines] = iTmp;
					}
				}			
				break;
			}
			
		default: 	// Old Format
			{
				for(unsigned long i = 0; i < this->LibSize; i++)
				{
					if(WithLabels)
					{
						fscanf (pFile, "%ld", &iTmp);
						ClassLabels[i] = iTmp;
					}
					for(unsigned int n = 0; n < VectorSize; n++)
					{
						fscanf (pFile, "%G", &dTmp);
						LibMatrix[n + i * VectorSize] = dTmp;
					}
				}		
			}
	}
	
	fclose(pFile);
	if(NeedLabelsRealloc) NumberOfClasses = FindNumberOfClasses();

	return true;
}


// Reallocate memory for the library if its size was changed
void CExtLibrary::ReallocLibrary(const unsigned long NewLibSize, const unsigned int NewVectorSize)
{
	bool NeedMatrixRealloc = false;
	bool NeedLabelsRealloc = false;
	bool NeedCoordsRealloc = false;

	if(NewLibSize != LibSize)
	{
		NeedMatrixRealloc = true;
		NeedLabelsRealloc = true;
		NeedCoordsRealloc = true;
		this->LibSize = NewLibSize;
	}
	
	if(NewVectorSize != VectorSize)
	{
		NeedMatrixRealloc = true;
		this->VectorSize = NewVectorSize;
	}
	
	if(NeedMatrixRealloc)
	{
		if(LibMatrix == NULL)
		{
			LibMatrix = (float *) calloc(LibSize * VectorSize, sizeof(float));	
		}
		else
		{
			LibMatrix = (float *) realloc(LibMatrix, sizeof(float) * LibSize * VectorSize);
		}
	}
	
	if(NeedLabelsRealloc)
	{
		if(ClassLabels == NULL)
		{
			ClassLabels = (long *) calloc(LibSize, sizeof(long));	
		}
		else
		{
			ClassLabels = (long *) realloc(ClassLabels, sizeof(long) * LibSize);
		}
	}

	if(NeedCoordsRealloc)
	{
		if(Coords == NULL) Coords = new CCoordinates(LibSize);
		else Coords->Realloc(LibSize);
	}
}




// Calculate the number of classes stored in the library
unsigned long CExtLibrary::FindNumberOfClasses()
{
	if(!WithLabels) return 1;
	long count = 0;
	AuxTemplates::Maximum(ClassLabels, LibSize, &count);
	return count+1;
}
	



// Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
void CExtLibrary::CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CExtLibrary * pDestLib)
{
	float * SourceVector;
	float * DestVector;

	SourceVector = GetInternalLine(SourceLineID);
	if(pDestLib == NULL)
	{
		DestVector = GetInternalLine(DestLineID);
		if(WithLabels) ClassLabels[DestLineID] = ClassLabels[SourceLineID];
		if(WithCoordinates)
		{
			unsigned long frame = GetFrameID(SourceLineID);
			unsigned long channel = GetChannelID(SourceLineID);
			ResetFrameValue(DestLineID, frame);
			ResetChannelValue(DestLineID, channel);
		}
	}
	else
	{
		DestVector = pDestLib->GetInternalLine(DestLineID);
		if(WithLabels) pDestLib->SetLabel(DestLineID, ClassLabels[SourceLineID]);
		if(WithCoordinates)
		{
			unsigned long frame = GetFrameID(SourceLineID);
			unsigned long channel = GetChannelID(SourceLineID);
			pDestLib->ResetFrameValue(DestLineID, frame);
			pDestLib->ResetChannelValue(DestLineID, channel);
		}
	}

	for(unsigned int v = 0; v < VectorSize; v++) DestVector[v] = SourceVector[v];
}



/*
// Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
void CExtLibrary::CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CLibrary * pDestLib)
{
	float * SourceVector;
	float * DestVector;

	SourceVector = GetInternalLine(SourceLineID);
	if(pDestLib == NULL)
	{
		DestVector = GetInternalLine(DestLineID);
		if(WithLabels) ClassLabels[DestLineID] = ClassLabels[SourceLineID];
		if(WithCoordinates)
		{
			unsigned long frame = GetFrameID(SourceLineID);
			unsigned long channel = GetChannelID(SourceLineID);
			ResetFrameValue(DestLineID, frame);
			ResetChannelValue(DestLineID, channel);
		}
	}
	else
	{
		int DestLabel;
		pDestLib->GetInternalLine(DestLineID, &DestVector, &DestLabel);
		if(WithLabels) pDestLib->SetLabel(DestLineID, ClassLabels[SourceLineID]);
	}

	for(unsigned int v = 0; v < VectorSize; v++) DestVector[v] = SourceVector[v];
}
*/

