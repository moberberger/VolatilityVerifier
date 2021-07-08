#include "PreCompileHeader.h"

#include "Rng.h"


void Rng::SlowSeed(char* buf, size_t len)
{
    char* p = buf;
    for (size_t iByte = 0; iByte < len; iByte++)
    {
        std::this_thread::yield();
        for (size_t iBit = 0; iBit < 8; iBit++)
        {
            *p <<= 1;
            *p ^= __rdtsc() & 0x0f; // 4-bit window- arbitrary
            std::this_thread::yield();

            // should at least quadruple the standard deviation of expectable __rdtsc()
            // return values
            if (*p & 1)
            {
                std::this_thread::yield();
            }
            if (*p & 2)
            {
                std::this_thread::yield();
                std::this_thread::yield();
            }
        }
        std::this_thread::yield();
        *p++ ^= __rdtsc() & 0xff;
    }
    uint64_t* pFirst8 = (uint64_t*)buf;
    size_t repeatCount = __rdtsc() & 0x3ff; // up to 1000 times
    for (size_t i = 0; i < repeatCount; i++)
        *pFirst8 = Next_Linear64(*pFirst8); // destroy any original sequencing through affine transformation
}
