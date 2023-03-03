struct Particles
{
    float3 pos;
    float size;
    float3 vel;
    float rot;
    float4 color;
    float lifetime;
    float age;
    float orbitalAngle;
    float orbitalRadius;
    uint particleId;
};

struct IndirectDrawArgs
{
    uint vertexPerInstance;
    uint numInstances;
    uint startVertexLocation;
    uint startInstanceLocation;
};

struct CountBuffer
{
    uint maxParticles;
    uint listSize;
    uint deadListSize;
    uint lastAliveParticleIndex;
    //uint afterEmitParticleCount;
    //uint afterSimParticleCount;
    //uint lastAliveParticleIndex;
    //uint deadParticleCount;
};

struct SortData
{
    float distanceSq;
    float index;
};

cbuffer SimulationConstants : register(b8)
{
    float3 gravity;
    float1 deltaSeconds;
    float4 startColor;
    int gravityScale;
    uint randInt;
    int2 sheetGridLayout;
    float4 pointAttractorOffsetAndStrength[8]; //float4(offset.x, offset.y, offset.z, strength)
    float4 gpuOrbitAxis;
}

cbuffer AnimatedValuesConstants : register(b7)
{
    float4 velOverLifeXY[8];                            //float4(velX value, velX time, velY value, velY time)
    float4 velOverLifeZSizeOverLifeX[8];                //float4(velZ value, velZ time, sizeX value, sizeX time)
    float4 colorOverLifeRGB[8];                         //float4(r value, g value, b value, time)
    float4 colorOverLifeAlphaSizeOverLifeY[8];          //float4(alpha , time, sizeY value, sizeY time)
    float4 dragOverLifetimeRotationOverLifetime[8];     //float4(drag value, time, rot value, time)
    float4 orbitalVelAndRadiusOverLifetime[8];          //float4(orbitalVel, time, orbitalRadius, time)
    uint curveModeBitFlags;
}

cbuffer SpawnConstants : register(b6)
{
    float2 lifetimeRange;
    float2 startSpeedRange;
    float2 startSizeRange;
    float2 startRotRange;
    float3 offsetFromBase;
    uint shapeType;
    float3 boxDimensions;
    uint sphereEmitFrom;
    float sphereRadius;
    float3 boxForward;
    float coneHalfAngle;
    float3 worldPos;
    uint simSpace;
    float3 coneForward;
}

cbuffer CountConstants : register(b5)
{
    int emitCount;
    int totalParticles;
}

cbuffer BillboardConstants : register(b4)
{
    float4 cameraPos;
    float3 cameraUp;
    uint renderMode;
}

float NormalizedAge(Particles particle)
{
    return particle.age / particle.lifetime;
}

float RangeMap(float inputVal, float inputStart, float inputEnd, float outputStart, float outputEnd)
{
    return outputStart + ((inputVal - inputStart) / (inputEnd - inputStart)) * (outputEnd - outputStart);
}

static float PI = 3.14159f;
static float4x4 identityMat =
{
                            //indexing (column major by default in hlsl)
    1.f, 0.f, 0.f, 0.f, //11, 12, 13, 14    |   Ix, Jx, Kx, Tx
    0.f, 1.f, 0.f, 0.f, //21, 22, 23, 24    |   Iy, Jy, Ky, Ty
    0.f, 0.f, 1.f, 0.f, //31, 32, 33, 34    |   Iz, Jz, Kz, Tz
    0.f, 0.f, 0.f, 1.f //41, 42, 43, 44    |   Iw, Jw, Kw, Tw
};

float4x4 GetZRotationMatrix(float degrees)
{
    float radians = degrees * (PI / 180.f);
    float4x4 rotMat = identityMat;
    rotMat._11 = cos(radians);
    rotMat._21 = sin(radians);
    
    rotMat._12 = -sin(radians);
    rotMat._22 = cos(radians);

    return rotMat;
}

