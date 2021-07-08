#pragma once

#include "IMemory.h"

#define BANK_COUNT 2


class CanonicalMemory : public IMemory
{
public:
    char* database;
    char* currentData, * nextData;
    size_t currentBaseIndex; // Used to determine which bank is source and which is destination

    // 
    // Parameters
    // 
    size_t PopulationSize; // Exactly what the application asked for
    size_t ChromoSizeBytes; // Exactly what the application asked for
    std::function<float_t( const void* )> Evaluate; // Determine the return value for most IMemory methods


    // 
    // Precalculations
    // 
    size_t ChromoSizeBits; // Used often, so pre-calculate
    size_t ChromoStrideBytes; // represents the entire width of a chromo (in bytes)
    size_t PopulationStride; // Number of bytes required for one entire population.


    CanonicalMemory( size_t populationSize, size_t chromosomeSizeInBytes, std::function<float_t( const void* )> evaluator )
    {
        // Store parameters
        PopulationSize = populationSize;
        ChromoSizeBytes = chromosomeSizeInBytes;
        Evaluate = evaluator;

        // pre-calculate values which will be used once or more PER operation
        ChromoStrideBytes = ROUND_UP( ChromoSizeBytes, sizeof( uint_v ) );
        ChromoSizeBits = ChromoSizeBytes * 8;
        PopulationStride = ChromoStrideBytes * PopulationSize;

        // Allocate the memory
        const size_t populationAllocation = populationSize * BANK_COUNT * ChromoStrideBytes;
        database = new char[populationAllocation];

        // Initialize the page swapping variables
        currentBaseIndex = 0;
        NextGeneration();
    }

    ~CanonicalMemory()
    {
        delete[] database;
    }





    inline uint64_t* CopyChromo( size_t destinationIndex, size_t sourceIndex )
    {
        auto src = currentData + sourceIndex * ChromoStrideBytes;
        auto dest = nextData + destinationIndex * ChromoStrideBytes;

        // Stride should be vector-aligned, allowing the compiler to make this a vector-only
        // move internally (likely internal to REP STOx)
        memcpy( dest, src, ChromoStrideBytes );

        return (uint64_t*) dest;
    }

    inline static uint64_t Splice( uint64_t first, uint64_t second, size_t split )
    {
        uint64_t mask = (uint64_t( 1 ) << uint64_t( split )) - 1UL;
        uint64_t result = (first & mask) | (second & ~mask);
        return result;
    }





    virtual size_t GetPopulationCount() const override
    {
        return PopulationSize;
    }

    virtual void NextGeneration() override
    {
        nextData = database + currentBaseIndex;
        currentBaseIndex = PopulationStride - currentBaseIndex;
        currentData = database + currentBaseIndex;
    }

    virtual float_t Initialize( size_t index ) override
    {
        static uint_v rngState = 0;
        static bool rngInitialized = false;
        if (!rngInitialized)
        {
            SlowRng( rngState );
            rngInitialized = true;
        }

        uint_v* baseptr = (uint_v*) (nextData + index * ChromoStrideBytes);
        for (size_t i = 0; i < BLOCK_COUNT( ChromoSizeBytes, sizeof( uint_v ) ); i++)
        {
            rngState ^= rngState >> 13;
            rngState ^= rngState >> 17;
            rngState ^= rngState << 5;

            baseptr[i] = rngState;
        }

        return Evaluate( baseptr );
    }

    virtual float_t Copy( size_t destinationIndex, size_t sourceIndex ) override
    {
        uint64_t* dest = CopyChromo( destinationIndex, sourceIndex );
        return Evaluate( dest );
    }

    virtual float_t Mutate( size_t destinationIndex, size_t chromo1index, size_t bit1index ) override
    {
        uint64_t* dest64Ptr = CopyChromo( destinationIndex, chromo1index );

        size_t idx = bit1index % ChromoSizeBits;
        size_t bitIndex = idx & 63;
        size_t wordIndex = idx >> 6;
        uint64_t mask = uint64_t( 1 ) << uint64_t( bitIndex );

        dest64Ptr[wordIndex] ^= mask;

        return Evaluate( dest64Ptr );
    }

    virtual float_t Crossover( size_t destinationIndex, size_t chromo1index, size_t bit1index, size_t chromo2index, size_t bit2index ) override
    {
        size_t i1 = bit1index % ChromoSizeBits;
        size_t i2 = bit2index % ChromoSizeBits;

        if (i1 == i2) // this is a no-op
            return Copy( destinationIndex, chromo1index );

        size_t left = std::min( i1, i2 );
        size_t right = std::max( i1, i2 );

        size_t word1Index = left >> 6;
        size_t bit1Subindex = left & 63;
        size_t word2Index = right >> 6;
        size_t bit2Subindex = right & 63;

        uint64_t* destWordPtr = (uint64_t*) (nextData + destinationIndex * ChromoStrideBytes);
        uint64_t* src1WordPtr = (uint64_t*) (currentData + chromo1index * ChromoStrideBytes);
        uint64_t* src2WordPtr = (uint64_t*) (currentData + chromo2index * ChromoStrideBytes);

        const size_t end64BitIndex = BLOCK_COUNT( ChromoSizeBytes, sizeof( uint64_t ) );


        // have to treat the case where both bits in the same word
        if (word1Index == word2Index)
        {
            size_t idx = word1Index;

            for (size_t i = 0; i < idx; i++)
                destWordPtr[i] = src1WordPtr[i];

            auto tmp = Splice( src1WordPtr[idx], src2WordPtr[idx], bit1Subindex );
            destWordPtr[idx] = Splice( tmp, src1WordPtr[idx], bit2Subindex );

            for (size_t i = idx + 1; i < end64BitIndex; i++)
                destWordPtr[i] = src1WordPtr[i];
        }
        else
        {
            for (size_t i = 0; i < word1Index; i++)
                destWordPtr[i] = src1WordPtr[i];

            destWordPtr[word1Index] = Splice( src1WordPtr[word1Index], src2WordPtr[word1Index], bit1Subindex );

            for (size_t i = word1Index + 1; i < word2Index; i++)
                destWordPtr[i] = src2WordPtr[i];

            destWordPtr[word2Index] = Splice( src2WordPtr[word2Index], src1WordPtr[word2Index], bit2Subindex );

            for (size_t i = word2Index + 1; i < end64BitIndex; i++)
                destWordPtr[i] = src1WordPtr[i];
        }

        return Evaluate( destWordPtr );
    }
};

