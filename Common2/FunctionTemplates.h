#ifndef FUNCTIONTEMPLATES_H_
#define FUNCTIONTEMPLATES_H_

#include <cassert>
#include <cmath>
#include <climits>

// <DEBUG-APS>
#include <iostream>
// #include <cstdlib>
// #include <cstdio>

//********************************    T E M P L A T E S ***********************************************


namespace AuxTemplates
{


// Absolute value (module)
template< typename T >
T Module(T Value)
{
	if(Value < 0) return -Value;
	else return Value;
}


// Checking a mark: is a data channel saturated or not. Four first sample points is zero, if it is.
// sample - data frame to be checked. It might be float or integer.
template< typename T >
bool IsDataSaturated(T const * sample)
{
	if ((sample[0] == 0)&&(sample[1] == 0)&&(sample[2] == 0)&&(sample[3] == 0))	return true;
	else return false;
}



// Here function result is a simple summation of array elements. It is used for calculation 
// of a non-normalized mean value.
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// Calculation time = 1 a.u.
template< typename T >
void Sum(T const * sample, const unsigned long M,T * Result)
{
	T sum1 = 0.0;
	T sum2 = 0.0;
	T sum3 = 0.0;
	T sum4 = 0.0;
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	for(i=0; i<N; i+=4)
	{
		sum1 += sample[i];
		sum2 += sample[i+1];
		sum3 += sample[i+2];
		sum4 += sample[i+3];
	}
	sum1 += sum2+sum3+sum4;
	for(i=N; i<M; i++)
	{
		sum1 += sample[i];
	}
	*Result = sum1;
}

	
	
// Function result is a shifted non-normalized variance. A non-normalized mean value is substituted.  
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// Mean - Sum of all elements into the sample (without normalization)
// Calculation time = 1 a.u.
template< typename T >
void Variance(T const * sample, const unsigned long M, const T Mean, T * Result)
{
	unsigned long i,N;
	T a = -Mean * Mean / M;
	N = (M>>2)<<2; // N = (M/4)*4;
	T sum1 = 0.0;
	T sum2 = 0.0;
	T sum3 = 0.0;
	T sum4 = 0.0;
	for(i=0; i < N; i+=4)
	{
		sum1 += sample[i]   * sample[i];
		sum2 += sample[i+1] * sample[i+1];
		sum3 += sample[i+2] * sample[i+2];
		sum4 += sample[i+3] * sample[i+3];
	}
	a += sum1 + sum2 + sum3 + sum4;
	
	for(i=N; i<M; i++)
	{
		a +=sample[i] * sample[i];
	}

	*Result = a;
}

	
	
// Function result is a shifted non-normalized variance. 
// A non-normalized mean value is also calculated there.  
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// Calculation time = 2 a.u.
template< typename T >
void Variance(T const * sample, const unsigned long M, T * Result)
{
	T Mean = 0;
	Sum(sample,M,&Mean);
	Variance(sample,M,Mean,Result);
}





template< typename T >
void MeanAndVariance(T const * sample, const unsigned long M, T * OutMean, T * OutVariance)
{
	unsigned long i, N;
	N = (M >> 2) << 2; // N = (M/4)*4;
	T mean0 = 0.0f;
	T mean1 = 0.0f;
	T mean2 = 0.0f;
	T mean3 = 0.0f;
	T var0 = 0.0f;
	T var1 = 0.0f;
	T var2 = 0.0f;
	T var3 = 0.0f;
	T a0, a1, a2, a3;

	for (i = 0; i < N; i += 4)
	{
		a0 = sample[i];
		a1 = sample[i + 1];
		a2 = sample[i + 2];
		a3 = sample[i + 3];

		mean0 += a0;
		mean1 += a1;
		mean2 += a2;
		mean3 += a3;

		var0 += a0 * a0;
		var1 += a1 * a1;
		var2 += a2 * a2;
		var3 += a3 * a3;
	}
	for (i = N; i < M; i++)
	{
		a0 = sample[i];
		mean0 += a0;
		var0 += a0 * a0;
	}
	mean0 += mean1 + mean2 + mean3;
	// mean0 /= M;
	var0 += var1 + var2 + var3;
	// var0 /= M;
	var0 -= mean0 * mean0 / M;

	*OutMean = mean0;
	*OutVariance = var0;
}




	
// Function result is a maximal value within the frame without first several elements.   
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// StartFrom - the position of the first element included in the sum. The value should be more than 0.
// Calculation time = 1 a.u.
template< typename T >
void Maximum(T const * sample, const unsigned long M, double * Result)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	double sum1 = -10.0E50;
	double sum2 = -10.0E50;
	double sum3 = -10.0E50;
	double sum4 = -10.0E50;
	for(i=0; i<N; i+=4)
	{
		if (sum1 < sample[i])     sum1 = sample[i];
		if (sum2 < sample[i+1])   sum2 = sample[i+1];
		if (sum3 < sample[i+2])   sum3 = sample[i+2];
		if (sum4 < sample[i+3])   sum4 = sample[i+3];
	}
	if (sum2 > sum1) sum1 = sum2;
	if (sum3 > sum4) sum4 = sum3;
	if (sum4 > sum1) *Result= sum4;
	else *Result= sum1;

