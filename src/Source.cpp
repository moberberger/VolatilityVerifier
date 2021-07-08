#include "PreCompileHeader.h"
#include "ProjectHeader.h"

#include "Source.h"


float_t Values[8] = { 0, 1, 2, 100, 1000 };
float_t Probabilities[8] = { -1.0f, .1f, .1f, .002f, .00045f, 0, 0 };
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
        if (Probabilities[i] < 0)
            autosumIdx = i;
    }

    if (autosumIdx >= 0)
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
    stdev = sqrtf( stdev );
    return stdev;
}

int main( int _ac, char** av )
{
    float_v* vals = (float_v*) Values;
    float_v* probs = (float_v*) Probabilities;

    _getch();
    return 42;
}

