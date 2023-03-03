#include "CommonCS.hlsl"

//RWByteAddressBuffer countBuffer : register(u0);
RWStructuredBuffer<CountBuffer> count : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    //afterEmitAliveCount = countBuffer.Load(COUNT_BUFFER_AFTER_EMIT_ALIVE_COUNT_OFFSET);
    //afterSimDeadCount = countBuffer.Load(COUNT_BUFFER_AFTER_SIM_DEAD_COUNT_OFFSET);
    //countBuffer.Store(COUNT_BUFFER_ALIVE_COUNT_OFFSET, afterEmitAliveCount - afterSimDeadCount);
    //countBuffer.Store(COUNT_BUFFER_AFTER_EMIT_ALIVE_COUNT_OFFSET, afterEmitAliveCount - afterSimDeadCount);
    //countBuffer.Store(COUNT_BUFFER_AFTER_SIM_DEAD_COUNT_OFFSET, 0);
    //count[0].afterEmitParticleCount = count[0].afterSimParticleCount;
    //count[0].afterSimParticleCount = 0;
    //count[0].deadParticleCount = 0;
}