	for(i=N; i<M; i++)
	{
		if(*Result < sample[i]) *Result = sample[i];
	}
}

template< typename T >
void Maximum(T const * sample, const unsigned long M, float * Result)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	float sum1 = -10.0E50;
	float sum2 = -10.0E50;
	float sum3 = -10.0E50;
	float sum4 = -10.0E50;
	for(i=0; i<N; i+=4)
	{
		if (sum1 < sample[i])     sum1 = sample[i];
		if (sum2 < sample[i+1])   sum2 = sample[i+1];
		if (sum3 < sample[i+2])   sum3 = sample[i+2];
		if (sum4 < sample[i+3])   sum4 = sample[i+3];
	}
	if (sum2 > sum1) sum1 = sum2;
	if (sum3 > sum4) sum4 = sum3;
	if (sum4 > sum1) *Result= sum4;
	else *Result= sum1;
	
	for(i=N; i<M; i++)
	{
		if(*Result < sample[i]) *Result = sample[i];
	}
}

template< typename T >
void Maximum(T const * sample, const unsigned long M, long * Result)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	long sum1 = LONG_MIN;
	long sum2 = LONG_MIN;
	long sum3 = LONG_MIN;
	long sum4 = LONG_MIN;
	for(i=0; i<N; i+=4)
	{
		if (sum1 < sample[i])     sum1 = sample[i];
		if (sum2 < sample[i+1])   sum2 = sample[i+1];
		if (sum3 < sample[i+2])   sum3 = sample[i+2];
		if (sum4 < sample[i+3])   sum4 = sample[i+3];
	}
	if (sum2 > sum1) sum1 = sum2;
	if (sum3 > sum4) sum4 = sum3;
	if (sum4 > sum1) *Result= sum4;
	else *Result= sum1;

	for(i=N; i<M; i++)
	{
		if(*Result < sample[i]) *Result = sample[i];
	}
}


template< typename T >
void Maximum(T const * sample, const unsigned long M, T * Result, unsigned long * Index)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	T sum1 = -10.0E50;
	T sum2 = -10.0E50;
	T sum3 = -10.0E50;
	T sum4 = -10.0E50;
	unsigned long id1 = M, id2 = M, id3 = M, id4 = M;
	for(i=0; i<N; i+=4)
	{
		if (sum1 < sample[i])     {sum1 = sample[i];  id1 = i;}
		if (sum2 < sample[i+1])   {sum2 = sample[i+1];id2 = i+1;}
		if (sum3 < sample[i+2])   {sum3 = sample[i+2];id3 = i+2;}
		if (sum4 < sample[i+3])   {sum4 = sample[i+3];id4 = i+3;}
	}
	if (sum2 > sum1) {sum1 = sum2; id1 = id2;}
	if (sum3 > sum4) {sum4 = sum3; id4 = id3;}
	if (sum4 > sum1) {sum1 = sum4; id1 = id4;}
	
	for(i=N; i<M; i++)
	{
		if(sum1 < sample[i]) {sum1 = sample[i]; id1 = i;}
	}
	
	*Result = sum1;
	*Index = id1;
}


