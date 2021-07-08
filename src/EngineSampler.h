#pragma once

#include "ProjectHeader.h"


class EngineSampler
{
public:
    float_t _bottom[VecElementCount * VecElementCount * VecElementCount];
    float_t _middle[VecElementCount * VecElementCount];
    float_t    _top[VecElementCount];


    EngineSampler()
    {
        for (size_t i = 0; i < MaxPopCount; i++)
        {
            _bottom[i] = FLT_MAX;
            _middle[i >> 3] = FLT_MAX;
            _top[i >> 6] = FLT_MAX;
        }
    }


    inline float_v BuildKeys( float_v* data, size_t populationCount )
    {
        // inverting each float can be done vectorized
        for (size_t i = 0; i < MaxPopVectorCount; i += 2)
        {
            data[i + 0] = approx_recipr( data[i + 0] );
            data[i + 1] = approx_recipr( data[i + 1] );
        }

        // build aliases for readability
        float_t* ptr = (float_t*) data;
        float_t* endptr = ptr + populationCount;
        float_t* tptr = _top;
        float_t* mptr = _middle;
        float_t* bptr = _bottom;
        float_t sum = 0;

        for (size_t itop = 0; itop < 8; itop++)
        {
            for (size_t imid = 0; imid < 8; imid++)
            {
                for (size_t ibot = 0; ibot < 8; ibot++)
                {
                    if (ptr >= endptr)
                        return sum;

                    sum += *ptr++;
                    *bptr++ = sum;
                }
                *mptr++ = sum;
            }
            *tptr++ = sum;
        }

        return sum;
    }


    inline static uint_t GetIndex( float_v vSample, float_v keys )
    {
        auto mask = _mm256_cmp_ps( keys, vSample, _CMP_LE_OS );
        uint_t bits = _mm256_movemask_ps( mask );
        bits = ~bits;
        return _tzcnt_u32( bits );
    }

    /// <summary>
    /// 
    /// inline void CreateSamplingIndicies()
    /// 
    /// INPUT:
    /// 
    /// [param] scaledFloatValues :: floating point values, to be used to index into the population
    /// using the keys
    /// 
    /// [param] sampleSetCount :: Instructs method on how many samples to generate
    /// 
    /// OUTPUT:
    /// 
    /// [param] outputIndicies :: The index of each respective value, using the Keys arrays for
    /// lookup
    /// 
    /// </summary>
    /// <remarks>
    /// 
    /// Why Twelve? Dunno, but ideas-
    /// 
    /// 1. Compiler ran out of perceived register space. 4 YMMs left over after 12 used for
    /// loop-carried dependencies
    /// 
    /// 2. Batching 12 memory reads / writes at these sizes happens to match up perfectly with
    /// L1 cache's prefetch algorithms
    /// 
    /// 3. May not be 12 on other processors- need to test. On my coffee lake, 12 is the magic
    /// number.
    /// 
    /// 4. The 12 may represent the second (or higher ordinal) "sweet spot" for memory access
    /// which, when combined with instruction pipelining, made 12 better than other sweet spots
    /// 
    /// </remarks>

#define LOAD(N)        float_v raw##N = scaledFloatValues[i + N]
#define TOP_INDEX(N)    uint_t index##N = GetIndex( raw##N, top )
#define MIDDLE_INDEX(N)        index##N = VecElementCount * index##N + GetIndex( raw##N, middle[index##N] );
#define BOTTOM_INDEX(N)        index##N = VecElementCount * index##N + GetIndex( raw##N, bottom[index##N] );
#define STORE(N)               outputIndicies[i + N] = index##N;

    inline void CreateSamplingIndicies( float_t scaledFloatValues[], uint_t outputIndicies[], size_t sampleSetCount )
    {
        auto ptop = (float_v*) _top;
        float_v top = *ptop;
        auto middle = (float_v*) _middle;
        auto bottom = (float_v*) _bottom;

        for (size_t i = 0; i < sampleSetCount; i += 12)
        {
            LOAD( 0 );
            LOAD( 1 );
            LOAD( 2 );
            LOAD( 3 );
            LOAD( 4 );
            LOAD( 5 );
            LOAD( 6 );
            LOAD( 7 );
            LOAD( 8 );
            LOAD( 9 );
            LOAD( 10 );
            LOAD( 11 );

            TOP_INDEX( 0 );
            TOP_INDEX( 1 );
            TOP_INDEX( 2 );
            TOP_INDEX( 3 );
            TOP_INDEX( 4 );
            TOP_INDEX( 5 );
            TOP_INDEX( 6 );
            TOP_INDEX( 7 );
            TOP_INDEX( 8 );
            TOP_INDEX( 9 );
            TOP_INDEX( 10 );
            TOP_INDEX( 11 );

            MIDDLE_INDEX( 0 );
            MIDDLE_INDEX( 1 );
            MIDDLE_INDEX( 2 );
            MIDDLE_INDEX( 3 );
            MIDDLE_INDEX( 4 );
            MIDDLE_INDEX( 5 );
            MIDDLE_INDEX( 6 );
            MIDDLE_INDEX( 7 );
            MIDDLE_INDEX( 8 );
            MIDDLE_INDEX( 9 );
            MIDDLE_INDEX( 10 );
            MIDDLE_INDEX( 11 );

            BOTTOM_INDEX( 0 );
            BOTTOM_INDEX( 1 );
            BOTTOM_INDEX( 2 );
            BOTTOM_INDEX( 3 );
            BOTTOM_INDEX( 4 );
            BOTTOM_INDEX( 5 );
            BOTTOM_INDEX( 6 );
            BOTTOM_INDEX( 7 );
            BOTTOM_INDEX( 8 );
            BOTTOM_INDEX( 9 );
            BOTTOM_INDEX( 10 );
            BOTTOM_INDEX( 11 );

            STORE( 0 );
            STORE( 1 );
            STORE( 2 );
            STORE( 3 );
            STORE( 4 );
            STORE( 5 );
            STORE( 6 );
            STORE( 7 );
            STORE( 8 );
            STORE( 9 );
            STORE( 10 );
            STORE( 11 );
        }
    }

};

