#include "CommonCS.hlsl"

StructuredBuffer<Particles> currentParticleList : register(t0);
RWStructuredBuffer<Particles> sortedParticleList : register(u0);

cbuffer SortConstants : register(b9)
{
    float3 camPos;
    uint iterationNum;
    uint numParticles;
}

Particles GetFartherParticleFromCamera(Particles a, Particles b)
{
    float aDistSquared = dot(a.pos - camPos, a.pos - camPos);
    float bDistSquared = dot(b.pos - camPos, b.pos - camPos);
    if (aDistSquared > bDistSquared)
        return a;
    else
        return b;
}

Particles GetCloserParticleFromCamera(Particles a, Particles b)
{
    float aDistSquared = dot(a.pos - camPos, a.pos - camPos);
    float bDistSquared = dot(b.pos - camPos, b.pos - camPos);
    
    if (aDistSquared < bDistSquared)
        return a;
    else
        return b;
}

[numthreads(SORT_THREAD_COUNT, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    int index = asint(DTid.x);
    if (index < numParticles)
    {
        if (iterationNum % 2 == 0)
        {
            if (index % 2 == 0)
            {
            //need to clamp the index
                sortedParticleList[index] = GetFartherParticleFromCamera(currentParticleList[index], currentParticleList[clamp(index + 1, 0, asint(numParticles - 1))]);
            }
            else
            {
                sortedParticleList[index] = GetCloserParticleFromCamera(currentParticleList[index], currentParticleList[clamp(index - 1, 0, asint(numParticles - 1))]);
            }
        }
        else
        {
            if (index % 2 == 0)
            {
                sortedParticleList[index] = GetCloserParticleFromCamera(currentParticleList[index], currentParticleList[clamp(index - 1, 0, asint(numParticles - 1))]);
            }
            else
            {
                sortedParticleList[index] = GetFartherParticleFromCamera(currentParticleList[index], currentParticleList[clamp(index + 1, 0, asint(numParticles - 1))]);
            }
        }
    }
}
