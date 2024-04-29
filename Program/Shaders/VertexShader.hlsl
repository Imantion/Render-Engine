struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut main(float2 pos : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = float4(pos, 0.0f, 1.0f);
    output.color = color;

    return output;
}