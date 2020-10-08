#include <iostream>
#include <vector>
#include <string>
#include <mmintrin.h>

using namespace std;

int main()
{
    __int8  A[8] = { 1,2,-30,4,5,-63,-7,8 };
    __int8  B[8] = { 3,-8,3,-14,5,6,27,1 };
    __int8  C[8] = { 2,2,3,4,11,-6,-7,1 };
    __int16 D[8] = { 1,3,4,23,5,200,32,8 };

    __int16 F[8] = { 0,0,0,0,0,0,0,0 };

    printf("F = (A + B) * (C + D)\n");
    printf("Result CPP = ");
    printf("{");

    for (int i = 0; i < 8; i++)
    {
        cout << (A[i] + B[i]) * (C[i] + D[i]) << ",";
    }
    printf("}\n");

    __m64* pA = (__m64*) A;

    __m64* pB = (__m64*) B;

    __m64* pC = (__m64*) C;

    __m64* pD = (__m64*) D;

    __m64* pF = (__m64*) F;
        
    
    __m64 v0 = _mm_setzero_si64();
    __m64 compareA = _m_pcmpgtb(v0, *pA);
    __m64 compareB = _m_pcmpgtb(v0, *pB);
    __m64 compareC = _m_pcmpgtb(v0, *pC);

    __m64 A16[2] = { _mm_unpacklo_pi8(*pA, compareA), _mm_unpackhi_pi8(*pA, compareA) };

    __m64 B16[2] = { _mm_unpacklo_pi8(*pB, compareB) , _mm_unpackhi_pi8(*pB, compareB) };

    __m64 C16[2] = { _mm_unpacklo_pi8(*pC, compareC), _mm_unpackhi_pi8(*pC, compareC) };

    __m64 Sum1[2];
    __m64 Sum2[2];
    __m64 Mul[2];

    for (int i = 0; i < 2; i++, *pD++)
    {
        Sum1[i] = _mm_adds_pi16(A16[i], B16[i]);
        Sum2[i] = _mm_adds_pi16(C16[i], *pD);
        Mul[i] = _mm_mullo_pi16(Sum1[i], Sum2[i]);
    }
    

    for (int i = 0, k = 4; i < 4; i++, k++)
    {
        F[i] = Mul[0].m64_i16[i];
        F[k] = Mul[1].m64_i16[i];
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
