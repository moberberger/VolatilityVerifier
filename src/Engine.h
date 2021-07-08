#pragma once

#include "EngineRng.h"
#include "EngineSampler.h"
#include "IMemory.h"
#include "Timer.h"



#define Sample_SET_Allocation ROUND_UP(MaxPopCount * 2, 12)
#define Sample_VEC_Allocation (Sample_SET_Allocation / VecElementCount)



class Engine
{
public:
    union UBank0
    {
        UBank0() { }
        float_t NextDeviations[Sample_SET_Allocation];
        float_v vNextDeviations[Sample_VEC_Allocation];

        float_t RawFloatValues[Sample_SET_Allocation];
        float_v vRawFloatValues[Sample_VEC_Allocation];

        uint_t ChromoIndicies[Sample_SET_Allocation];
    } Bank0;

    uint_t BitIndicies[Sample_SET_Allocation];

    EngineSampler sampler;
    EngineRng rng;


    // Configuration
    size_t PopulationCount;
    float_t MutationRate = 0.05f;
    IMemory& Memory;

    // Derived Data
    size_t XoverCount;
    size_t SampleSetCount;

    // This is the only data carried forward from generation to generation
    size_t BestIndexForGeneration;


    // Timers
    Timer indiciesTimer;
    Timer samplesTimer;
    Timer samplingIndiciesTimer;
    Timer evolutionTimer;


    /// <summary>
    /// Construct and Initialize the engine
    /// </summary>
    /// <param name="memory"></param>
    Engine( IMemory& memory ) :
        Memory( memory ),
        indiciesTimer( "     Building Keys" ),
        samplesTimer( "Generating Samples" ),
        samplingIndiciesTimer( "Samples 2 Indicies" ),
        evolutionTimer( "  Evolving Chromos" )
    {
        PopulationCount = memory.GetPopulationCount();

        size_t mutationCount = (size_t) (PopulationCount * (double) MutationRate + 0.5);
        XoverCount = PopulationCount - mutationCount;
        SampleSetCount = XoverCount * 2 + mutationCount + 1;

        Initialize();
    }


    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    float_t EvolveOneGeneration()
    {
        evolutionTimer.Start();


        indiciesTimer.Start();
        float_v scale = BuildKeys();
        indiciesTimer.End();

        samplesTimer.Start();
        CreateSamples( scale );
        samplesTimer.End();

        samplingIndiciesTimer.Start();
        CreateSamplingIndicies();
        samplingIndiciesTimer.End();


        evolutionTimer.End();


        float_t best = Evolve();
        return best;
    }



    /// <summary>
    /// Reset the engine (and memory) without changing the configuration. The constructor calls
    /// this. Don't use this unless the application requires "reset using current config"
    /// functionality
    /// 
    /// INPUT:
    /// 
    /// 
    /// OUTPUT:
    /// 
    /// [member] NextDeviations :: The deviations of each (presumably) random initial
    /// chromosome.
    /// 
    /// [member] BestDeviationForGeneration :: The index of the best deviation, used for Elitism
    /// 
    /// </summary>
    void Initialize()
    {
        ClearMem( Bank0 );

        float_t min = FLT_MAX;
        size_t minIndex = 0;

        // Initialize to max values to make sure Osearch works when N .LT. 512
        for (size_t i = 0; i < PopulationCount; i++)
        {
            float_t dev = Bank0.NextDeviations[i] = Memory.Initialize( i );
            if (dev < min) { min = dev; minIndex = i; }
        }

        BestIndexForGeneration = minIndex;
    }


    /// <summary>
    /// 
    /// inline float_t BuildKeys()
    /// 
    /// INPUT:
    /// 
    /// [member] NextDeviations[PopulationSize] :: The deviation for each corresponding
    /// chromosome in the sample set
    /// 
    /// 
    /// OUTPUT:
    /// 
    /// [member] SamplerKeys :: Contains the keys used for sampling lookup
    /// 
    /// [return] scale :: The maximum key value; aka the sum of all inverted deviations; aka the
    /// scale for the random floating point number used when generating the random sample
    /// floating point data.
    /// 
    /// </summary>
    inline float_v BuildKeys()
    {
        return sampler.BuildKeys( Bank0.vNextDeviations, PopulationCount );
    }


