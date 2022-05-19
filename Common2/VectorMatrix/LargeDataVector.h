/*
 * LargeDataVector.h
 *
 *  Created on: Feb 13, 2014
 *  Modified on: Mar 08, 2022
 *      Author: Artem Khlybov
 */

#ifndef LARGEDATAVECTOR_H_
#define LARGEDATAVECTOR_H_


#include <iostream>
#include <fstream>
#include <string>


// The class is a container of 1D data vector of real values.
template<typename T>
class CLargeDataVector
{
private:
	unsigned int DataStrType; // String type of the contained data: // 0 - double & float (%g), 1 - int & unsigned int (%d), 2 - long & unsigned long (%ld)
	unsigned long Size; // Number of elements
	const char Delimiter = '\t'; // String Delimiteriter (tab)
	char StrType[3]; // String type of the internal data
	char StrTypeWithDelim[4]; // String type of the internal data with the delimiter in the beginning
	T * Content;	// Data content (1D array)
	
	//_________________________________   private methods  ______________________________________

	// Nulls internal structures
	void Nulling()
	{
		Size = 0;
		Content = NULL;

		// 0 - double & float (%g), 1 - int & unsigned int (%d), 2 - long & unsigned long (%ld)
		if constexpr (std::is_same_v<T, double>) DataStrType = 0;
		if constexpr (std::is_same_v<T, float>) DataStrType = 0;
		if constexpr (std::is_same_v<T, int>) DataStrType = 1;
		if constexpr (std::is_same_v<T, unsigned int>) DataStrType = 1;
		if constexpr (std::is_same_v<T, unsigned char>) DataStrType = 1;
		if constexpr (std::is_same_v<T, char>) DataStrType = 1;
		if constexpr (std::is_same_v<T, unsigned short>) DataStrType = 1;
		if constexpr (std::is_same_v<T, short>) DataStrType = 1;
		if constexpr (std::is_same_v<T, long>) DataStrType = 2;
		if constexpr (std::is_same_v<T, unsigned long>) DataStrType = 2;

		if (DataStrType == 0) strcpy(StrType, "%g");
		if (DataStrType == 1) strcpy(StrType, "%d");
		if (DataStrType == 2) strcpy(StrType, "%ld");
		StrTypeWithDelim[0] = Delimiter;
		strcat(StrTypeWithDelim, StrType);
	}

	// Reads data from a text file
	bool ReadDataFile(FILE* pFile)
	{
		// 1. Check the presence of the file
		if (pFile == NULL)
		{
			printf("\nInput file is not ready\n");
			return false;
		}
		long lines = 0;

		// 2. Find the number of lines
		rewind(pFile);
		char string[100];
		while (!feof(pFile))
		{
			fgets(string, 100, pFile);
			lines++;
		}
		lines--;
		if (lines <= 0)
		{
			printf("\nInput file is empty\n");
			fclose(pFile);
			return false;
		}
		Size = lines;

		// 3. Read data
		rewind(pFile);
		Content = (T*)calloc(Size, sizeof(T));
		T dTmp;
		for (unsigned long i = 0; i < Size; i++)
		{
			fscanf(pFile, StrType, &dTmp);
			Content[i] = dTmp;
		}

		// fclose(pFile);
		return true;
	}


public:
	//_________________________________   public methods  ______________________________________

	// Constructor 1 (copy)
	CLargeDataVector(CLargeDataVector<T>* Vector)
	{
		Nulling();
		if (Vector == NULL) return;
		if (!Vector->IsDataReady()) return;
		Size = Vector->Size;
		Content = (T*)calloc(Size, sizeof(T));
		T* ExtContent = Vector->GetInternalData();
		for (unsigned long i = 0; i < Size; i++) Content[i] = ExtContent[i];
	}

	// Constructor 2 (empty vector)
	CLargeDataVector(const unsigned long Size)
	{
		Nulling();
		if (Size > 0)
		{
			this->Size = Size;
			Content = (T*)calloc(Size, sizeof(T));
		}
	}

	// Constructor 3 (text file)
	CLargeDataVector(FILE* pFile)
	{
		Nulling();
		ReadDataFile(pFile);
	}

