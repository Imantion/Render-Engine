#include "..\\declarations.hlsli"

struct PSIn
{
    float4 pos : SV_Position;
    float3 worldPos : WorldPos;
    float3x3 tbn : TBN;
    float2 tc : TC;
    nointerpolation int BoneIDs[MAX_BONE_INFLUENCE] : BONES;
    nointerpolation float Weights[MAX_BONE_INFLUENCE] : WEIGHTS;
    nointerpolation uint objectId : OBJECTID;
    
};

cbuffer bone : register(b15)
{
    int selectedBoneId;
}

float4 main(PSIn input) : SV_TARGET
{
	float4 fragColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (input.BoneIDs[i] == selectedBoneId)
        {
            if(input.Weights[i] > 0.7)
            {
                fragColor = float4(1.0f, 0.0f, 0.0f, 1.0f) * input.Weights[i];
            }
            else if(input.Weights[i] < 0.7 && input.Weights[i] > 0.4)
            {
                fragColor = float4(0.0f, 1.0f, 0.0f, 1.0f) * input.Weights[i];
            }
            else
            {
                fragColor = float4(0.0f, 1.0f, 1.0f, 1.0f) * input.Weights[i];
            }
        }
    }
    
    return fragColor;
}