template< typename T >
void Minimum(T const * sample, const unsigned long M, T * Result)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	T sum1 = 10.0E50;
	T sum2 = 10.0E50;
	T sum3 = 10.0E50;
	T sum4 = 10.0E50;
	for(i=0; i<N; i+=4)
	{
		if (sum1 > sample[i])     sum1 = sample[i];
		if (sum2 > sample[i+1])   sum2 = sample[i+1];
		if (sum3 > sample[i+2])   sum3 = sample[i+2];
		if (sum4 > sample[i+3])   sum4 = sample[i+3];
	}
	if (sum2 < sum1) sum1 = sum2;
	if (sum3 < sum4) sum4 = sum3;
	if (sum4 < sum1) *Result= sum4;
	else *Result= sum1;
	
	for(i=N; i<M; i++)
	{
		if(*Result > sample[i]) *Result = sample[i];
	}
}




template< typename T >
void Minimum(T const * sample, const unsigned long M, T * Result, unsigned long * Index)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	T sum1 = 10.0E50;
	T sum2 = 10.0E50;
	T sum3 = 10.0E50;
	T sum4 = 10.0E50;
	unsigned long id1 = M, id2 = M, id3 = M, id4 = M;
	for(i=0; i<N; i+=4)
	{
		if (sum1 > sample[i])     {sum1 = sample[i];  id1 = i;}
		if (sum2 > sample[i+1])   {sum2 = sample[i+1];id2 = i+1;}
		if (sum3 > sample[i+2])   {sum3 = sample[i+2];id3 = i+2;}
		if (sum4 > sample[i+3])   {sum4 = sample[i+3];id4 = i+3;}
	}
	if (sum2 < sum1) {sum1 = sum2; id1 = id2;}
	if (sum3 < sum4) {sum4 = sum3; id4 = id3;}
	if (sum4 < sum1) {sum1 = sum4; id1 = id4;}
	
	for(i=N; i<M; i++)
	{
		if(sum1 > sample[i]) {sum1 = sample[i]; id1 = i;}
	}
	
	*Result = sum1;
	*Index = id1;
}

	
	
// One function result is a sum of all elements in the frame except first several ones, and 
// at the same time a maximal value is also obtained. Several start elements are excluded from the
// consideration. This function works farster, than two proper functions in a sequence. 
// The function is used for calculating the total power of the process within the frame
// and obtaining a maximal harmonic.   
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// StartFrom - the position of the first element included in the sum. This value should be more than 0.
// Calculation time = 1 a.u.  
template< typename T >
void MaxAndSum(T const * sample, const unsigned long M, T * ResultMax, T * ResultSum)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	T sum1 = 0.0;
	T sum2 = 0.0;
	T sum3 = 0.0;
	T sum4 = 0.0;
	T max1 = -10.0E50;
	T max2 = -10.0E50;
	T max3 = -10.0E50;
	T max4 = -10.0E50;

	for(i=0; i<N; i+=4)
	{
		if (max1 < sample[i])     max1 = sample[i];
		if (max2 < sample[i+1])   max2 = sample[i+1];
		if (max3 < sample[i+2])   max3 = sample[i+2];
		if (max4 < sample[i+3])   max4 = sample[i+3];
		sum1 += sample[i];
		sum2 += sample[i+1];
		sum3 += sample[i+2];
		sum4 += sample[i+3];
	}
	
	sum1 += sum2+sum3+sum4;

	if (max2 > max1) max1 = max2;
	if (max3 > max4) max4 = max3;
	if (max4 > max1) max1 = max4;
	
	for(i=N; i<M; i++)
	{
		sum1 += sample[i];
		if (max1< sample[i]) max1 = sample[i];
	}

	*ResultMax = max1;
	*ResultSum = sum1;
}