float4x4 GetYRotationMatrix(float degrees)
{
    float radians = degrees * (PI / 180.f);
    float4x4 rotMat = identityMat;
    rotMat._11 = cos(radians);
    rotMat._31 = -sin(radians);
    
    rotMat._13 = sin(radians);
    rotMat._33 = cos(radians);

    return rotMat;
}

float4x4 GetXRotationMatrix(float degrees)
{
    float radians = degrees * (PI / 180.f);
    float4x4 rotMat = identityMat;
    rotMat._22 = cos(radians);
    rotMat._32 = sin(radians);
    
    rotMat._23 = -sin(radians);
    rotMat._33 = cos(radians);

    return rotMat;
}

float4x4 GetTranslationMatrix(float3 translation)
{
    float4x4 translationMat = identityMat;
    translationMat._14 = translation.x;
    translationMat._24 = translation.y;
    translationMat._34 = translation.z;
    return translationMat;
}

float4x4 GetRotationMatrixAroundAnAxis(float3 axisUnitVector, float rotationDegrees)
{
    float cosTheta = cos(radians(rotationDegrees));
    float sinTheta = sin(radians(rotationDegrees));
    float OneMinusCosTheta = 1.f - cosTheta;
    float4x4 mat = identityMat;

    mat._11 = cosTheta + (axisUnitVector.x * axisUnitVector.x * OneMinusCosTheta);
    mat._21 = (axisUnitVector.y * axisUnitVector.x * OneMinusCosTheta) + (axisUnitVector.z * sinTheta);
    mat._31 = (axisUnitVector.z * axisUnitVector.x * OneMinusCosTheta) - (axisUnitVector.y * sinTheta);

    mat._12 = (axisUnitVector.x * axisUnitVector.y * OneMinusCosTheta) - (axisUnitVector.z * sinTheta);
    mat._22 = cosTheta + (axisUnitVector.y * axisUnitVector.y * OneMinusCosTheta);
    mat._32 = (axisUnitVector.z * axisUnitVector.y * OneMinusCosTheta) + (axisUnitVector.x * sinTheta);

    mat._13 = (axisUnitVector.x * axisUnitVector.z * OneMinusCosTheta) + (axisUnitVector.y * sinTheta);
    mat._23 = (axisUnitVector.y * axisUnitVector.z * OneMinusCosTheta) - (axisUnitVector.x * sinTheta);
    mat._33 = cosTheta + (axisUnitVector.z * axisUnitVector.z * OneMinusCosTheta);

    return mat;
}

struct RNG
{
    uint s; // state
    
    uint pcg_hash(uint input)
    {
        uint state = input * 747796405u + 2891336453u;
        uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }
    
    uint rand_xorshift()
    {
    // Xorshift algorithm from George Marsaglia's paper
        s ^= (s << 13);
        s ^= (s >> 17);
        s ^= (s << 5);
        return s;
    }
    
    uint SquirrelNoise5(uint positionX, unsigned int seed)
    {
        const uint SQ5_BIT_NOISE1 = 0xd2a80a3f; // 11010010101010000000101000111111
        const uint SQ5_BIT_NOISE2 = 0xa884f197; // 10101000100001001111000110010111
        const uint SQ5_BIT_NOISE3 = 0x6C736F4B; // 01101100011100110110111101001011
        const uint SQ5_BIT_NOISE4 = 0xB79F3ABB; // 10110111100111110011101010111011
        const uint SQ5_BIT_NOISE5 = 0x1b56c4f5; // 00011011010101101100010011110101

        uint mangledBits = positionX;
        mangledBits *= SQ5_BIT_NOISE1;
        mangledBits += seed;
        mangledBits ^= (mangledBits >> 9);
        mangledBits += SQ5_BIT_NOISE2;
        mangledBits ^= (mangledBits >> 11);
        mangledBits *= SQ5_BIT_NOISE3;
        mangledBits ^= (mangledBits >> 13);
        mangledBits += SQ5_BIT_NOISE4;
        mangledBits ^= (mangledBits >> 15);
        mangledBits *= SQ5_BIT_NOISE5;
        mangledBits ^= (mangledBits >> 17);
        return mangledBits;
    }
    
