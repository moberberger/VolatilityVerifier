#pragma once


class IMemory
{
public:
    virtual void NextGeneration() = 0;
    virtual size_t GetPopulationCount() const = 0;

    virtual float_t Initialize( size_t index ) = 0;
    virtual float_t Copy( size_t destinationIndex, size_t sourceIndex ) = 0;
    virtual float_t Mutate( size_t destinationIdx, size_t chromo1idx, size_t bit1idx ) = 0;
    virtual float_t Crossover( size_t destinationIdx, size_t chromo1idx, size_t bit1idx, size_t chromo2idx, size_t bit2idx ) = 0;
};

