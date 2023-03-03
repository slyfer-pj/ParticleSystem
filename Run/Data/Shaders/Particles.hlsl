#include "CommonCS.hlsl"

struct vs_input_t
{
    uint vertexID : SV_VertexID;
};

struct v2p_t
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer CameraConstants : register(b2)
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
}

cbuffer ModelConstants : register(b3)
{
    float4x4 modelMatrix;
    float4 modelColor;
}

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);
StructuredBuffer<Particles> particleData : register(t1);
StructuredBuffer<float2> particleIndexList : register(t2);  //the 2nd float has the indices, the first one has the sq dist from cam which was used in sorting

static float camUpSign[6] = { 1.f, -1.f, -1.f, 1.f, -1.f, 1.f };
static float rightVectorSign[6] = { -1.f, -1.f, 1.f, -1.f, 1.f, 1.f };
static float2 uvs[6] = { float2(0.f, 1.f), float2(0.f, 0.f), float2(1.f, 0.f), float2(0.f, 1.f), float2(1.f, 0.f), float2(1.f, 1.f) };

float3 GetBillboardedVertex(uint vertexNum, Particles particle)
{
    RNG rng;
    float normalizedAge = NormalizedAge(particle);
    float2 keysArray[8];
    [unroll]
    for (int i = 0; i < 8; i++)
    {
        keysArray[i] = velOverLifeZSizeOverLifeX[i].zw;
    }
    float sizeScaleX = GetInterpolatedValueFromAnimFrames(normalizedAge, keysArray, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_SIZE_OVERLIFE_X, rng, particle.particleId);
    
    [unroll]
    for (int j = 0; j < 8; j++)
    {
        keysArray[j] = colorOverLifeAlphaSizeOverLifeY[j].zw;
    }
    float sizeScaleY = GetInterpolatedValueFromAnimFrames(normalizedAge, keysArray, curveModeBitFlags, ANIM_CURVE_BIT_FLAG_SIZE_OVERLIFE_Y, rng, particle.particleId);
    
    float3 upVec, rightVec, forwardVec;
    if (renderMode == 0)
    {
        float4 particleWorldPos = mul(modelMatrix, float4(particle.pos, 1.f));
        forwardVec = normalize(cameraPos.xyz - particleWorldPos.xyz);
        rightVec = cross(cameraUp.xyz, forwardVec);
        upVec = cameraUp;
    }
    else
    {
        forwardVec = float3(0.f, 0.f, 1.f); //world Z
        upVec = float3(0.f, 1.f, 0.f); //world Y
        rightVec = cross(upVec, forwardVec);
    }
    float halfSizeX = particle.size * sizeScaleX * 0.5f;
    float halfSizeY = particle.size * sizeScaleY * 0.5f;
    
    float4x4 rotationMatrix = GetRotationMatrixAroundAnAxis(forwardVec, particle.rot);
    upVec = mul(rotationMatrix, float4(upVec.xyz, 0.f));
    rightVec = mul(rotationMatrix, float4(rightVec.xyz, 0.f));
    
    float3 randAxis = float3(gpuOrbitAxis.y, gpuOrbitAxis.z, gpuOrbitAxis.x);
    float3 radialVector = cross(gpuOrbitAxis.xyz, randAxis);
    float4x4 rotMat = GetRotationMatrixAroundAnAxis(gpuOrbitAxis.xyz, particle.orbitalAngle);
    radialVector = mul(rotMat, float4(radialVector.xyz, 0.f));
    
    float3 vert = particle.pos + (radialVector * particle.orbitalRadius) + (upVec * halfSizeY * camUpSign[vertexNum]) + (rightVec * halfSizeX * rightVectorSign[vertexNum]);
    
    return vert;
}

float2 GetParticleUVs(Particles particle, int vertexNum)
{
    int numFrames = sheetGridLayout.x * sheetGridLayout.y;
    int currentSpriteIndex = floor(NormalizedAge(particle) * numFrames);
    int xCoord = currentSpriteIndex % sheetGridLayout.x;
    int yCoord = (sheetGridLayout.y - 1) - (currentSpriteIndex / sheetGridLayout.x);    //since sprite sheets are usually ordered from top left, so "invert" the y coord
    uint textureWidth = 0;
    uint textureHeight = 0;
    diffuseTexture.GetDimensions(textureWidth, textureHeight);
    float2 spriteSize = float2(textureWidth / sheetGridLayout.x, textureHeight / sheetGridLayout.y);
    float2 spriteUVOrigin = float2(xCoord * spriteSize.x, yCoord * spriteSize.y);
    
    float2 absoluteVertUV = spriteUVOrigin + (uvs[vertexNum] * spriteSize);
    float normUVx = RangeMap(absoluteVertUV.x, 0.f, textureWidth, 0.f, 1.f);
    float normUVy = RangeMap(absoluteVertUV.y, 0.f, textureHeight, 0.f, 1.f);
    return float2(normUVx, normUVy);
}

v2p_t VertexMain(vs_input_t input)
{    
    uint vertexNum = input.vertexID % 6;
    uint particleIndex = particleIndexList[input.vertexID / 6].y;
    Particles particle = particleData[particleIndex];
    float3 billboardedVert = GetBillboardedVertex(vertexNum, particle);
    
    if (particle.lifetime < 0.f)
    {
        v2p_t v2p;
        v2p.position = float4(0.f, 0.f, 0.f, 1.f);
        v2p.color = float4(0.f, 0.f, 0.f, 0.f);
        v2p.uv = float2(0.f, 0.f);
        return v2p;
    }
    
    float4 localPosition = float4(billboardedVert.xyz, 1.f);
    float4 worldSpacePos = mul(modelMatrix, localPosition);
    float4 viewSpacePos = mul(viewMatrix, worldSpacePos);
    float4 ndcPos = mul(projectionMatrix, viewSpacePos);
    v2p_t v2p;
    v2p.position = ndcPos;
    v2p.color = particle.color;
    v2p.uv = GetParticleUVs(particle, vertexNum);
    //v2p.uv = uvs[vertexNum];
    //v2p.uv = float2(0.f, 0.f);
    return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
    float4 tint = input.color * modelColor;
    float4 output = float4(diffuseTexture.Sample(diffuseSampler, input.uv) * tint);
    //clip(output.a - 0.2f);
    return output;
}