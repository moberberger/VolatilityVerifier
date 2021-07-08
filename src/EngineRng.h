#pragma once

/*

D:\Git\ProbGenAvx > ent.exe 8bit.bin
Entropy = 7.999999 bits per byte.

Optimum compression would reduce the size
of this 249250997 byte file by 0 percent.

Chi square distribution for 249250997 samples is 259.73, and randomly
would exceed this value 40.60 percent of the times.

Arithmetic mean value of data bytes is 127.4984 (127.5 = random).
Monte Carlo value for Pi is 3.141860186 (error 0.01 percent).
Serial correlation coefficient is - 0.000056 (totally uncorrelated = 0.0).




D:\Git\ProbGenAvx > ent.exe 32bit.bin
Entropy = 8.000000 bits per byte.

Optimum compression would reduce the size
of this 997003988 byte file by 0 percent.

Chi square distribution for 997003988 samples is 242.55, and randomly
would exceed this value 70.23 percent of the times.

Arithmetic mean value of data bytes is 127.4987 (127.5 = random).
Monte Carlo value for Pi is 3.141732306 (error 0.00 percent).
Serial correlation coefficient is 0.000013 (totally uncorrelated = 0.0).

*/


class EngineRng
{
    uint_v m_state0, m_state1, m_state2, m_state3;

public:
    EngineRng()
    {
        SlowRng( m_state0 );
        SlowRng( m_state1 );
        SlowRng( m_state2 );
        SlowRng( m_state3 );
    }

    inline void CreateSamples( float_v scale, size_t sampleSetCount, void* _uints, void* _floats )
    {
        const float_v scaleHalf = scale / 2;
        auto uints = (uint_v*) _uints;
        auto floats = (float_v*) _floats;

        uint_v state0 = m_state0;
        uint_v state1 = m_state1;
        uint_v state2 = m_state2;
        uint_v state3 = m_state3;

        auto ssVectorCount = sampleSetCount / VecElementCount;
#define stride 8 // build this many pairs per iteration

        for (size_t i = 0; i < ssVectorCount; i += stride)
        {
            state0 ^= state0 << 13;
            state0 ^= state0 >> 17;
            state0 ^= state0 << 5;

            state1 ^= state1 << 13;
            state1 ^= state1 >> 17;
            state1 ^= state1 << 5;
            auto x01 = state0 ^ state1;

            state2 ^= state2 << 13;
            state2 ^= state2 >> 17;
            state2 ^= state2 << 5;
            auto x02 = state0 ^ state2;
            auto x12 = state1 ^ state2;

            state3 ^= state3 << 13;
            state3 ^= state3 >> 17;
            state3 ^= state3 << 5;
            auto x03 = state0 ^ state3;
            auto x13 = state1 ^ state3;
            auto x23 = state2 ^ state3;

            auto x012 = x01 ^ state2;
            auto x013 = x01 ^ state3;
            auto x023 = x02 ^ state3;
            auto x123 = x12 ^ state3;
            auto x0123 = x012 ^ state3;

            // somewhat arbitrary- took the only freely available source of variable information
            // from generation to generation. Changing this from a scaled-float to a uint output 
            // does not seem to alter the randomness tests.
            auto x01234 = x0123 ^ reinterpret_i( scale );

            uints[i + 0] = state0;
            uints[i + 1] = state1;
            uints[i + 2] = state2;
            uints[i + 3] = state3;
            uints[i + 4] = x01;
            uints[i + 5] = x02;
            uints[i + 6] = x03;
            uints[i + 7] = x0123;

            floats[i + 0] = ToScaledFloat( x12, scale, scaleHalf );
            floats[i + 1] = ToScaledFloat( x13, scale, scaleHalf );
            floats[i + 2] = ToScaledFloat( x23, scale, scaleHalf );
            floats[i + 3] = ToScaledFloat( x012, scale, scaleHalf );
            floats[i + 4] = ToScaledFloat( x013, scale, scaleHalf );
            floats[i + 5] = ToScaledFloat( x023, scale, scaleHalf );
            floats[i + 6] = ToScaledFloat( x123, scale, scaleHalf );
            floats[i + 7] = ToScaledFloat( x01234, scale, scaleHalf );
        }

        m_state0 = state0;
        m_state1 = state1;
        m_state2 = state2;
        m_state3 = state3;
    }
};
