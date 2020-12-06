#include <chrono>
#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <clocale>


using namespace std;

#define N 1000


void RandMatrix(vector<vector<int>>& matrix)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            matrix[i][j] = 1 +  rand() % 10;
        }

    }
}



int main() {
    setlocale(LC_ALL, "russian");

    int sum = 0;
    vector<vector<int>> matrixA,matrixB,matrixC,matrixD;
    matrixA.resize(N, vector<int>(N,1));
    matrixB.resize(N, vector<int>(N,2));
    matrixC.resize(N, vector<int>(N,0));
    matrixD.resize(N, vector<int>(N,0));
    RandMatrix(matrixA);
    RandMatrix(matrixB);

    auto start  = omp_get_wtime();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrixC[i][j] = 0;
            for (int k = 0; k < N; k++) {
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }


    auto end = omp_get_wtime();

    auto time = end - start;
    cout << "Последовательные вычисления:" << time << endl;
//////////////////////////////////////////////////////////////////////////
    start = omp_get_wtime();

    omp_set_num_threads(4);

    int i, j, k = 0;
#pragma omp parallel for shared(matrixA,matrixB,matrixD) private (i,j,k)
    for (i = 0; i < N; i++) {

        for (j = 0; j < N; j++) {
            matrixD[i][j] = 0;
            for (k = 0; k < N; k++) {
                matrixD[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }


    end = omp_get_wtime();

    time = end - start;

    cout << "Последовательные вычисления:" << time<< endl;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (matrixC[i][j] != matrixD[i][j])
                cout << "Матрицы разные" << endl;
        }

    }
    
    return 0;

}
