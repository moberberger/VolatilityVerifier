#pragma once

#include "Engine.h"


class ProbabilityGenerator
{
    double targetValue;
    size_t count;
    double* values;



public:
    ProbabilityGenerator(double _targetValue, size_t _valueCount, double _values[])
    {
        targetValue = _targetValue;
        count = _valueCount;
        values = _values;

    }



};

