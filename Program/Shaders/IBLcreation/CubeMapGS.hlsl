cbuffer PerFace : register(b0)
{
    float3 normal[6];
    float4 color[6];
};

struct GSInput
{
    float4 pos : SV_POSITION;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outputStream)
{
    for (uint face = 0; face < 6; face++)
    {
        for (uint i = 0; i < 3; i++)
        {
            PSInput output;
            output.pos = input[i].pos;
            output.normal = normal[face];
            output.color = color[face];
            output.renderTargetArrayIndex = face;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}