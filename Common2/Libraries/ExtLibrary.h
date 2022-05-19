#ifndef ADVANCEDLIB_H_
#define ADVANCEDLIB_H_

#include "CLibrary.h"


// ************************************************************************************************************
// ************************************************************************************************************
//   ______________________________   Class CCoordinates  _____________________________________________________
// ************************************************************************************************************
// ************************************************************************************************************


// An internal Data structure holding informmation about frame and channel values related to each sample in the library 
class CCoordinates
{
private:
	unsigned long LibSize; // Library size (number of samples)
	unsigned long * Frames; // Array of frames
	unsigned long * Channels; // Array of channel values
	
public:
	CCoordinates(const unsigned long LibSize); // Constructor
	virtual ~CCoordinates();	// Destructor
	
	void Realloc(const unsigned long NewLibSize); // Reallocate the internal arrays if necessary
	void AutoFillCoordinates();	// Substitute dummy values to the arrays
	unsigned long GetFrame(const unsigned long ID);	// Get a particular frame value related to some sample ID
	unsigned long GetChannel(const unsigned long ID); // Get a particular channel value related to some sample ID
	void SetFrame(const unsigned long ID, const unsigned long Frame); // Set a new frame value related to some sample ID
	void SetChannel(const unsigned long ID, const unsigned long Channel); // Set a new channel value related to some sample ID
	unsigned long GetLibSize();	// Get the size of the internal library (i.e. the number of samples)
};




// ************************************************************************************************************
// ************************************************************************************************************
//   ______________________________   Class CExtLibrary  _____________________________________________________
// ************************************************************************************************************
// ************************************************************************************************************


// This is descendant class of the CLibrary allowing to store either class labels and time-spatial coordinates.
// Two file formats are supported. 
class CExtLibrary : public CLibrary
{
	private:
		CCoordinates * Coords; // This object holds time-spatial coordinates (frames and channels)
		
		int GetFileFormat(const char * FileName, unsigned long * DataLines, unsigned int * DataCols, bool * WithCoordinates, bool * WithLabels); // Determine the format of the input data file and find other information about the file
		unsigned long FindNumberOfClasses(); // Calculate the number of classes stored in the library
		int DataFormat; // Data format of the library. 0: old (simple), 1: new (with info symbols) 
		
	public:
		bool WithLabels;		// Flag: are class labels stored in the file
		bool WithCoordinates;	// Flag: are time-spatial coordinates stored in the file

		CExtLibrary(const char * FileName);	// Constructor 1 (data file name)
		CExtLibrary(const unsigned long LibSize, const unsigned int VectorSize); // Constructor 2 (empty library)
		~CExtLibrary();				// Destructor
		bool ReassignLibrary(const char * FileName);	// Reallocate all internal arrays for modified data file (overwritten)
		
		float * GetInternalLine(const unsigned long LineNumber); // Get the reference to a given internal line of data
		void PrintOutLibrary();				// Print out the library's content
		void ResetLibValue(const unsigned long LineID, const unsigned int FeatureID, const float NewValue); // Reset one value in the internal matrix
		void ResetClassLabel(const unsigned long LineID, const long NewValue); // Reset one class label at a given line
		void ResetFrameValue(const unsigned long LineID, const unsigned long NewValue); // Reset frame value at a given line
		void ResetChannelValue(const unsigned long LineID, const unsigned long NewValue); // Reset channel value at a given line
		unsigned long GetFrameID(const unsigned long LineID); // Get the frame value for a given sample ID
		unsigned long GetChannelID(const unsigned long LineID); // Get the channel value for a given sample ID
		int GetDataFormat() {return this->DataFormat;} // Get the Format of the Data file: 0 - old format; 1 - new format
		CExtLibrary * CopyLibrary(const unsigned int ReducedIn_N_Times = 1); // Create a new instance of the object by copying it
		unsigned long RecalculateNumberOfClasses();	// Calculate the number of classes again. If there is no class labels, the number of classes is supposed = 1
		void SetDataFormat(const unsigned int Format); // Set the format of the library data file (0 - old format; 1 - new data format with characters)
		bool WriteLibraryIntoFile(const char * OutputFile); // Write the library into a text file
		void ReallocLibrary(const unsigned long NewLibSize, const unsigned int NewVectorSize); // Reallocate memory for the library if its size was changed
		void AutoFillCoordinates();	// Substitute dummy values to the coordinates
		// float GetDistance(const unsigned long i, const unsigned long k); // Gets Manhattan distance between two vectors at given positions in the library
		void SetNumberOfClasses(const unsigned long PredefinedNumberOfClasses); // Set a predefined number of classes, if it is known.
		void ShuffleData();  // Random reordering the data instances
		void CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CExtLibrary * pDestLib); // Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
		// void CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CLibrary * pDestLib); // Copy a line (SourceLineID) from this library to the destination library (DestLineID). If pDestLib == NULL, then copy to the same library
		unsigned int GetLibVersion() {return 2;} // Version = 2 for the new (expanded) library class
};







#endif /*ADVANCEDLIB_H_*/
