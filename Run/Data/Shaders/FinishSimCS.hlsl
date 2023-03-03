#include "CommonCS.hlsl"

ConsumeStructuredBuffer<Particles> currentParticleList : register(u0);
AppendStructuredBuffer<Particles> nextFrameList : register(u1);

[numthreads(FINISH_SIM_THREAD_COUNT, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < count)
    {
        Particles particle = currentParticleList.Consume();
        nextFrameList.Append(particle);
    }
}