/*
 * Main.cpp
 *
 *  Created on: Mar 09, 2022
  *      Author: Artem Khlybov
 */

#include "VectorMatrix/LargeDataMatrix.h"




void TestScript_LargeDataMatrix()
{
	CLargeDataMatrix<float>* Matrix0 = new CLargeDataMatrix<float>(3, 5);
	Matrix0->ResetRow(0, "1.1	1.2	1.3	1.4	1.5");
	Matrix0->ResetRow(1, "2.1	2.2	2.3	2.4	2.5");
	Matrix0->ResetRow(2, "3.1	3.2	3.3	3.4	3.5");
	CLargeDataMatrix<float>* Matrix1 = new CLargeDataMatrix<float>(Matrix0);
	Matrix1->MultiplyByConstant(10.0);
	printf("\nnorm=%f", Matrix1->CalculateNorm());
	Matrix1->AddMatrix(Matrix1);
	Matrix0->PrintData();
	Matrix1->PrintData();

	CLargeDataMatrix<float>* Matrix2 = Matrix0->ExtractSubmatrix(1, 2, 0, 2);
	Matrix2->PrintData();

	Matrix0->ResetColumn(1, Matrix1->GetColumn(1));
	Matrix0->ResetRow(1, Matrix1->GetRow(1));
	Matrix0->PrintData();
	Matrix0->WriteDataIntoFile("Tmp1.txt");
	CLargeDataMatrix<float>* Matrix3 = new CLargeDataMatrix<float>("Tmp1.txt");
	Matrix3->PrintData();

	FILE* pBinFile = fopen("Tmp2.txt", "wb"); // creating the output binary file
	Matrix0->WriteDataIntoBinFile(pBinFile, true, true);
	fclose(pBinFile);
	CLargeDataMatrix<float>* Matrix4 = new CLargeDataMatrix<float>("Tmp2.txt", 0, 1);
	Matrix4->PrintData();
	CLargeDataVector<float> * pV1 = Matrix4->GetColumn(2);
	pV1->PrintData();

	delete Matrix0;
	delete Matrix1;
	delete Matrix2;
	delete Matrix3;
	delete Matrix4;
}



void TestScript_LargeDataVector()
{
	CLargeDataVector<float>* Vector = new CLargeDataVector<float>("1	5	3	9");
	CLargeDataVector<float>* Vector1 = new CLargeDataVector<float>(Vector);
	CLargeDataVector<float>* Vector2 = new CLargeDataVector<float>(4);

	Vector->PrintData();
	Vector1->PrintData();

	char str[40];
	Vector1->ConvertToString(str);
	printf("\n");
	printf(str);
	Vector2->CopyContent(Vector);
	Vector2->ResetValue(0, 8.8);
	if (Vector2->IsDataReady()) Vector2->PrintData();
	Vector2->WriteDataIntoFile("Tmp.txt", false);

	FILE* pFile = fopen("Tmp.txt", "r");
	CLargeDataVector<float>* Vector3 = new CLargeDataVector<float>(pFile);
	fclose(pFile);
	Vector3->PrintData();

	delete Vector;
	delete Vector1;
	delete Vector2;
	delete Vector3;
}





int main()
{
	TestScript_LargeDataMatrix();
	return 0;
}