template< typename T >
void MaxAndMin(T const * sample, const unsigned long M, T * ResultMax, T * ResultMin)
{
	unsigned long i,N;
	N = (M>>2)<<2; // N = (M/4)*4;
	T min1 = 10.0E50;
	T min2 = 10.0E50;
	T min3 = 10.0E50;
	T min4 = 10.0E50;
	T max1 = -10.0E50;
	T max2 = -10.0E50;
	T max3 = -10.0E50;
	T max4 = -10.0E50;
	
	for(i=0; i<N; i+=4)
	{
		if (max1 < sample[i])     max1 = sample[i];
		if (max2 < sample[i+1])   max2 = sample[i+1];
		if (max3 < sample[i+2])   max3 = sample[i+2];
		if (max4 < sample[i+3])   max4 = sample[i+3];

		if (min1 > sample[i])     min1 = sample[i];
		if (min2 > sample[i+1])   min2 = sample[i+1];
		if (min3 > sample[i+2])   min3 = sample[i+2];
		if (min4 > sample[i+3])   min4 = sample[i+3];
	}
	
	if (max2 > max1) max1 = max2;
	if (max3 > max4) max4 = max3;
	if (max4 > max1) max1 = max4;

	if (min2 < min1) min1 = min2;
	if (min3 < min4) min4 = min3;
	if (min4 < min1) min1 = min4;
	
	for(i=N; i<M; i++)
	{
		if (max1 < sample[i]) max1 = sample[i];
		if (min1 > sample[i]) min1 = sample[i];
	}

	*ResultMax = max1;
	*ResultMin = min1;
}

	
	



template< typename T >
void ManhattanDistance(const T * LibVector, const T * FeatureVector, const unsigned int VectorSize, T * Result)
{
	T sum1 = 0;
	T sum2 = 0;
	T sum3 = 0;
	T sum4 = 0;
	unsigned int i,N;

	N = (VectorSize >> 2) << 2;  // N = (M/4)*4;
	for(i=0; i<N; i+=4)
	{
		sum1 += fabs(LibVector[i]   - FeatureVector[i]  );
		sum2 += fabs(LibVector[i+1] - FeatureVector[i+1]);
		sum3 += fabs(LibVector[i+2] - FeatureVector[i+2]);
		sum4 += fabs(LibVector[i+3] - FeatureVector[i+3]);
	}
	sum1 += sum2+sum3+sum4;
	for(i=N; i<VectorSize; i++)
	{
		sum1 += fabs(LibVector[i]   - FeatureVector[i]  );
	}
	*Result = sum1;
}



template< typename T >
void SquaredDistance(const T * LibVector, const T * FeatureVector, const unsigned int VectorSize, T * Result)
{
	T sum1 = 0;
	T sum2 = 0;
	T sum3 = 0;
	T sum4 = 0;
	T t1, t2, t3, t4;
	unsigned int i,N;

	N = (VectorSize >> 2) << 2;  // N = (M/4)*4;
	for(i=0; i<N; i+=4)
	{
		t1 = (LibVector[i]   - FeatureVector[i]  );
		t2 = (LibVector[i+1] - FeatureVector[i+1]);
		t3 = (LibVector[i+2] - FeatureVector[i+2]);
		t4 = (LibVector[i+3] - FeatureVector[i+3]);

		sum1 += t1 * t1;
		sum2 += t2 * t2;
		sum3 += t3 * t3;
		sum4 += t4 * t4;
	}
	sum1 += sum2+sum3+sum4;
	for(i=N; i<VectorSize; i++)
	{
		t1 = (LibVector[i]   - FeatureVector[i]  );
		sum1 += t1 * t1;
	}
	*Result = sum1;
}