	// Constructor 4 (string)
	CLargeDataVector(const char* Str)
	{
		Nulling();
		const unsigned int StrLen = strlen(Str);
		if (StrLen == 0) return;

		// 1. Determining the size
		unsigned long id = 0;
		unsigned long count = 0;
		int a;
		T Tmp;
		while (true)
		{
			const char* pstr = &Str[id];
			a = sscanf(pstr, StrType, &Tmp);

			if (a < 0) break;
			for (unsigned long j = id; j <= StrLen; j++)
			{
				if (j == StrLen)
				{
					id = StrLen;
					break;
				}
				else
				{
					if ((Str[j] == ' ') || (Str[j] == '	'))
					{
						id = j + 1;
						break;
					}
				}
			}
			count++;
			if (id >= StrLen) break;
		}
		if (count == 0) return;
		Size = count;

		// 2. Filling the array
		if (Content != NULL) delete Content;
		Content = (T*)calloc(Size, sizeof(T));
		count = 0;
		id = 0;
		while (true)
		{
			const char* pstr = &Str[id];
			a = sscanf(pstr, StrType, &Tmp);
			if (a < 0) break;
			for (unsigned long j = id; j <= StrLen; j++)
			{
				if (j == StrLen)
				{
					id = StrLen;
					break;
				}
				else
				{
					if ((Str[j] == ' ') || (Str[j] == '	'))
					{
						id = j + 1;
						break;
					}
				}
			}
			Content[count] = Tmp;
			count++;
			if (id >= StrLen) break;
		}
	}

	// Constructor 5 (constrained copy)
	CLargeDataVector(CLargeDataVector<T>* Vector, CLargeDataVector<int>* Map)
	{
		Nulling();
		if (Vector == NULL) return;
		if (Map == NULL) return;
		if (!Vector->IsDataReady()) return;
		if (!Map->IsDataReady()) return;
		Size = Map->GetSize();
		Content = (T*)calloc(Size, sizeof(T));
		T* ExtContent = Vector->GetInternalData();
		for (unsigned long i = 0; i < Size; i++) Content[i] = ExtContent[Map->GetValue(i)];
	}

	// Destructor
	~CLargeDataVector()
	{
		if (Content != NULL) free(Content);
	}

	// Write the data into a text file
	bool WriteDataIntoFile(const char* OutputFile, const bool PrintInOneRow = true)
	{
		FILE* pFile = fopen(OutputFile, "w"); // creating the output file
		if (pFile == NULL)
		{
			printf("\nFile %s doesn't exist", OutputFile);
			return false;
		}

		if (PrintInOneRow) for (unsigned long r = 0; r < Size; r++)
		{
			if(r == 0) fprintf(pFile, StrType, Content[r]);
			else fprintf(pFile, StrTypeWithDelim, Content[r]);
		}
		else for (unsigned long r = 0; r < Size; r++)
		{
			fprintf(pFile, StrType, Content[r]);
			fprintf(pFile, "\n");
		}
		fclose(pFile);
		return true;
	}
	
	
	// Prints the vector to the console
	void PrintData(const bool PrintInOneRow = false)
	{
		if (PrintInOneRow)
		{
			for (unsigned long r = 0; r < Size; r++)
			{
				printf("\n");
				printf(StrType, Content[r]);
			}
		}
		else
		{
			printf("\n");
			for (unsigned long r = 0; r < Size; r++)
			{
				if (r == 0) printf(StrType, Content[r]);
				else printf(StrTypeWithDelim, Content[r]);
			}
		}
	}


	// Copies the content of existing vector (sizes of both vectors must be equal)
	bool CopyContent(CLargeDataVector<T>* Vector)
	{
		if (Vector == NULL) return false;
		if (!Vector->IsDataReady()) return false;
		if (Size != Vector->GetSize()) return false;
		T* ExtCont = Vector->Content;
		for (unsigned long i = 0; i < Size; i++) Content[i] = ExtCont[i];
		return true;
	}


	// Stores the content of the vector to a text string
	void ConvertToString(char* String)
	{
		String[0] = '\0';
		char Tmp[30];
		for (unsigned long c = 0; c < Size; c++)
		{
			if (c == 0) sprintf(Tmp, StrType, Content[c]);
			else sprintf(Tmp, StrTypeWithDelim, Content[c]);
			strcat(String, Tmp);
		}
	}




	//_________________________________   Access to properties ______________________________________


	// Get the flag of readiness
	bool IsDataReady()
	{
		if ((Content == NULL) || (Size == 0)) return false;
		else return true;
	}


	// Resets one element of the vector in a given position
	bool ResetValue(const unsigned long ID, const T NewValue)
	{
		if ((ID >= 0) && (ID < Size))
		{
			Content[ID] = NewValue;
			return true;
		}
		else return false;
	}

	// Gets one element of the vector in a given position
	T GetValue(const unsigned long ID)
	{
		return Content[ID];
	}

	// Gets the internal array of data
	T* GetInternalData()
	{
		return Content;
	}

	// Gets the number of elements
	unsigned long GetSize()
	{
		return Size;
	}
};







#endif /* LARGEDATAVECTOR_H_ */
