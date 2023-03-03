#include "CommonCS.hlsl"

ConsumeStructuredBuffer<Particles> simulateOutput : register(u0);
AppendStructuredBuffer<Particles> nextFrameInput : register(u1);
RWStructuredBuffer<VertexData> particlesVertexData : register(u2);
//RWByteAddressBuffer indirectDrawData : register(u3);

uint2 GetAnimatedValuesKeysForNormalizedAge(float normalizedAge, float4 keysArray[8])
{
    uint2 keyIndicies = uint2(0, 0);        //uint2(prevFrameIndex, nextFrameIndex)
    for (uint i = 0; i < 8; i++)
    {
        if (normalizedAge < keysArray[i].w)
        {
            keyIndicies.y = i;
            keyIndicies.x = clamp(i - 1, 0, 8);
            break;
        }
    }

    return keyIndicies;
}

[numthreads(BILLBOARDING_THREAD_COUNT, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x < count)
    {
        Particles particle = simulateOutput.Consume();
    
        float3 particleToCamera = normalize(cameraPos - particle.pos);
        float3 rightVector = cross(cameraUp, particleToCamera);
    
        //float normalizedAge = NormalizedAge(particle);
        //uint2 sizeKeyIndicies = GetAnimatedValuesKeysForNormalizedAge(normalizedAge, sizeOverLifetime); //sizeKeyIndicies.x = prevFrameIndex, sizeKeyIndicies.y = nextFrameIndex
        //float fractionInRange = (normalizedAge - sizeOverLifetime[sizeKeyIndicies.x].w) / (sizeOverLifetime[sizeKeyIndicies.y].w - sizeOverLifetime[sizeKeyIndicies.x].w);
        //float sizeScale = lerp(sizeOverLifetime[sizeKeyIndicies.x].x, sizeOverLifetime[sizeKeyIndicies.y].x, fractionInRange);
    
        VertexData topLeft, botLeft, topRight, botRight;
        float halfSize = particle.size /** sizeScale*/ * 0.5f;
        topLeft.position = particle.pos + cameraUp * halfSize + (-rightVector) * halfSize;
        botLeft.position = particle.pos + (-cameraUp) * halfSize + (-rightVector) * halfSize;
        botRight.position = particle.pos + (-cameraUp) * halfSize + rightVector * halfSize;
        topRight.position = particle.pos + cameraUp * halfSize + rightVector * halfSize;
    
        topLeft.color = particle.color;
        botLeft.color = particle.color;
        botRight.color = particle.color;
        topRight.color = particle.color;
    
        topLeft.uv = float2(0.f, 1.f);
        botLeft.uv = float2(0.f, 0.f);
        botRight.uv = float2(1.f, 0.f);
        topRight.uv = float2(1.f, 1.f);
        
        topLeft.padding1 = 0.f;
        botLeft.padding1 = 0.f;
        botRight.padding1 = 0.f;
        topRight.padding1 = 0.f;
        
        topLeft.padding2 = float2(0.f, 0.f);
        botLeft.padding2 = float2(0.f, 0.f);
        botRight.padding2 = float2(0.f, 0.f);
        topRight.padding2 = float2(0.f, 0.f);
        
        nextFrameInput.Append(particle);
    
        uint baseIndex = DTid.x * 6;
        particlesVertexData[baseIndex] = topLeft;
        particlesVertexData[baseIndex + 1] = botLeft;
        particlesVertexData[baseIndex + 2] = botRight;
        particlesVertexData[baseIndex + 3] = topLeft;
        particlesVertexData[baseIndex + 4] = botRight;
        particlesVertexData[baseIndex + 5] = topRight;
    
    //store num of verts in indirect args buffer
    //uint numVerts = 0;
    //uint stride = 0;
    //nextFrameInput.GetDimensions(numVerts, stride);
    //indirectDrawData.Store(0, numVerts * 6);
    }
}