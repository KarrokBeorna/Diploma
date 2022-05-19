/*
* PhaseProc.h
*
*  Created on: Jan 20, 2022
*      Author: Artem Khlybov
*/


#ifndef PHASEPROC_SIGNALEMULATOR_H_
#define PHASEPROC_SIGNALEMULATOR_H_

// The class contains all operations related to phase processing: phase wrapping, phase unwrapping and
// phase correction. Phase detection operations were not included by now.


class CPhaseProc
{
private:
	bool Ready;	// Flag of readiness
	unsigned int FrameSize;					// Frame size = cell size (2^n)
	float SmoothnessCriterion[3]; // Criterion of the signal smoothness
	float* TmpVector[3]; // Temporary vectors. Size = Time frame size
	float* Means;  // Temporary vector of moving average. Size = Time frame size

	void Init(); // Initializes internal structures
	float WrappingOperator(const float WrappedValue); // Wrapping operator
	float Sign(const float Value); // Gets sign of the input value
	void DataCopy(const unsigned int Size, float* Source, float* Dest); // Copies data from one array to another
	float ComputeSmoothnessCriterion_Fast(const unsigned int Size, float* PhaseLine); // Computes the criterion of smoothness for corrected phase signal, faster version (single-pass)
	float ComputeSmoothnessCriterion(const unsigned long Size, float MeanAbsIncrement, float* PhaseLine); // Computes the criterion of smoothness for corrected phase signal. Mean absolute increment is computed outside of the method 
	void PostProc_TemporalCorrection3_FirstPass(float* InLine, float** OutLine, float* OutSmoothnessCriterion);  // Temporal correction scheme, February 2017. First pass. Max phase increment: 2 rad
	void PostProc_TemporalCorrection3_SecondPass(const unsigned int HalfWidth, float* InLine, float** OutLine, float* OutSmoothnessCriterion); // Temporal correction scheme, February 2017. Second pass. Max phase increment for HalfWidth = 2: 0.785 rad; for for HalfWidth = 5: 0.32

public:
	CPhaseProc(const unsigned int FrameSize); // Constructor
	~CPhaseProc(); // Destructor

	void ApplyPhaseDistortions(float* DataCell); // Applies phase distortions associated to unwrapping process to a data cell
	void ApplyPhaseCorrection(float* DataCell); // Applies phase correction to a data cell


		// Access to properties
	bool IsReady(); // Gets the flag of readiness
};




#endif /*PHASEPROC_SIGNALEMULATOR_H_*/