    /// <summary>
    /// 
    /// inline void CreateSamples( float_t scale )
    /// 
    /// INPUT:
    /// 
    /// [member] rng :: generation of sample values (floats) and bit indicies (uints) for as
    /// many samples as required, based on Xover and Mutate counts.
    /// 
    /// [param] scale :: set the RNG output of floating point numbers scaled [0..scale)
    /// 
    /// 
    /// OUTPUT:
    /// 
    /// [member] RawFloatValues[SampleSetCount] :: floating point values, to be used to index
    /// into the population using the keys, scaled as specified
    /// 
    /// [member] BitIndicies[SampleSetCount] :: Index into the sampled chromosome where
    /// evolution is supposed to take action, 32 random bits
    /// 
    /// </summary>
    inline void CreateSamples( float_v scale )
    {
        rng.CreateSamples( scale, SampleSetCount, BitIndicies, Bank0.vRawFloatValues );
    }


    /// <summary>
    /// 
    /// inline void CreateSamplingIndicies()
    /// 
    /// INPUT:
    /// 
    /// [member] RawFloatValues[SampleSetCount] :: Scaled random floating point numbers used to
    /// lookup and generate an index
    /// 
    /// 
    /// OUTPUT:
    /// 
    /// [member] Indicies[SampleSetCount] :: Integer indicies representing chromosomes in the
    /// population, according to the sampler's keys
    /// 
    /// </summary>
    inline void CreateSamplingIndicies()
    {
        sampler.CreateSamplingIndicies( Bank0.RawFloatValues, Bank0.ChromoIndicies, SampleSetCount );
    }


    /// <summary>
    /// 
    /// inline float_t Evolve()
    /// 
    /// INPUT:
    /// 
    /// [member] XoverCount :: Pre-calculated number of crossovers vs. mutations
    /// 
    /// [member] SampleSet[SampleSetSize] :: Contains an array of Chromosome Indicies and an
    /// array of Bit Indicies
    /// 
    /// [member] IMemory :: Contains and implements a memory scheme and chromosome evaluator
    /// which is indexed by the Engine
    /// 
    /// 
    /// OUTPUT:
    /// 
    /// [member] NextDeviations :: The deviations generated through evolution
    /// 
    /// [return] BestDeviationForGeneration :: The lowest deviation generated. Returned to the
    /// application as an indication of error progression.
    /// 
    /// [member] BestIndexForGeneration :: Index of the best deviation generated. Used for
    /// Elitism NextDeviations[PopulationSize] :: The deviations of each element in the output
    /// (destination) array.
    /// 
    /// </summary>
    inline float_t Evolve()
    {
        Memory.NextGeneration();

        // induction variables (Elitism)
        size_t sampleIndex = 1;
        size_t destIndex = 1;

        // 
        // handle elitism here
        // 
        size_t minIndex = 0;
        float_t min = Memory.Copy( 0, BestIndexForGeneration );
        Bank0.NextDeviations[0] = min;

        // 
        // handle Crossover here
        // 
        while (destIndex < XoverCount)
        {
            float_t dev = Bank0.NextDeviations[destIndex] = Memory.Crossover( destIndex,
                Bank0.ChromoIndicies[sampleIndex],
                BitIndicies[sampleIndex],
                Bank0.ChromoIndicies[sampleIndex + 1],
                BitIndicies[sampleIndex + 1] );

            if (dev < min) { min = dev; minIndex = destIndex; }

            destIndex++;
            sampleIndex += 2;
        }

        // 
        // handle Mutate here
        // 
        while (destIndex < PopulationCount)
        {
            float_t dev = Bank0.NextDeviations[destIndex] = Memory.Mutate( destIndex,
                Bank0.ChromoIndicies[sampleIndex],
                BitIndicies[sampleIndex] );

            if (dev < min) { min = dev; minIndex = destIndex; }

            destIndex++;
            sampleIndex++;
        }

        // 
        // Store scalar results
        // 
        BestIndexForGeneration = minIndex;

        return min;
    }

};
