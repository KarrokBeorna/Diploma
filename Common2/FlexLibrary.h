/*
 * FlexLibrary.h
 *
 *  Created on: Feb 26, 2012
 *      Author: artem khlybov
 */

#ifndef FLEXLIBRARY_H_
#define FLEXLIBRARY_H_


#include "CLibrary.h"

// The class expands the base class CLibrary. Here the number of lines and the vector size
// are flexible. An empty object can be created without actual data. It is useful to prevent
// numerous memory reallocations, if the library is frequently rewritten with new data. The
// class holds maximal possible number of lines and the vector size together with their
// actual values.

class CFlexLibrary : public CLibrary
{
private:  // HIDDEN methods
	CFlexLibrary(const char * FileName);
	CFlexLibrary(FILE * pFile, fpos_t InitPos, const unsigned long N_Lines, const unsigned int NumberOfClasses);
	CFlexLibrary(FILE * LibFile);

private:
	unsigned long MaxLibSize; // Maximal possible library size (i.e. the the number of lines)
	unsigned int MaxVectorSize; // Maximal possible vector size (i.e. the number of columns)

public:
	CFlexLibrary(const unsigned long N_Lines, const unsigned int VectorSize, const unsigned int N_Classes); // Constructor (creates an empty library of maximal volume)
	CFlexLibrary(CFlexLibrary * ExtLib); // Constructor (copy) 1
	CFlexLibrary(CLibrary * ExtLib, const unsigned long NewLibSize, const unsigned int NewVectorSize); // Constructor (copy) 2

	// Properties
	bool ResetLibSize(const unsigned long NewLibSize); // Resets the actual number of lines
	bool ResetVectorSize(const unsigned long NewVectorSize); // Resets the actual vector size
	void GetInternalLine(const unsigned long LineNumber, float ** Vector, int * Label); // This function gets access to a particular Library vector and its class label via the number in the Library
	float GetSqEuclidDistance(const unsigned long i, const unsigned long k); // Gets Squared Euclidean Distance between two vectors at given positions in the library
	float GetManhattanDistance(const unsigned long i, const unsigned long k); // Gets Manhattan Distance between two vectors at given positions in the library
	float GetLibValue(const unsigned long LineID, const unsigned int FeatureID); // Get a value of the internal matrix at the given position
	unsigned int GetLibVersion() {return 3;} // Version = 3 for the flexible library class
};




#endif /* FLEXLIBRARY_H_ */