template< typename T >
void EuclidDistance(const T * LibVector, const T * FeatureVector, const unsigned int VectorSize, T * Result)
{
	SquaredDistance(LibVector, FeatureVector, VectorSize, Result);
	*Result = sqrt(*Result);
}


	
	
// One function result is a sum of all elements in the frame except first several ones, and 
// at the same time a maximal value is also obtained. Several start elements are excluded from the
// consideration. This function works farster, than two proper functions in a sequence. 
// The function is used for calculating the total power of the process within the frame
// and obtaining a maximal harmonic.   
// sample - data frame, each element is a float number
// M - frame size (number of float values; 1 float point value = 4 bytes)
// StartFrom - the position of the first element included in the sum. This value should be more than 0.
// Calculation time = 1 a.u.  
template< typename T >
void SetSaturationMark(T * sample, const unsigned int M, const T SaturationLevel)
{
	unsigned int N;
	N = (M>>2)<<2;

	T min0 = sample[0];
	T min1 = sample[1];
	T min2 = sample[2];
	T min3 = sample[3];

	T max0 = min0;
	T max1 = min1;
	T max2 = min2;
	T max3 = min3;

	unsigned int maxCount0 = 1;
	unsigned int maxCount1 = 1;
	unsigned int maxCount2 = 1;
	unsigned int maxCount3 = 1;

	unsigned int minCount0 = 1;
	unsigned int minCount1 = 1;
	unsigned int minCount2 = 1;
	unsigned int minCount3 = 1;


	for(unsigned i=3; i<N; i+=4)
	{
		// sample[i]
		T temp = sample[i];
		if (max0 < temp)
		{
			max0 = temp;
			maxCount0 = 1;
		}
		else if (max0 == temp)	maxCount0++;
		if (min0 > temp)
		{
			min0 = temp;
			minCount0 = 1;
		}
		else if (min0 == temp)	minCount0++;


		// sample[i+1]
		temp = sample[i+1];
		if (max1 < temp)
		{
			max1 = temp;
			maxCount1 = 1;
		}
		else if (max1 == temp)	maxCount1++;
		if (min1 > temp)
		{
			min1 = temp;
			minCount1 = 1;
		}
		else if (min1 == temp)	minCount1++;
		
		
		// sample[i+2]
		temp = sample[i+2];
		if (max2 < temp)
		{
			max2 = temp;
			maxCount2 = 1;
		}
		else if (max2 == temp)	maxCount2++;
		if (min2 > temp)
		{
			min2 = temp;
			minCount2 = 1;
		}
		else if (min2 == temp)	minCount2++;
		
		
		// sample[i+3]
		temp = sample[i+3];
		if (max3 < temp)
		{
			max3 = temp;
			maxCount3 = 1;
		}
		else if (max3 == temp)	maxCount3++;
		if (min3 > temp)
		{
			min3 = temp;
			minCount3 = 1;
		}
		else if (min3 == temp)	minCount3++;
	}

	minCount0 += minCount1 + minCount2 + minCount3;
	maxCount0 += maxCount1 + maxCount2 + maxCount3;

	if (max1 > max0) max0 = max1;
	if (max2 > max0) max0 = max2;
	if (max3 > max0) max0 = max3;

	if (min1 < min0) min0 = min1;
	if (min2 < min0) min0 = min2;
	if (min3 < min0) min0 = min3;


	for(unsigned i=N; i<M; i++)
	{
		// sample[i]
		T temp = sample[i];
		if (max0 < temp)
		{
			max0 = temp;
			maxCount0 = 1;
		}
		else if (max0 == temp)	maxCount0++;
		if (min0 > temp)
		{
			min0 = temp;
			minCount0 = 1;
		}
		else if (min0 == temp)	minCount0++;
	}
	
//		*ResultMaxCount = maxCount0;
//		*ResultMinCount = minCount0;
	
	if((maxCount0 > SaturationLevel) || (minCount0 > SaturationLevel))
	{
		sample[0] = 0;
		sample[1] = 0;
		sample[2] = 0;
		sample[3] = 0;
	}

}

	
	

