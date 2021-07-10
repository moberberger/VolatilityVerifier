#pragma once

template<class T>
inline static void SlowRng( T* buffer, size_t length )
{
    uint32_t* ptr = (uint32_t*) buffer;
    for (size_t i = 0; i < length; i += 4)
        _rdseed32_step( ptr++ );
}


template<class T>
inline static void SlowRng( T& buffer )
{
    SlowRng( &buffer, sizeof( T ) );
}


template<class T>
inline static void SlowRng( T buffer[] )
{
    SlowRng( buffer, sizeof( T ) );
}

template<class T>
inline static void ClearMem( T& obj )
{
    memset( &obj, 0, sizeof( T ) );
}


inline static float_v ToUnscaledFloat( uint_v as32bits )
{
    const uint_v float_mantissaMask = 0x007fffff;
    const uint_v float_exponentMask = 0x40000000;

    uint_v tmp = as32bits & float_mantissaMask | float_exponentMask;
    return reinterpret_f( tmp );
}

inline static float_v ToScaledFloat( uint_v _32bits, float_v scaleHalf, float_v scaleFull )
{
    auto asFloats = ToUnscaledFloat( _32bits );
    return _mm256_fmsub_ps( asFloats, scaleHalf, scaleFull );
}

inline static uint_t GetIndex( float_v sample, float_v keys )
{
    auto mask = _mm256_cmp_ps( keys, sample, _CMP_LE_OS );
    uint_t bits = _mm256_movemask_ps( mask );
    bits = ~bits;
    return _tzcnt_u32( bits );
}

class XorShift
{
    uint_v state;
public:
    XorShift() { SlowRng( state ); }

    void Next( float_p dest )
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;

        *(float_vp) dest = ToScaledFloat( state, 0.5f, 1.0f );
    }
};