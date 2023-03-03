struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct v2p_t
{
    float4 position : SV_Position;
    float3 cubeLocalPos : POSITION;
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

Texture2DArray skyboxTextures : register(t0);
SamplerState diffuseSampler : register(s0);

v2p_t VertexMain(vs_input_t input)
{
    float4 localPosition = float4(input.localPosition.x, input.localPosition.y, input.localPosition.z, 1.f);
    float4 modelSpacePos = mul(modelMatrix, localPosition);
    float4 viewSpacePos = mul(viewMatrix, modelSpacePos);
    float4 ndcPos = mul(projectionMatrix, viewSpacePos);
    v2p_t v2p;
    v2p.position = ndcPos;
    v2p.position.z = v2p.position.w;
    v2p.cubeLocalPos = input.localPosition;
    return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
    float absX = abs(input.cubeLocalPos.x);
    float absY = abs(input.cubeLocalPos.y);
    float absZ = abs(input.cubeLocalPos.z);
    
    uint faceToSample;
    float texCoordU = 0.f;
    float texCoordV = 0.f;
    if (input.cubeLocalPos.x > 0 && absX >= absY && absX >= absZ)
    {
        faceToSample = 0; //front face
        texCoordU = -input.cubeLocalPos.y;
        texCoordV = input.cubeLocalPos.z;
    }
    else if (input.cubeLocalPos.x < 0 && absX >= absY && absX >= absZ)
    {
    
        faceToSample = 1;   //back face
        texCoordU = input.cubeLocalPos.y;
        texCoordV = input.cubeLocalPos.z;
    }
    else if (input.cubeLocalPos.y > 0 && absY >= absX && absY >= absZ)
    {
        faceToSample = 2;   //left face
        texCoordU = input.cubeLocalPos.x;
        texCoordV = input.cubeLocalPos.z;
    }
    else if (input.cubeLocalPos.y < 0 && absY >= absX && absY >= absZ)
    {
        faceToSample = 3;   //right face
        texCoordU = -input.cubeLocalPos.x;
        texCoordV = input.cubeLocalPos.z;
    }
    else if (input.cubeLocalPos.z > 0 && absZ >= absX && absZ >= absY)
    {
        faceToSample = 4;   //top face
        texCoordU = -input.cubeLocalPos.y;
        texCoordV = -input.cubeLocalPos.x;
    }
    else
    {
        faceToSample = 5;   //bottom face
        texCoordU = -input.cubeLocalPos.y;
        texCoordV = input.cubeLocalPos.x;
    }
    
    texCoordU = (texCoordU + 1.f) / 2.f;
    texCoordV = (texCoordV + 1.f) / 2.f;
    float3 samplerInput = float3(texCoordU, texCoordV, faceToSample);
    return skyboxTextures.Sample(diffuseSampler, samplerInput);
}