// The result is a non-normalized cross-correlation coefficient -1 < C/M < 1
// obtained for two different frames.  The function is used for one anti-fading strategy.
// Mean values for the both frames are supposed known, they are substituted in the function in an
// explicit form
// sample1 - data of the first frame, each element is a float number
// sample2 - data of the second frame
// M - frame size (number of float values; 1 float point value = 4 bytes)
// Mean1 - a non-normalized mean value (Sum) of the first sample (substituted).  
// Mean2 - a non-normalized mean value (Sum)of the second sample (substituted).
// Calculation time = 1 a.u.
template< typename T >
void CrossCorr(T const * sample1, T const * sample2, const unsigned long M, const T Mean1, const T Mean2, T * Result)
{
	T InverseM = 1.0 / M;
	T m1 = Mean1 * InverseM;
	T m2 = Mean2 * InverseM;
	unsigned long i;
	T sum1 = 0.0;
	T sum2 = 0.0;
	T sum3 = 0.0;
	T sum4 = 0.0;
	for(i=0; i<M; i+=4)
	{
		T a1 = sample1[i] - m1;
		T a2 = sample1[i+1] - m1;
		T a3 = sample1[i+2] - m1;
		T a4 = sample1[i+3] - m1;

		T b1 = sample2[i] - m2;
		T b2 = sample2[i+1] - m2;
		T b3 = sample2[i+2] - m2;
		T b4 = sample2[i+3] - m2;

		sum1 += a1   * b1;
		sum2 += a2   * b2;
		sum3 += a3   * b3;
		sum4 += a4   * b4;
	}
	*Result = sum1 + sum2 + sum3 + sum4;
}



template< typename T >
void CrossCorr(T const * sample1, T const * sample2, const unsigned long M, const T Mean1, const T Mean2, double * Result)
{
	T InverseM = 1.0 / M;
	T m1 = Mean1 * InverseM;
	T m2 = Mean2 * InverseM;
	unsigned long i;
	T sum1 = 0.0f;
	T sum2 = 0.0f;
	T sum3 = 0.0f;
	T sum4 = 0.0f;
	for(i=0; i<M; i+=4)
	{
		T a1 = sample1[i] - m1;
		T a2 = sample1[i+1] - m1;
		T a3 = sample1[i+2] - m1;
		T a4 = sample1[i+3] - m1;

		T b1 = sample2[i] - m2;
		T b2 = sample2[i+1] - m2;
		T b3 = sample2[i+2] - m2;
		T b4 = sample2[i+3] - m2;

		sum1 += a1   * b1;
		sum2 += a2   * b2;
		sum3 += a3   * b3;
		sum4 += a4   * b4;
	}
	*Result = sum1 + sum2 + sum3 + sum4;
}




