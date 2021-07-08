#pragma once

#define ROUND_UP(x,block_size) (BLOCK_COUNT( x, block_size ) * block_size)
#define BLOCK_COUNT(x,block_size) (((x) + ((block_size)-1)) / (block_size))


typedef float float_t;
typedef unsigned int uint_t;

typedef Vec8f float_v;
typedef Vec8ui uint_v;



#define MaxPopCount 512 // set because of Keys algorithm
#define VecElementByteCount sizeof( float_t )
#define VecSize sizeof( float_v )
#define VecElementCount (VecSize / VecElementByteCount)
#define MaxPopVectorCount (MaxPopCount / VecElementCount)

#include "ProjectLib.h"

#include "EngineRng.h"
#include "EngineSampler.h"
#include "Timer.h"
