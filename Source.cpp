#include <iostream>
#include <vector>
#include <string>
#include <malloc.h>
#include <time.h>
#include <mmintrin.h>

using namespace std;

int main()
{
    __int8  A[8] = { 1,2,3,4,5,6,7,8 };
    __int8  B[8] = { 1,2,3,4,5,6,7,1 };
    __int8  C[8] = { 1,2,3,4,5,6,7,1 };
    __int16 D[8] = { 1,2,3,4,5,6,7,8 };

    __int16 F[8] = { 0,0,0,0,0,0,0,0 };

    printf("F = (A + B) * (C + D)\n");
    printf("Result CPP = ");
    printf("{");

    for (int i = 0; i < 8; i++)
    {
        // printf("%d,", ((A1[i]+B1[i])*(C1[i]*D1[i])));
        cout << (A[i] + B[i]) * (C[i] + D[i]) << ",";
    }
    printf("}\n");

    __m64* pA = (__m64*) A;

    __m64* pB = (__m64*) B;

    __m64* pC = (__m64*) C;

    __m64* pD = (__m64*) D;


    __m64* pF = (__m64*) F;

    _mm_empty();

    __m64 MM0 = _mm_setzero_si64();

    // Берём младшие биты заносим в одну переменную, а старшие в другую
    // Тоже самое делаем и со вторым массивом
    // Затем поочерёдно складываем младшие и страшие разряды массивов
    // И пакуем в 8 бит

    __m64 Alow = _mm_unpacklo_pi8(*pA, MM0);

    __m64 AHigh = _mm_unpackhi_pi8(*pA, MM0);

    __m64 Blow = _mm_unpacklo_pi8(*pB, MM0);

    __m64 Bhigh = _mm_unpackhi_pi8(*pB, MM0);


    // A + B
    __m64 Sumlower_1 = _mm_add_pi16(Alow, Blow);
    __m64 SumHigh_1 = _mm_adds_pi16(AHigh, Bhigh);

    // C + D
    Alow = _mm_unpacklo_pi8(*pC, MM0);
    AHigh = _mm_unpackhi_pi8(*pC, MM0);

   __m64 Sumlower_2 = _mm_add_pi16(Alow, *pD);
   __m64 SumHigh_2 = _mm_adds_pi16(AHigh, *++pD);
        
   // (A + B) * (C + D)
   __m64 Mullower = _mm_mullo_pi16(Sumlower_1, Sumlower_2);
   __m64 Mulhiger = _mm_mullo_pi16(SumHigh_1, SumHigh_2);


    for (int i = 0, k = 4; i < 4; i++, k++)
    {
        F[i] = Mullower.m64_i16[i];
        F[k] = Mulhiger.m64_i16[i];
    }


    printf("Result MMX = ");
    printf("{");
    for (int i = 0; i < 8; i++)
    {
        printf("%d,", F[i]);
    }
    printf("}\n");


	return 0;
}