// The result is a non-normalized cross-correlation coefficient -1 < C/M < 1
// obtained for two different frames. Here calculations of mean values (Sums) for the both
// frames are included. The function is used for one anti-fading strategy.
// sample1 - data of the first frame, each element is a float number
// sample2 - data of the second frame
// M - frame size (number of float values; 1 float point value = 4 bytes)
// Calculation time = 3 a.u.
template< typename T >
void CrossCorr(T const * sample1, T const * sample2, const unsigned long M, T * Result)
{
	T Mean1 =0 ,Mean2 =0;
	Sum(sample1,M,&Mean1);
	Sum(sample2,M,&Mean2);
	// CrossCorr(sample1,sample2,M,Mean1,Mean2,Result);
	T InverseM = 1.0 / M;
	T m1 = Mean1 * InverseM;
	T m2 = Mean2 * InverseM;
	unsigned long i;
	T sum1 = 0.0f;
	T sum2 = 0.0f;
	T sum3 = 0.0f;
	T sum4 = 0.0f;
	for(i=0; i<M; i+=4)
	{
		T a1 = sample1[i] - m1;
		T a2 = sample1[i+1] - m1;
		T a3 = sample1[i+2] - m1;
		T a4 = sample1[i+3] - m1;

		T b1 = sample2[i] - m2;
		T b2 = sample2[i+1] - m2;
		T b3 = sample2[i+2] - m2;
		T b4 = sample2[i+3] - m2;

		sum1 += a1   * b1;
		sum2 += a2   * b2;
		sum3 += a3   * b3;
		sum4 += a4   * b4;
	}
	*Result = sum1 + sum2 + sum3 + sum4;
}



	
	
// NOT USED NOW
// Set all sample points to zero
// start - a dataset
// NumPoints - frame size
template< typename T >
void SetZero( T * start, unsigned const NumPoints )
{
	unsigned UnfoldedLoops = NumPoints >> 2; // integer division, no remainder
	
	// Testing shows that a for loop here is slightly faster than using while:
	for( unsigned i = 0; i < UnfoldedLoops; ++i )
	{
		start[i] = static_cast<T>(0.0);
		start[i+1] = static_cast<T>(0.0);
		start[i+2] = static_cast<T>(0.0);
		start[i+3] = static_cast<T>(0.0);
	}
	
	for( unsigned i = UnfoldedLoops; i < NumPoints; ++i )
	{
		start[i] = static_cast<T>(0.0);
	}
}
	
	
	
	
	


// Centering the sample and normalizing its energy
template< typename T >
void Normalization(T * sample, const unsigned int M, const bool IsAlreadyCentered)
{
	T sigma, sum = 0.0;
	unsigned int i,N;
	N = (M >> 2) << 2;  // N = (M/4)*4;

	if(IsAlreadyCentered) Variance(sample, (unsigned long) M, sum, &sigma);
	else
	{
		Sum(sample, M, &sum);
		sum = sum / M;
		Variance(sample, M, sum, &sigma);
	}
	// Note: sigma cannot be 0, because the sample was already checked whether it is saturated or not
	// if(sigma == 0.0) return;
	sigma = sigma / M;
	sigma = 1.0 / sqrt(sigma);

	if(IsAlreadyCentered)
	{
		for(i=0; i<N; i+=4)
		{
			sample[i]     *= sigma;
			sample[i + 1] *= sigma;
			sample[i + 2] *= sigma;
			sample[i + 3] *= sigma;
		}
		for(i=N; i<M; i++) sample[i] *= sigma;
	}
	else
	{
		T p0, p1, p2, p3;
		for(i=0; i<N; i+=4)
		{
			p0 = sample[i]     - sum;
			p1 = sample[i + 1] - sum;
			p2 = sample[i + 2] - sum;
			p3 = sample[i + 3] - sum;
			
			sample[i]     = p0 * sigma;
			sample[i + 1] = p1 * sigma;
			sample[i + 2] = p2 * sigma;
			sample[i + 3] = p3 * sigma;
		}
		for(i=N; i<M; i++)
		{
			p0 = sample[i] - sum;
			sample[i] = p0 * sigma;
		}
	}

}


// Checking whether the input integer value is a result of power 2^n (where n is an integer)
template< typename T >
bool CheckPower2(const T Value)
{
	bool res = true;
	T temp = Value;
	if(temp <= 0) return false;
	while(1)
	{
		if((temp % 2) != 0)
		{
			res = false;
			break;
		}
		temp /= 2;
		if(temp <= 1) break;
	}
	return res;
}
		
}

#endif /*FUNCTIONTEMPLATES_H_*/
