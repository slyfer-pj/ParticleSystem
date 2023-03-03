#include "CommonCS.hlsl"

//AppendStructuredBuffer<Particles> currentParticlesList : register(u0);
RWStructuredBuffer<Particles> particleData : register(u0);
ConsumeStructuredBuffer<uint> deadParticlesIndexList : register(u1);
//RWStructuredBuffer<uint> afterSpawnParticleList : register(u2);
//RWByteAddressBuffer countBuffer : register(u3);
RWStructuredBuffer<CountBuffer> count : register(u2);

static float3 Rand_Dir[6] =
{
    float3(1.0f, 0.0f, 0.0f),
   float3(-1.0f, 0.0f, 0.0f),
   float3(0.0f, 1.0f, 0.0f),
   float3(0.0f, -1.0f, 0.0f),
   float3(0.0f, 0.0f, 1.0f),
   float3(0.0f, 0.0f, -1.0f),
};
static float3 CONE_DIR = float3(0.0001f, 0.f, 1.f); //setting x=0.f, give undefined behaviour for atan2, hence giving it some minimal value

[numthreads(SPAWN_THREAD_COUNT, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{    
    if (DTid.x < emitCount)
    {
        Particles newParticle;
    
        RNG rng;
        rng.s = rng.pcg_hash(DTid.x + randInt);
        float3 velDir = float3(0.f, 0.f, 0.f);
        float3 startPos = float3(0.f, 0.f, 0.f);
        if (shapeType == 0)     //Cone
        {
            velDir = rng.GetRandomDirectionInCone(coneForward, coneHalfAngle);
        }
        else if (shapeType == 1)    //Sphere
        {
            velDir = rng.GetRandomDirectionInSphere();
            startPos = (sphereEmitFrom == 0 ? rng.GetRandomFloatInRange(0.f, sphereRadius) : sphereRadius) * velDir;
        }
        else if (shapeType == 2)    //Box
        {
            velDir = boxForward;
            float3 halfDimensions = boxDimensions * 0.5f;
            startPos.x = lerp(-halfDimensions.x, halfDimensions.x, rng.GetRandomFloatZeroToOne());
            startPos.y = lerp(-halfDimensions.y, halfDimensions.y, rng.GetRandomFloatZeroToOne());
            startPos.z = -halfDimensions.z;
        }

        newParticle.pos = offsetFromBase + startPos;
        if (simSpace == 1)
        {
            newParticle.pos += worldPos;
        }
        newParticle.vel = rng.GetRandomFloatInRange(startSpeedRange.x, startSpeedRange.y) * velDir;
        newParticle.age = 0.f;
        newParticle.color = startColor;
        newParticle.rot = rng.GetRandomFloatInRange(startRotRange.x, startRotRange.y);
        newParticle.size = rng.GetRandomFloatInRange(startSizeRange.x, startSizeRange.y);
        newParticle.lifetime = rng.GetRandomFloatInRange(lifetimeRange.x, lifetimeRange.y);
        newParticle.orbitalAngle = rng.GetRandomFloatInRange(0.f, 360.f);
        //float3 distanceVector = newParticle.pos - offsetFromBase;
        //float xyLength = length(float2(distanceVector.xy));
        //newParticle.orbitalRadius = xyLength;
        newParticle.orbitalRadius = 0.f;
        newParticle.particleId = randInt + DTid.x;
        uint particleIndex;
        
        //the first n threads always spawn using n indices from the dead list (if list size is 0, then they don't use dead particle indices)
        if (DTid.x < count[0].deadListSize)
        {
            particleIndex = deadParticlesIndexList.Consume();
            InterlockedAdd(count[0].deadListSize, -1);
            particleData[particleIndex] = newParticle;
        }
        //no need to check for last alive particle index as this shader is guaranteed to only spawn as many particles as there are slots
        else /*(count[0].lastAliveParticleIndex < count[0].maxParticles)*/
        {
            uint lastParticleIndex;
            InterlockedAdd(count[0].lastAliveParticleIndex, 1, lastParticleIndex);
            particleIndex = lastParticleIndex;
            InterlockedAdd(count[0].listSize, 1);
            particleData[particleIndex] = newParticle;
        }
    }
}