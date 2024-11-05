#ifndef CACHE_H
#define CACHE_H
#define L1_BLOCK_SZ_BYTES 64
#define L1_BLOCK_BITS 6 
#define L1_SETS 64
#define L1_SET_BITS 6
#define L1_WAYS 4 
#define L1_SZ_BYTES (L1_SETS*L1_WAYS*L1_BLOCK_SZ_BYTES)  
#define FULL_MASK 0xFFFFFFFFFFFFFFFF
#define TAG_MASK (FULL_MASK << (L1_SET_BITS + L1_BLOCK_BITS))
#define OFF_MASK (~(FULL_MASK << L1_BLOCK_BITS))
#define SET_MASK (~(TAG_MASK | OFF_MASK))

uint8_t dummyMem[5 * L1_SZ_BYTES];

void flushCache(uint64_t addr, uint64_t sz)
{

    uint64_t numSetsClear = sz >> L1_BLOCK_BITS;
    if ((sz & OFF_MASK) != 0)
    {
        numSetsClear += 1;
    }

    if (numSetsClear > L1_SETS)
    {
        numSetsClear = L1_SETS;
    }
    
    uint8_t dummy = 0;
    uint64_t alignedMem = (((uint64_t)dummyMem) + L1_SZ_BYTES) & TAG_MASK;
        
    for (uint64_t i = 0; i < numSetsClear; ++i)
    {
        uint64_t setOffset = (((addr & SET_MASK) >> L1_BLOCK_BITS) + i) << L1_BLOCK_BITS;
        for(uint64_t j = 0; j < 4*L1_WAYS; ++j)  
        {
            uint64_t wayOffset = j << (L1_BLOCK_BITS + L1_SET_BITS);
            dummy = *((uint8_t*)(alignedMem + setOffset + wayOffset));
        }
    }
}

#endif