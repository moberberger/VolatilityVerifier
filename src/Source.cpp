#include "PreCompileHeader.h"
#include "ProjectHeader.h"

#include "Source.h"


size_t PopulationSize = 1000000;
size_t iterations = 10000;

float_t Values[8] = { 0, 1, 2, 10, 100, 1000, 0, 0 };
float_t Probabilities[8] = { 0.7889999f, .10f, .10f, .01f, .001f, .0000001f, 0, 0 };

//float_t Values[8] = { 0, 1, 5, 10, 25 };
//float_t Probabilities[8] = { 0.15f, .40f, .20f, .15f, .10f, 0, 0, 0 };

float_t Confidence = 0.95f;
float_t ZScoreForConfidence = 1.95996f;

float_t ConfidenceIntervals[] = { .80f, .90f, .95f, .98f, .99f, .999f };
float_t ZScores[] = { 1.281552f, 1.644854f, 1.95996f, 2.32635f, 2.57583f, 3.2905f };

float_t TheoreticalValue;
float_t Stdev;

void Setup()
{
    size_t count = 0;
    float_t sumV = 0, sumP = 0;
    TheoreticalValue = 0;

    for (size_t i = 0; i < 8; i++)
    {
        if (Probabilities[i] > 0)
        {
            count++;
            sumP += Probabilities[i];
            sumV += Values[i];
            TheoreticalValue += Probabilities[i] * Values[i];
        }
    }

    float_t mean = sumV / count;
    for (size_t i = 0; i < 8; i++)
    {
        if (Probabilities[i] > 0)
        {
            auto delta = Values[i] - mean;
            delta *= delta;
            auto val = Probabilities[i] * delta;
            Stdev += val;
        }
    }
    Stdev = sqrtf( Stdev );
    printf( "stdev: %f\n\n", Stdev );
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



float_t GetRtp( float_v& keys )
{
    XorShift rng;
    float_t randomNumbers[8];

    float_t counts[8] = { 0,0,0,0,0,0,0,0 };

    for (size_t i = 0; i < PopulationSize / 8; i++)
    {
        rng.Next( randomNumbers );
        for (size_t j = 0; j < 8; j++)
        {
            auto sample = randomNumbers[j];
            auto index = GetIndex( sample, keys );
            counts[index]++;
        }
    }

    float_v vValues = *(float_vp) Values;
    float_v vCounts = *(float_vp) counts;
    float_v product = vCounts * vValues;
    float_t dotP = horizontal_add( product );
    float_t sumCounts = horizontal_add( vCounts );
    float_t rtp = dotP / sumCounts;

    return rtp;
}





int main( int _ac, char** av )
{
    Setup();

    float_t volIdx = ZScoreForConfidence * Stdev / sqrtf( (float) PopulationSize );
    printf( "Volatility Index VI = %f\n", volIdx );

    volIdx *= 0.01;
    printf( "        Modified VI = %f\n", volIdx );

    float_t mean = TheoreticalValue;
    printf( "Theoretical Mean = %f\n", mean );

    float_v probKeys = GetProbabilitiesAsKeys();

    int count = 0;
    int countBad = 0;

    for (size_t i = 0; i < 10000; i++)
    {
        float_t rtp = GetRtp( probKeys );
        count++;
        if (rtp < mean - volIdx || rtp > mean + volIdx)
            countBad++;
    }

    printf( "Count    : %d\n", count );
    printf( "Count Bad: %d\n", countBad );

    // double sum = 0; for (size_t i = 0; i 8; i++) { double x = (double) Values[i] * counts[i];
    // sum += x;

    // double percent = 100.0 * counts[i] / (double) sumCounts; printf( "(%lf%%) %lf x %d =
    // %lf\n", percent, Values[i], counts[i], x ); }

    // printf( "\nCounts = %d\nTotal = %lf\n\n", sumCounts, sum / sumCounts );

    printf( "< Press Any Key >" );
    _getch();
    return 42;
}

