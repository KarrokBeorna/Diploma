/*
 * LargeDataMatrix.h
 *
 *  Created on: Feb 13, 2014
 *  Modified on: Mar 08, 2022
 *      Author: Artem Khlybov
 */

#ifndef LARGEDATAMATRIX_H_
#define LARGEDATAMATRIX_H_



#include <cstdlib>
#include <cstdio>
#include "LargeDataVector.h"



// The class is a container of 2D data matrix of real values. It represents a large matrix.
template<typename T>
class CLargeDataMatrix
{
private:
	unsigned int DataStrType; // String type of the contained data: // 0 - double & float (%g), 1 - int & unsigned int (%d), 2 - long & unsigned long (%ld)
	const char Delimiter = '\t'; // String delimiter (tab)
	char StrType[3]; // String type of the internal data
	// char StrTypeWithDelim[4]; // String type of the internal data with the delimiter in the beginning

	//_________________________________   private methods  ______________________________________

	

	// Reads data from a text file
	bool ReadDataFile(const char * FileName)
	{
		// 1. Check the presence of the file 
		bool out = true;
		std::ifstream infile;
		infile.open(FileName);
		if (!infile.is_open())
		{
			printf("\nThe file %s doesn't exist\n", FileName);
			return false;
		}
		unsigned long cols = 0;
		unsigned long lines = 0;

		// 2. Find the number of rows 
		std::string line;
		while (!infile.eof())
		{
			getline(infile, line);
			lines++;
		}
		if (lines > 0) lines--;
		
		// 3. Find the number of cols
		infile.clear();
		infile.seekg(0, std::ios::beg);	// Reset to the start of the file
		getline(infile, line);

		unsigned long temp = line.size();
		if (temp == 1)
		{
			getline(infile, line);
			temp = line.size();
		}

		for (unsigned long i = 0; i < temp; i++) if (line[i] == Delimiter) cols++;
		cols++;
		if (temp > 1)
		{
			// if (((string[temp - 1] == '\r') || (string[temp - 1] == '\n')) && (string[temp - 2] == '\t')) cols--;
			if ((line[temp - 2] == Delimiter) || (line[temp - 1] == Delimiter) || (line[temp - 1] == '\r') || (line[temp - 1] == '\n')) cols--;
		}

		if ((lines == 0) || (cols < 1))
		{
			printf("\nThe matrix file %s is empty or has a wrong format.", FileName);
			printf("\nLines = %ld; Columns = %ld.", lines, cols);
			printf("\n");
			out = false;
		}
		N_Rows = lines;
		N_Cols = cols;
		if (!out) return out;

		// 4. Read data
		DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));
		T dTmp;
		infile.clear();
		infile.seekg(0, std::ios::beg);	// Reset to the start of the file

		// infile.read(DataMatrix, N_Rows * N_Cols);
		
		for (unsigned long i = 0; i < N_Rows; i++)
		{
			getline(infile, line);
			/*
			stringstream ss(line, ios_base::out);
			while (ss >> dTmp) 
			{
				if (ss.peek() == Delimiter) ss.ignore();
				DataMatrix[n + i * N_Cols] = dTmp;
			}
			*/
			
			/*
			const char* cstr = line.data();			
			for (unsigned long n = 0; n < N_Cols; n++)
			{
				sscanf(cstr, StrType, &dTmp);
				DataMatrix[n + i * N_Cols] = dTmp;
			}
			*/

			ResetRow(i, line.data(), N_Cols);
		}
		
		infile.close();

		if (AColumn != NULL)
		{
			if (AColumn->GetSize() != N_Rows)
			{
				delete AColumn;
				AColumn = NULL;
			}
		}
		if (ARow != NULL)
		{
			if (ARow->GetSize() != N_Cols)
			{
				delete ARow;
				ARow = NULL;
			}
		}
		if (AColumn == NULL) AColumn = new CLargeDataVector<T>(N_Rows);
		if (ARow == NULL) ARow = new CLargeDataVector<T>(N_Cols);

		return true;
	}




	// Nulls internal structures
	void Nulling()
	{
		DataMatrix = NULL;
		AColumn = NULL;
		ARow = NULL;
		N_Rows = 0;
		N_Cols = 0;

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

		// StrTypeWithDelim[0] = Delimiter;
		// strcat(StrTypeWithDelim, StrType);
	}


	// Read a 4-byte unsigned integer
	unsigned int ReadInt4(FILE* pFile)
	{
		unsigned int iTmp0 = 0;
		for (unsigned int k = 0; k < 4; k++)
		{
			unsigned char c;
			fscanf(pFile, "%c", &c);
			iTmp0 += c << (k * 8);
		}
		return iTmp0;
	}



	// Reads and converts a 2-byte value to 4-byte
	unsigned int ReadInt2(unsigned char* Buffer, const unsigned long pos)
	{
		unsigned int iTmp0 = 0;
		for (unsigned int k = 0; k < 2; k++)
		{
			unsigned char c = Buffer[pos + k];
			iTmp0 += c << (k * 8);
		}
		return iTmp0;
	}


	// Reads data from the binary file. DataFormat: 0 - unsigned short, 1 - float, 2 - double
	bool ReadBinDataFile(FILE* pBinFile, const unsigned int DataStartPos, const unsigned int DataFormat)
	{
		fseek(pBinFile, DataStartPos, SEEK_SET);
		N_Cols = ReadInt4(pBinFile);

		long pos, end;
		pos = ftell(pBinFile);
		fseek(pBinFile, 0, SEEK_END);
		end = ftell(pBinFile);
		fseek(pBinFile, pos, SEEK_SET);
		end -= (DataStartPos + 4);

		unsigned int* IntBuffer = NULL;
		float* FloatBuffer = NULL;
		double* DoubleBuffer = NULL;
		unsigned long size = N_Cols * 2;

		switch (DataFormat)
		{
		case 0:   // unsigned short
		{
			end /= (N_Cols * 2);
			N_Rows = end;
			IntBuffer = (unsigned int*)calloc(N_Cols, sizeof(unsigned int));
		}
		break;

		case 1:   // float
		{
			end /= (N_Cols * 4);
			N_Rows = end;
			FloatBuffer = (float*)calloc(N_Cols, sizeof(float));
			size *= 2;
		}
		break;

		case 2:   // double
		{
			end /= (N_Cols * 8);
			N_Rows = end;
			DoubleBuffer = (double*)calloc(N_Cols, sizeof(double));
			size *= 4;
		}
		break;
		}

		unsigned long nread;
		DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));
		T dTmp;
		for (unsigned long i = 0; i < N_Rows; i++)
		{
			T* Line = GetInternalLine(i);
			switch (DataFormat)
			{
			case 0:   // unsigned int
			{
				nread = fread(IntBuffer, 2, N_Cols, pBinFile);
				if (nread != N_Cols)
				{
					printf("\nError at reading a binary file: unexpected end of data. CLargeDataMatrix::ReadBinDataFile");
					return false;
				}
				for (unsigned long n = 0; n < N_Cols; n++) Line[n] = IntBuffer[n];
			}
			break;

			case 1:   // float
			{
				nread = fread(FloatBuffer, 4, N_Cols, pBinFile);
				if (nread != N_Cols)
				{
					printf("\nError at reading a binary file: unexpected end of data. CLargeDataMatrix::ReadBinDataFile");
					return false;
				}
				for (unsigned long n = 0; n < N_Cols; n++) Line[n] = FloatBuffer[n];
			}
			break;

			case 2:   // double
			{
				nread = fread(DoubleBuffer, 8, N_Cols, pBinFile);
				if (nread != N_Cols)
				{
					printf("\nError at reading a binary file: unexpected end of data. CLargeDataMatrix::ReadBinDataFile");
					return false;
				}
				for (unsigned long n = 0; n < N_Cols; n++) Line[n] = DoubleBuffer[n];
			}
			break;
			}
		}

		if (IntBuffer != NULL) free(IntBuffer);
		if (FloatBuffer != NULL) free(FloatBuffer);
		if (DoubleBuffer != NULL) free(DoubleBuffer);
		return true;
	}



