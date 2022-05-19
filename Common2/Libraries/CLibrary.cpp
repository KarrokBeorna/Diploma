
#include <cstdlib>

#include "GeneralSettings.h"
#include "SecondFileInfo.h"
#include "../FunctionTemplates.h"

#include "CLibrary.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   ______________________________   Class CLibrary   ____________________________________________________
// ************************************************************************************************************
// ************************************************************************************************************


//   ______________________________   public methods  ____________________________________________

// Constructor 1 (empty)
CLibrary::CLibrary()
{
	Init();
}


// Constructor 2 (data file name)
CLibrary::CLibrary(const char * FileName)
{
	Init();
	strcpy(LibFileName, FileName);
	if(ReassignLibrary(FileName)) IsReady = true;
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	IsReady = true;
}




// Constructor 3 (data segment into the config file)
CLibrary::CLibrary(FILE * pFile, fpos_t InitPos, const unsigned long N_Lines, const unsigned int NumberOfClasses)
{
	Init();
	if(pFile == NULL)
	{
		printf("\nUnable to read the library data from the info file. Halted...");
		return;
	}
	fsetpos(pFile, &InitPos);
	this->LibSize = N_Lines;
	this->NumberOfClasses = NumberOfClasses;
	
	char TmpStr[10000];
	fgets(TmpStr, 10000, pFile);
	unsigned int temp = strlen(TmpStr);
	if(temp == 1)
	{
		fgets(TmpStr, 10000, pFile);
		temp = strlen(TmpStr);
	}
	unsigned int cols = 0;
	for(unsigned long i = 0; i < temp; i++) if( TmpStr[ i ] == Delimiter ) cols++;
	cols++;

	if (temp > 1)
	{
		if ((TmpStr[temp - 2] == Delimiter) || (TmpStr[temp - 1] == Delimiter)) cols--;
	}
	if (temp > 2)
	{
		if ((TmpStr[temp - 2] == '\r') && (TmpStr[temp - 3] == Delimiter)) cols--;
	}

	this->VectorSize = cols - 1; // the first column is a class label, not a data field
	if(VectorSize <= 0)
	{
		printf("\nWrong number of columns (%d) of the class library into the info file. Halted...", VectorSize);
		return;
	}
	fsetpos(pFile, &InitPos);
	
	LibMatrix = (float *) calloc(LibSize * VectorSize, sizeof(float));
	ClassLabels = (long *) calloc(LibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	
	ReadData(pFile);
	IsReady = true;
}



// Constructor 4 (creates an empty library)
CLibrary::CLibrary(const unsigned long N_Lines, const unsigned int VectorSize, const unsigned int N_Classes)
{
	Init();
	this->LibSize = N_Lines;
	this->VectorSize = VectorSize;
	this->NumberOfClasses = N_Classes;
	LibMatrix = (float *) calloc(LibSize * VectorSize, sizeof(float));
	ClassLabels = (long *) calloc(LibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	IsReady = true;
}


// Constructor 5 (copy)
CLibrary::CLibrary(CLibrary * ExtLib)
{
	Init();
	this->IsReady = ExtLib->IsReady;
	if(!IsReady) return;
	this->LibSize = ExtLib->LibSize;
	this->VectorSize = ExtLib->VectorSize;
	this->NumberOfClasses = ExtLib->NumberOfClasses;
	unsigned long data_size = LibSize * VectorSize;
	LibMatrix = (float *) calloc(data_size, sizeof(float));
	ClassLabels = (long *) calloc(LibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	float * ExtData;
	long * ExtLabels;
	ExtLib->GetInternalLib(&ExtData, &ExtLabels);
	for(unsigned long i = 0; i < data_size; i++) LibMatrix[i] = ExtData[i];
	for(unsigned long i = 0; i < LibSize; i++) ClassLabels[i] = ExtLabels[i];
	IsReady = true;
}




// Constructor 6 (data file)
CLibrary::CLibrary(FILE * LibFile)
{
	Init();
	if(ReassignLibrary(LibFile)) IsReady = true;
	if((LibMatrix == NULL) || (ClassLabels == NULL)) IsReady = false;
}



// Destructor
CLibrary::~CLibrary()
{
	if(ClassLabels != NULL) free(ClassLabels);
	if(LibMatrix != NULL) free(LibMatrix);
}


	

// Reallocate all internal arrays for modified data file
bool CLibrary::ReassignLibrary(const char * FileName)
{
	FILE * pFile = fopen ( FileName , "r" );
	if(pFile == NULL)
	{
		printf("\nUnable to open the library file %s. Halted...", FileName);
		return false;
	}
	
	bool b = ReassignLibrary(pFile);
	fclose(pFile);
	return b;
}
		


// Reallocate all internal arrays for modified data file
bool CLibrary::ReassignLibrary(FILE * pFile)
{
	unsigned int cols = 0;
	unsigned long lines = 0;
	if(!GetFileInfo2(pFile, &lines, &cols)) return false;
	cols--;

	if(pFile == NULL)
	{
		printf("\nUnable to open the library file. Halted...");
		return false;
	}
	// now find the number of classes counting the class labels
	rewind(pFile);
	int ClassLabel = -1;
	unsigned int NumberOfClasses = 0;
	int MaxClassID = -1;
	char string[100000];
	while (!feof(pFile))
	{
		int iTmp;
		fscanf (pFile, "%d", &iTmp);
		/*
		if(iTmp != ClassLabel)
		{
			NumberOfClasses++;
			ClassLabel = iTmp;
		}
		*/
		if(iTmp > MaxClassID) MaxClassID = iTmp;
		fgets(string , 100000 , pFile);
	}
	rewind(pFile);
	NumberOfClasses = MaxClassID + 1;

	bool NeedMatrixRealloc = false;
	bool NeedClassLabelsRealloc = false;
	if(this->NumberOfClasses != NumberOfClasses)
	{
		NeedMatrixRealloc = true;
		this->NumberOfClasses = NumberOfClasses;
	}
	if(this->VectorSize != cols)
	{
		NeedMatrixRealloc = true;
		this->VectorSize = cols;
		if(VectorSize <= 1)
		{
			printf("\nWrong number of columns (%d) into the the library data file. Halted...", VectorSize);
			return false;
		}
	}
	if(this->LibSize != lines)
	{
		NeedMatrixRealloc = true;
		NeedClassLabelsRealloc = true;
		this->LibSize = lines;
	}

	// realloc internal arrays
	if(NeedMatrixRealloc)
	{
		if(LibMatrix == NULL) LibMatrix = (float *) calloc(LibSize * VectorSize, sizeof(float));
		else LibMatrix = (float *) realloc(LibMatrix, sizeof(float) * LibSize * VectorSize);
	}
	if(NeedClassLabelsRealloc)
	{
		if(ClassLabels == NULL) ClassLabels = (long *) calloc(LibSize, sizeof(long));
		else ClassLabels = (long *) realloc(ClassLabels, sizeof(long) * LibSize);
	}
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the method CLibrary::ReassignLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return false;
	}

	ReadData(pFile);
	return true;
}







// Print out the library's content
void CLibrary::PrintOutLibrary()
{
	if(!this->IsReady)
	{
		printf("\nUnable to print out the library's content: the library is not ready.");
		return;
	}
	if(strlen(LibFileName) > 0) printf("\nPrinting out the library created on file %s", LibFileName);
	else printf("\nPrinting out the library");
	printf("\nNumber of lines = %ld. Vector size = %d. Number of classes = %d", LibSize, VectorSize, NumberOfClasses);
	for(unsigned long r = 0; r < LibSize; r++)
	{
		float * Vector;
		int ClassLabel;
		GetInternalLine(r, &Vector, &ClassLabel);
		printf("\n%ld: %d	", r, ClassLabel);
		for(unsigned int c = 0; c < VectorSize; c++) printf("%.4f	", Vector[c]);
	}
	printf("\n");
}




// Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
void CLibrary::CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CLibrary * pDestLib)
{
	float * SourceVector;
	float * DestVector;
	int SourceClassLabel;
	int DestClassLabel;

	GetInternalLine(SourceLineID, &SourceVector, &SourceClassLabel);
	if(pDestLib == NULL)
	{
		GetInternalLine(DestLineID, &DestVector, &DestClassLabel);
		ClassLabels[DestLineID] = ClassLabels[SourceLineID];
	}
	else
	{
		pDestLib->GetInternalLine(DestLineID, &DestVector, &DestClassLabel);
		pDestLib->SetLabel(DestLineID, ClassLabels[SourceLineID]);
	}

	for(unsigned int v = 0; v < VectorSize; v++) DestVector[v] = SourceVector[v];
}



// Save content of the library to the text file (returned true if successful)
bool CLibrary::SaveToFile(FILE * pFile)
{
	if(pFile == NULL)
	{
		printf("\nError. Unable to save data into the file. Method CLibrary::SaveToFile.");
		return false;
	}

	float * Vector;
	int ClassLabel;
	for(unsigned long i = 0; i < LibSize; i++) if(ClassLabels[i] >= 0)
	{
		// fprintf(pFile, "\n%ld	", ClassLabels[i]);
		if(i == 0) fprintf(pFile, "%ld", ClassLabels[i]);
		else fprintf(pFile, "\n%ld", ClassLabels[i]);
		fprintf(pFile, Delim);
		GetInternalLine(i, &Vector, &ClassLabel);
		for(unsigned int n = 0; n < VectorSize; n++)
		{
			// fprintf(pFile, "%.8G	", Vector[n]);
			fprintf(pFile, Precision, Vector[n]);
			fprintf(pFile, Delim);
		}
	}
	return true;
}




// Save content of the library to the text file (returned true if successful)
bool CLibrary::SaveToFile(const char * FileName)
{
	FILE * LibFile = fopen(FileName, "w");
	if (LibFile == NULL) return false;
	bool b = SaveToFile(LibFile);
	fclose(LibFile);
	return b;
}



// Normalize vectors containing in the library
void CLibrary::NormalizeLibrary()
{
	float * Vector;
	int ClassLabel;
	for(unsigned long r = 0; r < LibSize; r++)
	{
		GetInternalLine(r, &Vector, &ClassLabel);
		AuxTemplates::Normalization(Vector, VectorSize, false);
	}
}



//______________________________________ Properties ________________________________________________


// This function gets access to a particular Library vector and its class label via the number in the Library
void CLibrary::GetInternalLine(const unsigned long LineNumber, float ** Vector, int * Label)
{
	if(LineNumber >= LibSize)
	{
		*Vector = NULL;
		*Label = -1;
		printf("\nWrong line number (out of the library size)");
	}
	else
	{
		*Vector = &LibMatrix[LineNumber * VectorSize];
		*Label = ( int )this->ClassLabels[LineNumber];
	}
}




// Sets the content of one internal line in the library
void CLibrary::SetInternalLine(const unsigned long LineNumber, float * Vector, int Label)
{
	if(LineNumber >= LibSize) printf("\nWrong line number (out of the library size)");
	else
	{
		ClassLabels[LineNumber] = Label;
		int LibClassLabel;
		float * LibVector;
		GetInternalLine(LineNumber, &LibVector, &LibClassLabel);
		for(unsigned int v = 0; v < VectorSize; v++) LibVector[v] = Vector[v];
	}
}






// Sets the content of one internal line in the library
void CLibrary::SetInternalLine(const unsigned long LineNumber, double * Vector, int Label)
{
	if (LineNumber >= LibSize) printf("\nWrong line number (out of the library size)");
	else
	{
		ClassLabels[LineNumber] = Label;
		int LibClassLabel;
		float * LibVector;
		GetInternalLine(LineNumber, &LibVector, &LibClassLabel);
		for (unsigned int v = 0; v < VectorSize; v++) LibVector[v] = Vector[v];
	}
}







// Gets the number of classes
unsigned int CLibrary::GetNumberOfClasses()
{
	if(NumberOfClasses > 0) return NumberOfClasses;
	int MaxClassLabel = 0;
	for(unsigned long i = 0; i < LibSize; i++) if(ClassLabels[i] > MaxClassLabel) MaxClassLabel = ClassLabels[i];
	NumberOfClasses = MaxClassLabel + 1;
	return NumberOfClasses;
}




// Get a reference to the internal data matrix and the array of class labels
void CLibrary::GetInternalLib(float ** LibMatrix, long ** ClassLabels)
{
	*LibMatrix = this->LibMatrix;
	*ClassLabels = this->ClassLabels;	
}



// Gets Squared Euclidean Distance between two vectors at given positions in the library
float CLibrary::GetSqEuclidDistance(const unsigned long i, const unsigned long k)
{
	if(i == k) return 0.0;
	float * AVector = &LibMatrix[i * VectorSize];
	float * CurrentVector = &LibMatrix[k * VectorSize];
	float D;
	AuxTemplates::SquaredDistance(CurrentVector, AVector, VectorSize, &D);
	return D;
}



// Gets Manhattan Distance between two vectors at given positions in the library
float CLibrary::GetManhattanDistance(const unsigned long i, const unsigned long k)
{
	if(i == k) return 0.0;
	float * AVector = &LibMatrix[i * VectorSize];
	float * CurrentVector = &LibMatrix[k * VectorSize];
	float D;
	AuxTemplates::ManhattanDistance(CurrentVector, AVector, VectorSize, &D);
	return D;
}



// Get a value of the internal matrix at the given position
float CLibrary::GetLibValue(const unsigned long LineID, const unsigned int FeatureID)
{
	return LibMatrix[LineID * VectorSize + FeatureID];
}


// Get the class label it the given line
long CLibrary::GetLabel(const unsigned long LineID)
{
	return ClassLabels[LineID];
}


// Reset class label in the given line
void CLibrary::SetLabel(const unsigned long LineID, long ClassLabel)
{
	ClassLabels[LineID] = ClassLabel;
}



// Sets a new value into the desired position of the library
void CLibrary::SetValue(const unsigned long LineID, const unsigned int ColumnID, const float Value)
{
	LibMatrix[ColumnID + LineID * VectorSize] = Value;
}



//______________________________________ private methods ________________________________________________


// Default initialization of the main fields
void CLibrary::Init()
{
	IsReady = false;
	ClassLabels = NULL;
	LibMatrix = NULL;
	LibSize = 0;
	VectorSize = 0;
	NumberOfClasses = 0;
	sprintf(LibFileName, "%s", "");
}




// Read data from a text file (the file should be opened for reading)
void CLibrary::ReadData(FILE * pFile)
{
	long iTmp;
	float dTmp;
	for(unsigned long i = 0; i < LibSize; i++)
	{
		fscanf (pFile, "%ld", &iTmp);
		ClassLabels[i] = iTmp;
		for(unsigned int n = 0; n < VectorSize; n++)
		{
			fscanf (pFile, "%G", &dTmp);
			LibMatrix[n + i * VectorSize] = dTmp; 
		}
	}
}


