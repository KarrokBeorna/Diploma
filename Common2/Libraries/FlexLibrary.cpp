/*
 * FlexLibrary.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: artem
 */

#include <cstdlib>
#include "../FunctionTemplates.h"

#include "FlexLibrary.h"



// ************************************************************************************************************
//   ______________________________   class CFlexLibrary   ___________________________________________
// ************************************************************************************************************


//   ______________________________   public methods   __________________________________________________


// Constructor (creates an empty library of maximal volume)
CFlexLibrary::CFlexLibrary(const unsigned long N_Lines, const unsigned int VectorSize, const unsigned int N_Classes) : CLibrary()
{
	this->MaxLibSize = N_Lines;
	this->LibSize = N_Lines;
	this->MaxVectorSize = VectorSize;
	this->VectorSize = VectorSize;
	this->NumberOfClasses = N_Classes;

	LibMatrix = (float *) calloc(MaxLibSize * MaxVectorSize, sizeof(float));
	ClassLabels = (long *) calloc(MaxLibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	IsReady = true;
}



// Constructor (copy) 1
CFlexLibrary::CFlexLibrary(CFlexLibrary * ExtLib) : CLibrary()
{
	this->IsReady = ExtLib->IsReady;
	if(!IsReady) return;
	this->LibSize = ExtLib->LibSize;
	this->VectorSize = ExtLib->VectorSize;
	this->NumberOfClasses = ExtLib->NumberOfClasses;
	this->MaxLibSize = ExtLib->MaxLibSize;
	this->MaxVectorSize = ExtLib->MaxVectorSize;
	unsigned long data_size = MaxLibSize * MaxVectorSize;
	LibMatrix = (float *) calloc(data_size, sizeof(float));
	ClassLabels = (long *) calloc(MaxLibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	float * ExtData;
	long * ExtLabels;
	ExtLib->GetInternalLib(&ExtData, &ExtLabels);
	for(unsigned long i = 0; i < data_size; i++) LibMatrix[i] = ExtData[i];
	for(unsigned long i = 0; i < MaxLibSize; i++) ClassLabels[i] = ExtLabels[i];
	IsReady = true;
}




// Constructor (copy) 2
CFlexLibrary::CFlexLibrary(CLibrary * ExtLib, const unsigned long NewLibSize, const unsigned int NewVectorSize) : CLibrary()
{
	this->IsReady = ExtLib->IsLibReady();
	if(!IsReady) return;
	if(NewLibSize > ExtLib->GetLibSize())
	{
		printf("\nError in the copy constructor of CFlexLibrary: original library has less number of elements than required (%ld instead of %ld)", NewLibSize,  ExtLib->GetLibSize());
		IsReady = false;
		return;
	}
	if(NewVectorSize > ExtLib->GetVectorSize())
	{
		printf("\nError in the copy constructor of CFlexLibrary: original library has less vector size than required (%d instead of %d)", NewVectorSize,  ExtLib->GetVectorSize());
		IsReady = false;
		return;
	}
	this->LibSize = NewLibSize;
	this->VectorSize = NewVectorSize;
	this->NumberOfClasses = ExtLib->GetNumberOfClasses();
	this->MaxLibSize = NewLibSize;
	this->MaxVectorSize = NewVectorSize;
	unsigned long data_size = MaxLibSize * MaxVectorSize;
	LibMatrix = (float *) calloc(data_size, sizeof(float));
	ClassLabels = (long *) calloc(MaxLibSize, sizeof(long));
	if((LibMatrix == NULL) || (ClassLabels == NULL))
	{
		printf("\nError during memory allocation in the constructor of CFlexLibrary. LibMatrix=%p, ClassLabels=%p", LibMatrix, ClassLabels);
		return;
	}
	float * ExtData;
	long * ExtLabels;
	ExtLib->GetInternalLib(&ExtData, &ExtLabels);
	unsigned int OldVectorSize = ExtLib->GetVectorSize();
	for(unsigned long i = 0; i < NewLibSize; i++)
	{
		float * AVectorNew = &LibMatrix[i * VectorSize];
		float * AVectorOld = &ExtData[i * OldVectorSize];
		for(unsigned int c = 0; c < NewVectorSize; c++) AVectorNew[c] = AVectorOld[c];
		ClassLabels[i] = ExtLabels[i];
	}
	IsReady = true;
}





//   ______________________________   Properties   __________________________________________________

// Resets the actual number of lines
bool CFlexLibrary::ResetLibSize(const unsigned long NewLibSize)
{
	bool b = false;
	if(NewLibSize > MaxLibSize) LibSize = MaxLibSize;
	else
	{
		LibSize = NewLibSize;
		b = true;
	}
	return b;
}



// Resets the actual vector size
bool CFlexLibrary::ResetVectorSize(const unsigned long NewVectorSize)
{
	bool b = false;
	if(NewVectorSize > MaxVectorSize) VectorSize = MaxVectorSize;
	else
	{
		VectorSize = NewVectorSize;
		b = true;
	}
	return b;
}




// This function gets access to a particular Library vector and its class label via the number in the Library
void CFlexLibrary::GetInternalLine(const unsigned long LineNumber, float ** Vector, int * Label)
{
	*Vector = &LibMatrix[LineNumber * MaxVectorSize];
	*Label = ( int )this->ClassLabels[LineNumber];
}



// Gets Squared Euclidean Distance between two vectors at given positions in the library
float CFlexLibrary::GetSqEuclidDistance(const unsigned long i, const unsigned long k)
{
	if(i == k) return 0.0;
	float * AVector = &LibMatrix[i * MaxVectorSize];
	float * CurrentVector = &LibMatrix[k * MaxVectorSize];
	float D;
	AuxTemplates::SquaredDistance(CurrentVector, AVector, VectorSize, &D);
	return D;
}




// Gets Manhattan Distance between two vectors at given positions in the library
float CFlexLibrary::GetManhattanDistance(const unsigned long i, const unsigned long k)
{
	if(i == k) return 0.0;
	float * AVector = &LibMatrix[i * MaxVectorSize];
	float * CurrentVector = &LibMatrix[k * MaxVectorSize];
	float D;
	AuxTemplates::ManhattanDistance(CurrentVector, AVector, VectorSize, &D);
	return D;
}



// Get a value of the internal matrix at the given position
float CFlexLibrary::GetLibValue(const unsigned long LineID, const unsigned int FeatureID)
{
	return LibMatrix[LineID * MaxVectorSize + FeatureID];
}











