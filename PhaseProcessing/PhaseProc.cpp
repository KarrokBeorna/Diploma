/*
* PhaseProc.cpp
*
*  Created on: Jan 20, 2022
*      Author: Artem Khlybov
*/


#include <cstdlib>
// #include <cmath>
#include "PhaseProc.h"

#define PI 3.14159265358979323846
#define PI_Doubled 6.28318530717959
#define MIN_PHASE_JUMP1 4.0



// ************************************************************************************************************
// ************************************************************************************************************
//   _________________________________   Class CPhaseProc  ________________________________________
// ************************************************************************************************************
// ************************************************************************************************************

//   ______________________________   public methods   ________________________________________________________

// Constructor
CPhaseProc::CPhaseProc(const unsigned int FrameSize)
{
	if (FrameSize == 0) return;
	this->FrameSize = FrameSize;

	for (unsigned int i = 0; i < 3; i++)
	{
		TmpVector[i] = (float*)calloc(FrameSize, sizeof(float));
		if (TmpVector[i] == NULL) return;
	}

	Means = (float*)calloc(FrameSize, sizeof(float));
	if (Means == NULL) return;

	Ready = true;
}



// Destructor
CPhaseProc::~CPhaseProc()
{
	for (unsigned int i = 0; i < 3; i++) if (TmpVector[i] != NULL) free(TmpVector[i]);
	if (Means != NULL) free(Means);
}




// Applies phase distortions associated to unwrapping process to a data cell
void CPhaseProc::ApplyPhaseDistortions(float* DataCell)
{
	float* TmpDataCell = TmpVector[0];
	// Phase wrapping
	for (unsigned int t = 0; t < FrameSize; t += 4)
	{
		TmpDataCell[t] = WrappingOperator(DataCell[t]);
		TmpDataCell[t + 1] = WrappingOperator(DataCell[t + 1]);
		TmpDataCell[t + 2] = WrappingOperator(DataCell[t + 2]);
		TmpDataCell[t + 3] = WrappingOperator(DataCell[t + 3]);
	}

	// Phase unwrapping
	DataCell[0] = TmpDataCell[0];
	for (unsigned int t = 1; t < FrameSize; t++)
	{
		DataCell[t] = DataCell[t - 1] + WrappingOperator(TmpDataCell[t] - TmpDataCell[t - 1]);
	}

	// Centering - not necessary
}



// Applies phase correction to a data cell
void CPhaseProc::ApplyPhaseCorrection(float* DataCell)
{
	float InitSmoothnessCriterion = ComputeSmoothnessCriterion_Fast(FrameSize, DataCell); // For quality checks; no need to compute. Initial smoothness
	float MinSmoothnessCriterion = InitSmoothnessCriterion; 
	int best_option = -1;

	PostProc_TemporalCorrection3_FirstPass(DataCell, &(TmpVector[0]), &(SmoothnessCriterion[0]));
	PostProc_TemporalCorrection3_SecondPass(2, TmpVector[0], &(TmpVector[1]), &(SmoothnessCriterion[1]));
	PostProc_TemporalCorrection3_SecondPass(5, TmpVector[0], &(TmpVector[2]), &(SmoothnessCriterion[2]));
	
	// The smoothest option is selected
	for (unsigned int i = 0; i < 3; i++) if (SmoothnessCriterion[i] < MinSmoothnessCriterion)
	{
		MinSmoothnessCriterion = SmoothnessCriterion[i];
		best_option = i;
	}

	if (best_option >= 0)
	{
		float * OutLine_BestOption = TmpVector[best_option];
		DataCopy(FrameSize, OutLine_BestOption, DataCell);
	}


}


//   ______________________________   Access to properties   ________________________________________________________

// 
// Gets the flag of readiness
bool CPhaseProc::IsReady()
{
	return Ready;
}






//   ______________________________   private methods   ________________________________________________________


// Initializes internal structures
void CPhaseProc::Init()
{
	for (unsigned int i = 0; i < 3; i++) TmpVector[i] = NULL;
	Means = NULL;

	Ready = false;
}




// Wrapping operator
float CPhaseProc::WrappingOperator(const float WrappedValue)
{
	return WrappedValue - PI_Doubled * floor((WrappedValue + PI) / PI_Doubled);
}




// Gets sign of the input value
float CPhaseProc::Sign(const float Value)
{
	if (Value >= 0.0f) return 1.0f;
	else return -1.0f;
}




// This function copies the data from one array to another one.
void CPhaseProc::DataCopy(const unsigned int Size, float* Source, float* Dest)
{
	unsigned int i, i1, i2, i3;
	unsigned int N;
	N = (Size >> 2) << 2;
	for (i = 0; i < N; i += 4)
	{
		i1 = i + 1;
		i2 = i + 2;
		i3 = i + 3;
		Dest[i] = Source[i];
		Dest[i1] = Source[i1];
		Dest[i2] = Source[i2];
		Dest[i3] = Source[i3];
	}
	for (i = N; i < Size; i++) Dest[i] = Source[i];
}