    float Get1dNoiseZeroToOne(uint index, unsigned int seed)
    {
        float ONE_OVER_MAX_UINT = (1.f / 4294967296.f);
        float noiseAsFloat = float(SquirrelNoise5(index, seed));
        return ONE_OVER_MAX_UINT * noiseAsFloat;
    }
    
    float GetRandomFloatZeroToOne()
    {
        return float(rand_xorshift()) * (1.f / 4294967296.f);
    }
    
    float GetRandomFloatInRange(float minInclusive, float maxInclusive)
    {
        return minInclusive + ((maxInclusive - minInclusive) * GetRandomFloatZeroToOne());
    }
    
    float GetRandomFloatInRangeUsing1dNoise(float minInclusive, float maxInclusive, uint indexForNoiseFunction)
    {
        return minInclusive + (Get1dNoiseZeroToOne(indexForNoiseFunction, 0) * (maxInclusive - minInclusive));
    }
    
    float3 GetRandomDirectionInSphere()
    {
        float4x4 yawMat = GetZRotationMatrix(GetRandomFloatInRange(0.f, 360.f));
        float4x4 pitchMat = GetYRotationMatrix(GetRandomFloatInRange(0.f, 360.f));
        float4x4 rollMat = GetXRotationMatrix(GetRandomFloatInRange(0.f, 360.f));

        float4x4 resultantMat = mul(yawMat, pitchMat);
        resultantMat = mul(resultantMat, rollMat);

        return float3(resultantMat._11, resultantMat._21, resultantMat._31);
    }
    
    float3 GetRandomDirectionInCone(float3 forward, float coneAngle)
    {
        float halfAngle = coneAngle * 0.5f;
        float baseYaw = atan2(forward.y, forward.x) * (180.f / PI);
        float lenXY = length(float2(forward.x, forward.y));
        float basePitch = atan2(-forward.z, lenXY) * (180.f / PI);
        float randomYaw = GetRandomFloatInRange(-halfAngle + baseYaw, halfAngle + baseYaw);
        float randomPitch = GetRandomFloatInRange(-halfAngle + basePitch, halfAngle + basePitch);
        //float randomRoll = GetRandomFloatInRange(-halfAngle, halfAngle);
        
        float4x4 yawMat = GetZRotationMatrix(randomYaw);
        float4x4 pitchMat = GetYRotationMatrix(randomPitch);
        float4x4 rollMat = GetXRotationMatrix(0.f);

        float4x4 resultantMat = mul(yawMat, pitchMat);
        resultantMat = mul(resultantMat, rollMat);

        return float3(resultantMat._11, resultantMat._21, resultantMat._31);
    }
};

uint2 GetAnimatedValuesKeysForNormalizedAge(float normalizedAge, float keysTimeArray[8])
{
    uint2 keyIndicies = uint2(0, 0); //uint2(prevFrameIndex, nextFrameIndex)
    [unroll]
    for (int i = 0; i < 8; i++)
    {
        if (normalizedAge < keysTimeArray[i])
        {
            keyIndicies.y = i;
            keyIndicies.x = clamp(i - 1, 0, 7);
            break;
        }
    }

    return keyIndicies;
}

uint4 GetAnimatedValuesKeysForNormalizedAgeFromTwoCurves(float normalizedAge, float2 curveOneKeys[4], float2 curveTwoKeys[4])
{
    uint4 keyIndicies = uint4(0, 0, 0, 0); //uint2(prevFrameIndex, nextFrameIndex)
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        if (normalizedAge < curveOneKeys[i].y)
        {
            keyIndicies.y = i;
            keyIndicies.x = clamp(i - 1, 0, 3);
            break;
        }
    }
    [unroll]
    for (int j = 0; j < 4; j++)
    {
        if (normalizedAge < curveTwoKeys[j].y)
        {
            keyIndicies.w = j;
            keyIndicies.z = clamp(j - 1, 0, 4);
            break;
        }
    }

    return keyIndicies;
}

