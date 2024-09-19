#define MAX_PARTICLES 10000

RWBuffer<uint> particleDataRange : register(u1);
RWBuffer<uint> indirectDrawBuffer : register(u2);


//UINT IndexCountPerInstance;
//UINT InstanceCount;
//UINT StartIndexLocation;
//INT BaseVertexLocation;
//UINT StartInstanceLocation;

//UINT bill_IndexCountPerInstance;
//UINT bill_InstanceCount;
//UINT bill_StartIndexLocation;
//INT bill_BaseVertexLocation;
//UINT bill_StartInstanceLocation;

//UINT ThreadGroupCountX;
//UINT ThreadGroupCountY;
//UINT ThreadGroupCount;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    particleDataRange[1] = (particleDataRange[1] + particleDataRange[2]) % MAX_PARTICLES;
    particleDataRange[0] -= particleDataRange[2];
    particleDataRange[2] = 0;
    
    indirectDrawBuffer[1] = particleDataRange[0];
    indirectDrawBuffer[6] = particleDataRange[0];
    
    indirectDrawBuffer[10] = ceil((float) particleDataRange[0] / 64.0f);
}