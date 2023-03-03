#include "CommonCS.hlsl"

//ConsumeStructuredBuffer<Particles> currentParticlesList : register(u0);
//AppendStructuredBuffer<Particles> newParticlesList : register(u1);
RWStructuredBuffer<Particles> particleDataList : register(u0);
//RWStructuredBuffer<uint> currentAliveParticleIndexList : register(u1);
//RWStructuredBuffer<uint> afterSimParticleIndexList : register(u2);
AppendStructuredBuffer<uint> deadParticleIndexList : register(u1);
//RWByteAddressBuffer countBuffer : register(u4);
RWStructuredBuffer<CountBuffer> count : register(u2);
RWStructuredBuffer<SortData> sortData : register(u3);

float3 GetVelocityForCurrentAge(float normalizedAge, RNG rng, uint particleId)
{
    uint prevFrame, nextFrame;
    float2 xVelKeys[8];
    for (int i = 0; i < 8; i++)
    {
        xVelKeys[i] = velOverLifeXY[i].xy;
    }
    float xVel = GetInterpolatedValueFromAnimFrames(normalizedAge, xVelKeys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_X, rng, particleId);
    
    float2 yVelKeys[8];
    for (int j = 0; j < 8; j++)
    {
        yVelKeys[j] = velOverLifeXY[j].zw; //y vel over lifetime is store in the last 2 floats of the float4(velOverLifeXY)
    }
    float yVel = GetInterpolatedValueFromAnimFrames(normalizedAge, yVelKeys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_Y, rng, particleId);
    
    float2 zVelKeys[8];
    for (int k = 0; k < 8; k++)
    {
        zVelKeys[k] = velOverLifeZSizeOverLifeX[k].xy; //z vel over lifetime is store in the first 2 floats of the float4(velOverLifeZSizeOverLife)
    }
    float zVel = GetInterpolatedValueFromAnimFrames(normalizedAge, zVelKeys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_Z, rng, particleId);

    return float3(xVel, yVel, zVel);
}

float4 GetColorForCurrentAge(float normalizedAge)
{
    uint prevFrame, nextFrame;
    float colorKeys[8];
    for (int i = 0; i < 8; i++)
    {
        colorKeys[i] = colorOverLifeRGB[i].w;
    }
    uint2 keyFrames = GetAnimatedValuesKeysForNormalizedAge(normalizedAge, colorKeys);
    prevFrame = keyFrames.x;
    nextFrame = keyFrames.y;
    float fractionInRange = GetFractionWithin(normalizedAge, colorKeys[prevFrame], colorKeys[nextFrame]);
    float3 rgbValue = lerp(colorOverLifeRGB[prevFrame].rgb, colorOverLifeRGB[nextFrame].rgb, fractionInRange);
    float alphaValue = lerp(colorOverLifeAlphaSizeOverLifeY[prevFrame].x, colorOverLifeAlphaSizeOverLifeY[nextFrame].x, fractionInRange);
    
    return float4(rgbValue.rgb, alphaValue);
}

float3 GetVelocityToAttractor(float normalizedAge, float3 particlePos)
{
    float3 resultantVelocity;
    for (int i = 0; i < 8; i++)
    {
        float3 particleToAttractor = pointAttractorOffsetAndStrength[i].xyz - particlePos;
        float distanceToAttractor = clamp(dot(particleToAttractor, particleToAttractor), 0.00001f, 100000000000.f); //upper limit is magic number
        float strengthBasedOnDistance = pointAttractorOffsetAndStrength[i].w / distanceToAttractor;
        resultantVelocity += (strengthBasedOnDistance * particleToAttractor);
    }
    return resultantVelocity;
}

float GetDragForCurrentAge(float normalizedAge, RNG rng, uint particleId)
{
    float2 dragKeys[8];
    for (int i = 0; i < 8; i++)
    {
        dragKeys[i] = dragOverLifetimeRotationOverLifetime[i].xy;
    }
    
    return GetInterpolatedValueFromAnimFrames(normalizedAge, dragKeys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_DRAG_OVERLIFE, rng, particleId);    
}

float GetAngularVelocityForCurrentAge(float normalizedAge, RNG rng, uint particleId)
{
    uint prevFrame, nextFrame;
    float2 rotKeys[8];
    for (int i = 0; i < 8; i++)
    {
        rotKeys[i] = dragOverLifetimeRotationOverLifetime[i].zw;
    }
    float angularVel = GetInterpolatedValueFromAnimFrames(normalizedAge, rotKeys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_ROT_OVERLIFE, rng, particleId);
    
    return angularVel;
}