float GetFractionWithin(float inputValue, float rangeStart, float rangeEnd)
{
    if (rangeStart == rangeEnd)
        return 0.5f;
    return (inputValue - rangeStart) / (rangeEnd - rangeStart);
}

float GetInterpolatedValueFromAnimFrames(float normalizedAge, float2 keysArray[8], uint curveModesBitFlags, uint bitFlag, RNG rng, uint particleId)
{
    bool randBetweenTwoCurves = curveModesBitFlags & bitFlag;
    uint curveOnePrevFrame = 0, curveOneNextFrame = 0;
    uint curveTwoPrevFrame = 0, curveTwoNextFrame = 0;
    float fromValue = 0.f, toValue = 0.f;
    uint4 keyFrames;
    if (randBetweenTwoCurves)
    {
        float2 curveOneKeys[4], curveTwoKeys[4];
        for (int i = 0; i < 4; i++)
        {
            curveOneKeys[i] = keysArray[i];
            curveTwoKeys[i] = keysArray[i + 4];
        }
        
        keyFrames = GetAnimatedValuesKeysForNormalizedAgeFromTwoCurves(normalizedAge, curveOneKeys, curveTwoKeys);
        fromValue = rng.GetRandomFloatInRangeUsing1dNoise(curveOneKeys[keyFrames.x].x, curveTwoKeys[keyFrames.z].x, particleId);
        toValue = rng.GetRandomFloatInRangeUsing1dNoise(curveOneKeys[keyFrames.y].x, curveTwoKeys[keyFrames.w].x, particleId);
    }
    else
    {
        float keyTimeArray[8];
        for (int i = 0; i < 8; i++)
        {
            keyTimeArray[i] = keysArray[i].y;
        }
        uint2 keyFramesForSingleCurve = GetAnimatedValuesKeysForNormalizedAge(normalizedAge, keyTimeArray);
        keyFrames.x = keyFramesForSingleCurve.x;
        keyFrames.y = keyFramesForSingleCurve.y;
        fromValue = keysArray[keyFrames.x].x;
        toValue = keysArray[keyFrames.y].x;
    }
    
    float fractionInRange = GetFractionWithin(normalizedAge, keysArray[keyFrames.x].y, keysArray[keyFrames.y].y);
    float interpolatedValue = lerp(fromValue, toValue, fractionInRange);
    
    return interpolatedValue;
}

static const uint SPAWN_THREAD_COUNT = 128;
static const uint SIMULATION_THREAD_COUNT = 128;
static const uint BILLBOARDING_THREAD_COUNT = 128;
static const uint FINISH_SIM_THREAD_COUNT = 128;
static const uint SORT_THREAD_COUNT = 32;

static const uint COUNT_BUFFER_MAX_PARTICLE_COUNT_OFFSET = 0;
static const uint COUNT_BUFFER_AFTER_EMIT_ALIVE_COUNT_OFFSET = 4;
static const uint COUNT_BUFFER_ALIVE_COUNT_OFFSET = 8;
static const uint COUNT_BUFFER_LAST_ALIVE_INDEX_OFFSET = 12;
static const uint COUNT_BUFFER_AFTER_SIM_DEAD_COUNT_OFFSET = 16;

static const uint ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_X = 1;
static const uint ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_Y = 2;
static const uint ANIM_CURVE_BIT_FLAG_VEL_OVERLIFE_Z = 4;
static const uint ANIM_CURVE_BIT_FLAG_SIZE_OVERLIFE_X = 8;
static const uint ANIM_CURVE_BIT_FLAG_SIZE_OVERLIFE_Y = 16;
static const uint ANIM_CURVE_BIT_FLAG_DRAG_OVERLIFE = 32;
static const uint ANIM_CURVE_BIT_FLAG_ROT_OVERLIFE = 64;
static const uint ANIM_CURVE_BIT_FLAG_COLOR_OVERLIFE = 128;
static const uint ANIM_CURVE_BIT_FLAG_ORBIT_VEL_OVERLIFE = 256;
static const uint ANIM_CURVE_BIT_FLAG_ORBIT_RADIUS_OVERLIFE = 512;