#ifndef CLIBRARY_H_
#define CLIBRARY_H_

#include <cstdio>


// This class represents an internal data library in kNN classifiers
class CLibrary
{
private:
	void Init();					// Default initialization of the main fields
	void ReadData(FILE * pFile); // Read data from a text file (the file should be opened for reading)  
	
protected:
	bool IsReady;					// Flag: is the data matrix filled with data
	unsigned long LibSize;			// The number of instances in the library
	unsigned int NumberOfClasses;	// The number of classes
	unsigned int VectorSize;		// The size of each instance (length of the vector)
	long * ClassLabels;				// Array of class labels
	float * LibMatrix;				// Internal pseudo 2D data matrix
	char LibFileName[500];			// Name of the library file
	
public:
	CLibrary(); // Constructor 1  (empty)
	CLibrary(const char * FileName);	// Constructor 2 (data file name)
	CLibrary(FILE * pFile, fpos_t InitPos, const unsigned long N_Lines, const unsigned int NumberOfClasses); // Constructor 3 (data segment into the config file)
	CLibrary(const unsigned long N_Lines, const unsigned int VectorSize, const unsigned int N_Classes); // Constructor 4 (creates an empty library)
	CLibrary(CLibrary * ExtLib); // Constructor 5 (copy)
	CLibrary(FILE * LibFile);	// Constructor 6 (data file)
	virtual ~CLibrary();	// Destructor
	bool ReassignLibrary(const char * FileName);	// Reallocate all internal arrays for modified data file
	bool ReassignLibrary(FILE * pFile);	// Reallocate all internal arrays for modified data file
	virtual void PrintOutLibrary(); // Print out the library's content
	void CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CLibrary * pDestLib = NULL); // Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
	bool SaveToFile(FILE * pFile); // Save content of the library to the text file (returned true if successful)
	bool SaveToFile(const char * FileName); // Save content of the library to the text file (returned true if successful)
	void NormalizeLibrary(); // Normalize vectors containing in the library

	// Properties
	virtual void GetInternalLine(const unsigned long LineNumber, float ** Vector, int * Label); // This function gets access to a particular Library vector and its class label via the number in the Library
	void SetInternalLine(const unsigned long LineNumber, float * Vector, int Label); // Sets the content of one internal line in the library
	void SetInternalLine(const unsigned long LineNumber, double * Vector, int Label); // Sets the content of one internal line in the library
	unsigned long GetLibSize() { return LibSize; }  // Get the library size
	unsigned int GetVectorSize() {return VectorSize;}	// Get the feature vector size
	unsigned int GetNumberOfClasses(); // Gets the number of classes
	virtual void GetInternalLib(float ** LibMatrix, long ** ClassLabels); // Get a reference to the internal data matrix and the array of class labels
	long * GetClassLabels() {return ClassLabels;} // Gets the array of internal class labels
	bool IsLibReady() {return IsReady;}				// Get a ready state of the library
	char * GetLibFileName() {return LibFileName;}	// Get the library data filename.
	virtual unsigned int GetLibVersion() {return 1;}	// Version = 1 for the old library class
	virtual float GetSqEuclidDistance(const unsigned long i, const unsigned long k); // Gets Squared Euclidean Distance between two vectors at given positions in the library
	virtual float GetManhattanDistance(const unsigned long i, const unsigned long k); // Gets Manhattan Distance between two vectors at given positions in the library
	virtual float GetLibValue(const unsigned long LineID, const unsigned int FeatureID); // Get a value of the internal matrix at the given position
	long GetLabel(const unsigned long LineID); // Get the class label it the given line
	void SetLabel(const unsigned long LineID, long ClassLabel); // Reset class label in the given line
	void SetValue(const unsigned long LineID, const unsigned int ColumnID, const float Value); // Sets a new value into the desired position of the library
};



#endif /*CLIBRARY_H_*/