float GetOrbitalAngleFromVelocityForCurrentAge(float normalizedAge, RNG rng, uint particleId)
{
    uint prevFrame, nextFrame;
    float2 keys[8];
    for (int i = 0; i < 8; i++)
    {
        keys[i] = orbitalVelAndRadiusOverLifetime[i].xy;
    }
    float orbitalAngle = GetInterpolatedValueFromAnimFrames(normalizedAge, keys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_ORBIT_VEL_OVERLIFE, rng, particleId);
    
    return orbitalAngle;
}

float GetOrbitalRadiusForCurrentAge(float normalizedAge, RNG rng, uint particleId)
{
    uint prevFrame, nextFrame;
    float2 keys[8];
    for (int i = 0; i < 8; i++)
    {
        keys[i] = orbitalVelAndRadiusOverLifetime[i].zw;
    }
    float orbitalRadius = GetInterpolatedValueFromAnimFrames(normalizedAge, keys, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_ORBIT_RADIUS_OVERLIFE, rng, particleId);
    
    return orbitalRadius;
}

[numthreads(SIMULATION_THREAD_COUNT, 1, 1)]
void CSMain(uint3 DTid: SV_DispatchThreadID)
{  
    //afterEmitAliveCount = countBuffer.Load(COUNT_BUFFER_AFTER_EMIT_ALIVE_COUNT_OFFSET);
    uint listSize = count[0].listSize;
    
    if (DTid.x < listSize)
    {
        RNG rng;
        //rng.s = rng.pcg_hash(DTid.x + randInt);
        uint particleIndex = DTid.x;
        Particles particleIn = particleDataList[particleIndex];
    
        Particles particleOut;
        float ageOut = particleIn.age + deltaSeconds;
        if (ageOut < particleIn.lifetime)
        {
            particleOut.age = particleIn.age + deltaSeconds;
            particleOut.size = particleIn.size;
            particleOut.lifetime = particleIn.lifetime;
            float normalizedAge = NormalizedAge(particleOut);
            float3 gravitationalForce = (gravity * gravityScale);
            float3 dragForce = GetDragForCurrentAge(normalizedAge, rng, particleIn.particleId) * -1.f * particleIn.vel;
            float3 acceleration = gravitationalForce + dragForce + GetVelocityForCurrentAge(normalizedAge, rng, particleIn.particleId) + GetVelocityToAttractor(normalizedAge, particleIn.pos);
            particleOut.rot = particleIn.rot + (GetAngularVelocityForCurrentAge(normalizedAge, rng, particleIn.particleId) * deltaSeconds);
            particleOut.vel = particleIn.vel + (acceleration * deltaSeconds);
            particleOut.pos = particleIn.pos + (particleOut.vel * deltaSeconds);
            particleOut.color = GetColorForCurrentAge(normalizedAge);
            particleOut.orbitalAngle = particleIn.orbitalAngle + (GetOrbitalAngleFromVelocityForCurrentAge(normalizedAge, rng, particleIn.particleId) * deltaSeconds);
            particleOut.orbitalRadius = particleIn.orbitalRadius + (GetOrbitalRadiusForCurrentAge(normalizedAge, rng, particleIn.particleId) * deltaSeconds);
            particleOut.particleId = particleIn.particleId;
            particleDataList[particleIndex] = particleOut;
            
            
            
            //uint aliveCount;
            //InterlockedAdd(count[0].afterSimParticleCount, 1, aliveCount);
            //afterSimParticleIndexList[DTid.x] = particleIndex;
        }
        else if (particleIn.lifetime > 0.f)
        {
            particleIn.lifetime = -1.f;
            particleDataList[particleIndex] = particleIn;
            deadParticleIndexList.Append(particleIndex);
            //countBuffer.InterlockedAdd(COUNT_BUFFER_AFTER_SIM_DEAD_COUNT_OFFSET, 1, deadCount);
            InterlockedAdd(count[0].deadListSize, 1);
        }
        
        
        float3 distanceVectorToCamera = particleOut.pos - cameraPos.xyz;
        sortData[particleIndex].distanceSq = dot(distanceVectorToCamera, distanceVectorToCamera);
        sortData[particleIndex].index = particleIndex;
        
    }
}