// Computes the criterion of smoothness for corrected phase signal, faster version (single-pass)
float CPhaseProc::ComputeSmoothnessCriterion_Fast(const unsigned int Size, float* PhaseLine)
{
	unsigned int i, N;
	float summ_a = 0.0f;
	float summ_b = 0.0f;
	float summ_c = 0.0f;
	float summ_d = 0.0f;

	float a, b, c, d;
	N = ((Size - 1) >> 2) << 2; // N = (M/4)*4;
	for (i = 0; i < N; i += 4)
	{
		a = PhaseLine[i];
		b = PhaseLine[i + 1];
		c = PhaseLine[i + 2];
		d = PhaseLine[i + 3];

		summ_a += fabsf(a - b);
		summ_b += fabsf(b - c);
		summ_c += fabsf(c - d);
		summ_d += fabsf(d - PhaseLine[i + 4]);
	}
	summ_a += summ_b + summ_c + summ_d;

	for (i = N; i < Size; i++)
	{
		a = PhaseLine[i];
		if (i < (Size - 1)) summ_a += fabsf(a - PhaseLine[i + 1]);
	}
	summ_a /= Size;
	// if (summ_a == 0.0) summ_a = 0.0;
	return summ_a;
}




// Computes the criterion of smoothness for corrected phase signal. Mean absolute increment is computed outside of the method 
float CPhaseProc::ComputeSmoothnessCriterion(const unsigned long Size, float MeanAbsIncrement, float* PhaseLine)
{
	/*
	float OutVar;
	AuxTemplates::Variance(PhaseLine, Size, &OutVar);
	return (MeanAbsIncrement / Size) + STDEV_WEIGHT_IN_SMOOTHNESS_CRITERION * sqrt(OutVar / Size);
	*/
	return (MeanAbsIncrement / Size);
}





// Temporal correction scheme, February 2017. First pass. Max phase increment: 2 rad
void CPhaseProc::PostProc_TemporalCorrection3_FirstPass(float* InLine, float** OutLine, float * OutSmoothnessCriterion)
{
	float* OutLine1 = *OutLine;
	float a, shift = 0.0f;
	OutLine1[0] = InLine[0];
	float OutAbsInc = 0.0f;

	unsigned int t = 1;
	while (t < FrameSize - 1)
	{
		float sum1 = InLine[t + 1] - InLine[t - 1];
		if (fabsf(sum1) > MIN_PHASE_JUMP1)
		{
			if (t < FrameSize - 2)
			{
				float sum2 = InLine[t + 2] - InLine[t - 1];
				if (fabsf(sum2) < MIN_PHASE_JUMP1)
				{
					OutLine1[t] = InLine[t] + shift;
					OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
					t++;
					continue;
				}
			}
			a = PI * Sign(sum1);
			OutLine1[t] = InLine[t] + shift - a;
			OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
			a *= 2.0f;
			OutLine1[t + 1] = InLine[t + 1] + shift - a;
			OutAbsInc += fabsf(OutLine1[t + 1] - OutLine1[t]);
			shift -= a;
			t += 2;
		}
		else  // if fabsf(sum1) > MIN_PHASE_JUMP1
		{
			OutLine1[t] = InLine[t] + shift;
			OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
			t++;
		}
	} // end while(t)
	OutLine1[FrameSize - 1] = InLine[FrameSize - 1] + shift;

	*OutSmoothnessCriterion = ComputeSmoothnessCriterion(FrameSize, OutAbsInc, OutLine1);
}





// Temporal correction scheme, February 2017. Second pass. Max phase increment for HalfWidth = 2: 0.785 rad; for for HalfWidth = 5: 0.32
void CPhaseProc::PostProc_TemporalCorrection3_SecondPass(const unsigned int HalfWidth, float* InLine, float** OutLine, float* OutSmoothnessCriterion)
{
	// Filling the array of local means
	float diff, x = 0.0f;
	float OutAbsInc = 0.0f;
	const float MinPhaseJump2 = PI;
	const unsigned int FullWidth = HalfWidth * 2 + 1;
	const unsigned long end1 = FrameSize - 1 - HalfWidth;
	const unsigned long end2 = FrameSize - 1 - HalfWidth * 2;
	const float NormMean = 1.0f / (float)FullWidth;
	float* OutLine1 = *OutLine;
	int half = HalfWidth;
	int t;

	for (int k = -half; k <= half; k++) x += InLine[half + k];
	Means[half] = x * NormMean;
	for (t = half + 1; t <= end1; t++)
	{
		Means[t] = Means[t - 1] + NormMean * (InLine[t + half] - InLine[t - 1 - half]);
	}

	for (t = 0; t < half; t++)
	{
		OutLine1[t] = InLine[t];
		if (t > 0) OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
	}
	float shift = 0.0f;
	unsigned int count = 0;
	for (t = half; t <= end2; t++)
	{
		diff = Means[t - half] - Means[t + half];
		if ((fabsf(diff) > MinPhaseJump2) && (count > half * 2))
		{
			OutLine1[t] = InLine[t] + shift;
			shift += PI_Doubled * Sign(diff);
			count = 0;
		}
		else OutLine1[t] = InLine[t] + shift;
		OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
		count++;
	}
	for (t = end2 + 1; t < FrameSize; t++)
	{
		OutLine1[t] = InLine[t] + shift;
		OutAbsInc += fabsf(OutLine1[t] - OutLine1[t - 1]);
	}

	*OutSmoothnessCriterion = ComputeSmoothnessCriterion(FrameSize, OutAbsInc, OutLine1);
}