protected:
	unsigned long N_Rows; // Number of rows
	unsigned long N_Cols; // Number of columns
	T * DataMatrix;  // Internal pseudo 2D array of data
	char FileName[1000]; // Name of the related text file
	CLargeDataVector<T> * AColumn; // One column for temporary usage
	CLargeDataVector<T> * ARow; // One row for temporary usage

public:
	//_________________________________   public methods  ______________________________________

	// Constructor 1 (data file name)
	CLargeDataMatrix(const char* FileName)
	{
		Nulling();
		ReadDataFile(FileName);
	}

	// Constructor 2 (empty object)
	CLargeDataMatrix(const unsigned long NumberOfRows, const unsigned long NumberOfColumns)
	{
		Nulling();
		N_Rows = NumberOfRows;
		N_Cols = NumberOfColumns;
		DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));

		AColumn = new CLargeDataVector<T>(N_Rows);
		ARow = new CLargeDataVector<T>(N_Cols);
	}


	// Constructor 3 (copy)
	CLargeDataMatrix(CLargeDataMatrix<T>* ExtMatrix)
	{
		Nulling();
		if (!ExtMatrix->IsDataReady()) return;
		N_Cols = ExtMatrix->GetNumberOfCols();
		N_Rows = ExtMatrix->GetNumberOfRows();
		T* ExtDataMatrix = ExtMatrix->GetInternalDataMatrix();

		DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));
		const unsigned long TotalSize = N_Rows * N_Cols;
		for (unsigned long i = 0; i < TotalSize; i++) DataMatrix[i] = ExtDataMatrix[i];

		AColumn = new CLargeDataVector<T>(N_Rows);
		ARow = new CLargeDataVector<T>(N_Cols);
	}

	// Constructor 4 (fragment of a text file)
	CLargeDataMatrix(FILE* pFile, fpos_t InitPos)
	{
		Nulling();

		// 1. Check the presence of the file 
		bool out = true;
		std::ifstream infile;
		infile.open(FileName);
		if (!infile.is_open())
		{
			printf("\nThe file %s doesn't exist\n", FileName);
			return;
		}
		unsigned long cols = 0;
		unsigned long lines = 0;

		// 2. Find the number of rows 
		std::string line;
		infile.seekg(InitPos, std::ios::beg);	// Reset to the start of the file

		while (!infile.eof())
		{
			getline(infile, line);
			lines++;
		}
		if (lines > 0) lines--;
		if (lines <= 0)
		{
			printf("\nUnable to extract a data matrix from the text file.");
			return;
		}
		this->N_Rows = lines;

		// 3. Find the number of cols
		infile.clear();
		infile.seekg(InitPos, std::ios::beg);	// Reset to the start of the file
		getline(infile, line);

		unsigned long temp = line.size();
		if (temp == 1)
		{
			getline(infile, line);
			temp = line.size();
		}

		for (unsigned long i = 0; i < temp; i++) if (line[i] == Delimiter) cols++;
		cols++;
		if (temp > 1)
		{
			// if (((string[temp - 1] == '\r') || (string[temp - 1] == '\n')) && (string[temp - 2] == '\t')) cols--;
			if ((line[temp - 2] == Delimiter) || (line[temp - 1] == Delimiter) || (line[temp - 1] == '\r') || (line[temp - 1] == '\n')) cols--;
		}

		if ((lines == 0) || (cols < 1))
		{
			printf("\nThe matrix in a file is empty or has a wrong format.");
			printf("\nLines = %ld; Columns = %ld.", lines, cols);
			printf("\n");
			return;
		}
		N_Rows = lines;
		N_Cols = cols;

		// 4. Read data
		DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));
		T dTmp;
		infile.clear();
		infile.seekg(InitPos, std::ios::beg);	// Reset to the start of the file

		for (unsigned long i = 0; i < N_Rows; i++)
		{
			getline(infile, line);
			ResetRow(i, line.data(), N_Cols);
			/*
			for (unsigned long n = 0; n < N_Cols; n++)
			{
				const char* cstr = line.data();
				sscanf(cstr, StrType, &dTmp);
				DataMatrix[n + i * N_Cols] = dTmp;
			}
			*/

		}
		infile.close();

		if (AColumn != NULL)
		{
			if (AColumn->GetSize() != N_Rows)
			{
				delete AColumn;
				AColumn = NULL;
			}
		}
		if (ARow != NULL)
		{
			if (ARow->GetSize() != N_Cols)
			{
				delete ARow;
				ARow = NULL;
			}
		}
		if (AColumn == NULL) AColumn = new CLargeDataVector<T>(N_Rows);
		if (ARow == NULL) ARow = new CLargeDataVector<T>(N_Cols);
	}

	// Constructor 5 (binary file)
	CLargeDataMatrix(FILE* pBinFile, const unsigned int DataStartPos, const unsigned int DataFormat)
	{
		Nulling();
		if (pBinFile == NULL)
		{
			printf("\nError. Binary data file is not initialized. Constructor of CLargeDataMatrix");
			return;
		}

		ReadBinDataFile(pBinFile, DataStartPos, DataFormat);

		AColumn = new CLargeDataVector<T>(N_Rows);
		ARow = new CLargeDataVector<T>(N_Cols);
	}

	// Constructor 6 (binary file)
	CLargeDataMatrix(const char* BinFileName, const unsigned int DataStartPos, const unsigned int DataFormat)
	{
		Nulling();
		FILE* pBinFile = fopen(BinFileName, "rb");
		if (pBinFile == NULL)
		{
			printf("\nError. Binary data file %s is not readable. Constructor of CLargeDataMatrix", BinFileName);
			return;
		}

		ReadBinDataFile(pBinFile, DataStartPos, DataFormat);
		fclose(pBinFile);

		AColumn = new CLargeDataVector<T>(N_Rows);
		ARow = new CLargeDataVector<T>(N_Cols);
	}


	// Destructor
	virtual ~CLargeDataMatrix()
	{
		if (DataMatrix != NULL) free(DataMatrix);
		if (AColumn != NULL) delete AColumn;
		if (ARow != NULL) delete ARow;
	}
	
	// Write the data into a text file
	bool WriteDataIntoFile(const char* OutputFileName, const bool TransposeAtWriting = false)
	{
		FILE* OutputFile = fopen(OutputFileName, "w"); // creating the output file
		if (OutputFile == NULL)
		{
			printf("\nFile %s doesn't exist", OutputFileName);
			return false;
		}

		if (TransposeAtWriting)
		{
			for (long c = 0; c < N_Cols; c++)
			{
				for (long r = 0; r < N_Rows - 1; r++)
				{
					fprintf(OutputFile, StrType, DataMatrix[c + r * N_Cols]);
					fprintf(OutputFile, "%c", Delimiter);
				}
				fprintf(OutputFile, StrType, DataMatrix[c + (N_Rows - 1) * N_Cols]);
				fprintf(OutputFile, "\n");
			}
		}
		else
		{
			for (long r = 0; r < N_Rows; r++)
			{
				for (long c = 0; c < N_Cols - 1; c++)
				{
					fprintf(OutputFile, StrType, DataMatrix[c + r * N_Cols]);
					fprintf(OutputFile, "%c", Delimiter);
				}
				fprintf(OutputFile, StrType, DataMatrix[(N_Cols - 1) + r * N_Cols]);
				fprintf(OutputFile, "\n");
			}
		}

		fclose(OutputFile);
		return true;
	}


	// Write the data into a text file
	bool WriteDataIntoFile(FILE* OutputFile, const bool TransposeAtWriting = false)
	{
		if (OutputFile == NULL)
		{
			printf("\nOutput file is not ready");
			return false;
		}

		if (TransposeAtWriting)
		{
			for (long c = 0; c < N_Cols; c++)
			{
				for (long r = 0; r < N_Rows - 1; r++)
				{
					fprintf(OutputFile, StrType, DataMatrix[c + r * N_Cols]);
					fprintf(OutputFile, "%c", Delimiter);
				}
				fprintf(OutputFile, StrType, DataMatrix[c + (N_Rows - 1) * N_Cols]);
				fprintf(OutputFile, "\n");
			}
		}
		else
		{
			for (long r = 0; r < N_Rows; r++)
			{
				for (long c = 0; c < N_Cols - 1; c++)
				{
					fprintf(OutputFile, StrType, DataMatrix[c + r * N_Cols]);
					fprintf(OutputFile, "%c", Delimiter);
				}
				fprintf(OutputFile, StrType, DataMatrix[(N_Cols - 1) + r * N_Cols]);
				fprintf(OutputFile, "\n");
			}
		}

		return true;
	}


	// Write data into a binary file
	bool WriteDataIntoBinFile(FILE* OutputFile, const bool Need_N_Cols_Info, const bool TransposeAtWriting = false)
	{
		if (OutputFile == NULL)
		{
			printf("\nOutput file is not ready");
			return false;
		}

		if (Need_N_Cols_Info)
		{
			if (TransposeAtWriting)
			{
				unsigned int nn[1] = { N_Rows };
				fwrite(nn, 4, 1, OutputFile);
			}
			else
			{
				unsigned int nn[1] = { N_Cols };
				fwrite(nn, 4, 1, OutputFile);
			}
		}
		const unsigned int n_bytes = sizeof(T);

		if (TransposeAtWriting)
		{
			T* Line = AColumn->GetInternalData();
			for (unsigned long c = 0; c < N_Cols; c++)
			{
				for (unsigned long r = 0; r < N_Rows; r++) Line[r] = DataMatrix[c + r * N_Cols];
				fwrite(Line, n_bytes, N_Rows, OutputFile);
			}
		}
		else // if (!TransposeAtWriting)
		{
			for (unsigned long r = 0; r < N_Rows; r++)   
			{
				T* Line = GetInternalLine(r);
				fwrite(Line, n_bytes, N_Cols, OutputFile);
			}
		} // end else (TransposeAtWriting)

		return true;
	}


	// Prints the matrix to the console
	void PrintData(const unsigned long NumberOfPrintedRows = 0)
	{
		printf("\n");
		unsigned long N = NumberOfPrintedRows;
		if ((N == 0) || (N > N_Rows)) N = N_Rows;
		for (unsigned long r = 0; r < N; r++)
		{
			for (unsigned long c = 0; c < N_Cols; c++)
			{
				printf(StrType, DataMatrix[c + r * N_Cols]);
				if(c != (N_Cols-1)) printf("%c", Delimiter);
			}
			printf("\n");
		}
	}


	// Multiplies the matrix by a constant value
	void MultiplyByConstant(const T Multiplier)
	{
		const unsigned long N = N_Cols * N_Rows;
		for (unsigned long i = 0; i < N; i++) DataMatrix[i] *= Multiplier;
	}


	// Adds elements of another matrix to this one. Sizes must be identical
	bool AddMatrix(CLargeDataMatrix* AddedMatrix)
	{
		if (AddedMatrix == NULL)
		{
			printf("\nError in matrix operations: one matrix doesn't exist");
			return false;
		}
		if (!AddedMatrix->IsDataReady())
		{
			printf("\nError in matrix operations: one matrix is not ready");
			return false;
		}

		if ((N_Cols != AddedMatrix->N_Cols) || (N_Rows != AddedMatrix->N_Rows))
		{
			printf("\nSizes of matrices to be combined are not identical");
			return false;
		}
		T* OutDM = AddedMatrix->DataMatrix;
		const unsigned long N = N_Cols * N_Rows;
		for (unsigned long i = 0; i < N; i++) DataMatrix[i] += OutDM[i];
		return true;
	}
	

	// Calculates the second norm
	T CalculateNorm()
	{
		const unsigned long N = N_Cols * N_Rows;
		T a, sum = 0;
		for (unsigned long i = 0; i < N; i++)
		{
			a = DataMatrix[i];
			sum += a * a;
		}
		return sum;
	}
	
	// __________________________________  Access to Properties  ___________________________________________

	// Get the flag of readiness
	bool IsDataReady()
	{
		if ((DataMatrix == NULL) || (N_Rows == 0) || (N_Cols == 0) || (AColumn == NULL) || (ARow == NULL)) return false;
		else return true;
	}


	// Resets one element of the matrix in a given position
	void ResetValue(const unsigned long Row_ID, const unsigned long Col_ID, const T NewValue)
	{
		DataMatrix[Col_ID + Row_ID * N_Cols] = NewValue;
	}


	// Gets one element of the matrix in a given position
	T GetValue(const unsigned long Row_ID, const unsigned long Col_ID)
	{
		return DataMatrix[Col_ID + Row_ID * N_Cols];
	}


	// Gets an internal line of the matrix
	T* GetInternalLine(const unsigned long Row_ID)
	{
		T* Vector = &DataMatrix[Row_ID * N_Cols];
		return Vector;

	}

	// Gets the internal array of data
	T* GetInternalDataMatrix()
	{
		return DataMatrix;
	}


	unsigned long GetNumberOfCols() {return N_Cols;} // Gets the number of columns
	unsigned long GetNumberOfRows() {return N_Rows;} // Gets the number of rows
	char * GetFileName() {return FileName;}	// Get the library data filename.

	// Resets values in a column
	bool ResetColumn(const unsigned long ColumnID, CLargeDataVector<T>* Vector)
	{
		if (Vector == NULL) return false;
		if (Vector->GetSize() != N_Rows) return false;
		if (ColumnID >= N_Cols) return false;
		T* VectorContent = Vector->GetInternalData();
		for (unsigned long r = 0; r < N_Rows; r++) ResetValue(r, ColumnID, VectorContent[r]);
		return true;
	}


	// Resets values in a row
	bool ResetRow(const unsigned long RowID, CLargeDataVector<T>* Vector)
	{
		if (Vector == NULL) return false;
		if (Vector->GetSize() != N_Cols) return false;
		if (RowID >= N_Rows) return false;
		T* VectorContent = Vector->GetInternalData();
		for (unsigned long c = 0; c < N_Cols; c++) ResetValue(RowID, c, VectorContent[c]);
		return true;
	}


	// Creates a new matrix, which is a submatrix of the original matrix
	CLargeDataMatrix* ExtractSubmatrix(const unsigned long StartRowID, const unsigned long EndRowID, const unsigned long StartColID, const unsigned long EndColID)
	{
		if (StartRowID > EndRowID) return NULL;
		if (StartColID > EndColID) return NULL;
		if (EndRowID >= N_Rows) return NULL;
		if (EndColID >= N_Cols) return NULL;
		CLargeDataMatrix* SubMatrix = new CLargeDataMatrix(EndRowID - StartRowID + 1, EndColID - StartColID + 1);

		for (unsigned long r = StartRowID; r <= EndRowID; r++) for (unsigned long c = StartColID; c <= EndColID; c++)
		{
			T a = DataMatrix[r * N_Cols + c];
			SubMatrix->ResetValue(r - StartRowID, c - StartColID, a);
		}

		return SubMatrix;
	}


	// Copy a line (SourceLineID) from this library to the destination matrix (DestLineID). If pDestMatrix == NULL, then copy to the same matrix
	void CopyLine(const unsigned long SourceLineID, const unsigned long DestLineID, CLargeDataMatrix* pDestMatrix = NULL)
	{
		T* pDest;
		T* pSource = &DataMatrix[SourceLineID * N_Cols];
		if (pDestMatrix == NULL) pDest = &DataMatrix[DestLineID * N_Cols];
		else
		{
			T* pDestInternalData = pDestMatrix->DataMatrix;
			pDest = &pDestInternalData[DestLineID * N_Cols];
		}
		for (unsigned long c = 0; c < N_Cols; c++) pDest[c] = pSource[c];
	}


	// Creates a new matrix, which contains only the columns those IDs are enumerated in the input string
	CLargeDataMatrix* ExtractColumns(const char* XChannels)
	{
		const unsigned int StrLen = strlen(XChannels);
		char TmpStr[4];
		unsigned int id = 0;
		unsigned int X_Size = 0;
		while (true)
		{
			const char* pstr = &XChannels[id];
			unsigned int Tmp;
			int a = sscanf(pstr, "%d", &Tmp);
			if (a < 0) break;
			sprintf(TmpStr, "%d", Tmp);
			id += strlen(TmpStr) + 1;
			if ((id < StrLen) && ((XChannels[id] == ' ') || (XChannels[id] == '	'))) id++;
			if ((Tmp >= 0) && (Tmp < N_Cols)) X_Size++;
			if (id >= StrLen) break;
		}

		unsigned int* XLine = (unsigned int*)calloc(X_Size, sizeof(unsigned int));

		id = 0;
		unsigned int count = 0;
		printf("\nSelected columns: ");
		while (true)
		{
			const char* pstr = &XChannels[id];
			unsigned int Tmp;
			int a = sscanf(pstr, "%d", &Tmp);
			if (a < 0) break;
			sprintf(TmpStr, "%d", Tmp);
			id += strlen(TmpStr) + 1;
			if ((id < StrLen) && ((XChannels[id] == ' ') || (XChannels[id] == '	'))) id++;
			if ((Tmp >= 0) && (Tmp < N_Cols))
			{
				XLine[count] = Tmp;
				printf("%d ", Tmp);
				count++;
			}
			if (id >= StrLen) break;
		}
		printf("\n");

		CLargeDataMatrix* Derived = new CLargeDataMatrix(N_Rows, X_Size);
		for (unsigned long r = 0; r < N_Rows; r++)
		{
			T* SourceVector = &DataMatrix[r * N_Cols];
			T* DestVector = &(Derived->GetInternalDataMatrix())[r * X_Size];
			for (unsigned long c = 0; c < X_Size; c++) DestVector[c] = SourceVector[XLine[c]];
		}

		free(XLine);
		return Derived;
	}


	// Copies the content of an existing object (true in case of success)
	bool CopyContent(CLargeDataMatrix* Source)
	{
		if (!Source->IsDataReady()) return false;
		if (Source->GetNumberOfCols() != N_Cols) return false;
		if (Source->GetNumberOfRows() != N_Rows) return false;
		if (DataMatrix == NULL) DataMatrix = (T*)calloc(N_Rows * N_Cols, sizeof(T));
		T* SMatrix = Source->DataMatrix;
		unsigned long N = N_Rows * N_Cols;
		for (unsigned long i = 0; i < N; i++) DataMatrix[i] = SMatrix[i];
		if (AColumn != NULL)
		{
			if (AColumn->GetSize() != N_Rows)
			{
				delete AColumn;
				AColumn = NULL;
			}
		}
		if (ARow != NULL)
		{
			if (ARow->GetSize() != N_Cols)
			{
				delete ARow;
				ARow = NULL;
			}
		}
		if (AColumn == NULL) AColumn = new CLargeDataVector<T>(N_Rows);
		if (ARow == NULL) ARow = new CLargeDataVector<T>(N_Cols);

		return true;
	}


	// Gets a column by its ID
	CLargeDataVector<T>* GetColumn(const unsigned long ColumnID)
	{
		T* Content = AColumn->GetInternalData();
		for (unsigned long r = 0; r < N_Rows; r++) Content[r] = DataMatrix[ColumnID + N_Cols * r];
		return AColumn;
	}


	// Gets a row by its ID
	CLargeDataVector<T>* GetRow(const unsigned long RowID)
	{
		T* Content = ARow->GetInternalData();
		T* DM = &DataMatrix[N_Cols * RowID];
		for (unsigned long c = 0; c < N_Cols; c++) Content[c] = DM[c];
		return ARow;
	}


	// Resets a row in the matrix by values in the input string
	bool ResetRow(const unsigned long RowID, const char* Str, unsigned int N_Elements = 0)
	{
		const unsigned int StrLen = strlen(Str);
		if (StrLen == 0) return false;

		// 1. Determining the size
		unsigned long id = 0;
		unsigned long count = 0;

		if (N_Elements == 0) while (true)
		{
			const char* pstr = &Str[id];
			T Tmp;
			int a = sscanf(pstr, StrType, &Tmp);
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
		else count = N_Elements;
		if (count != N_Cols) return false;

		// 2. Filling the array
		count = 0;
		id = 0;
		while (true)
		{
			const char* pstr = &Str[id];
			T Tmp;
			int a = sscanf(pstr, StrType, &Tmp);
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
			DataMatrix[count + N_Cols * RowID] = Tmp;
			count++;
			if (id >= StrLen) break;
		}
		return true;
	}
};






#endif /*LARGEDATAMATRIX_H_*/
