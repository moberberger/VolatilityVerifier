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

inline static Vec8f ToScaledFloat( Vec8i _32bits, Vec8f scaleFull, Vec8f scaleHalf )
{
    const Vec8i float_mantissaMask = 0x007fffff;
    const Vec8i float_exponentMask = 0x40000000;

    Vec8i tmp = _32bits & float_mantissaMask | float_exponentMask;
    return _mm256_fmsub_ps( reinterpret_f( tmp ), scaleHalf, scaleFull );
}
