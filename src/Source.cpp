#include "PreCompileHeader.h"
#include "ProjectHeader.h"

#include "Source.h"


float_t Values[8] = { 0, 1, 2, 100, 1000 };
float_t Probabilities[8] = { -1.0f, .1f, .1f, .002f, .00045f, 0, 0, 0 };

float_t Confidence = 0.95f;
float_t ZScoreForConfidence = 1.95996f;

float_t ConfidenceIntervals[] = { .80f, .90f, .95f, .98f, .99f, .999f };
float_t ZScores[] = { 1.281552f, 1.644854f, 1.95996f, 2.32635f, 2.57583f, 3.2905f };


float_t Setup()
{
    size_t count = 0, autosumIdx = -1;
    float_t sumPV = 0, sumP = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (Probabilities[i] > 0)
        {
            count++;
            sumP += Probabilities[i];
            sumPV += Probabilities[i] * Values[i];
        }
        else if (Probabilities[i] < 0)
            autosumIdx = i; // multiples silently fail
    }

    if (autosumIdx >= 0) // sumP dependent on ENTIRE previous loop
    {
        auto autoSum = 1.0f - sumP;
        Probabilities[autosumIdx] = autoSum;

        count++;
        sumP += autoSum;
        sumPV += Probabilities[autosumIdx] * Values[autosumIdx];
    }

    float_t mean = sumPV / count;
    float_t stdev = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (Probabilities[i] > 0)
        {
            auto val = Probabilities[i] * Values[i];
            auto diff = val - mean;
            stdev += diff * diff;
        }
    }
    stdev = sqrtf( stdev / (count - 1) );
    return stdev;
}

float_v GetProbabilitiesAsKeys()
{
    float_v probKeys;
    float_p _pk = (float_p) &probKeys;

    _pk[0] = Probabilities[0];
    for (size_t i = 1; i < 8; i++)
        _pk[i] = _pk[i - 1] + Probabilities[i];

    return probKeys;
}






int main( int _ac, char** av )
{
    uint_t counts[8] = { 0,0,0,0,0,0,0,0 };
    XorShift rng;

    auto stdev = Setup();

    float_v probKeys = GetProbabilitiesAsKeys();
    float_v vRandomNumbers;
    float_p randomNumbers = (float_p) &vRandomNumbers;

    for (size_t i = 0; i < 1000000; i++)
    {
        rng.Next( &vRandomNumbers );
        for (size_t j = 0; j < 8; j++)
        {
            auto sample = randomNumbers[j];
            auto index = GetIndex( sample, probKeys );
            counts[index]++;
        }
    }

    uint_t sumCounts = 0;
    for (size_t i = 0; i < 8; i++)
        sumCounts += counts[i];

    double sum = 0;
    for (size_t i = 0; i < 8; i++)
    {
        double x = (double) Values[i] * counts[i];
        sum += x;

        double percent = 100.0 * counts[i] / (double) sumCounts;
        printf( "(%lf%%)   %lf x %d = %lf\n", percent, Values[i], counts[i], x );
    }

    printf( "\nCounts = %d\nTotal = %lf\n\n", sumCounts, sum / sumCounts );

    printf( "stdev: %f\n\n", stdev );
    printf( "< Press Any Key >" );
    _getch();
    return 42;
}

