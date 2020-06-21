uniform float4x4 mViewProj;
uniform float4x3 mBones[60];
uniform float3   vec_light;

void vs_main( float4 Pos		: POSITION,
			  float3 Norm		: NORMAL,
			  float2 Uv0        : TEXCOORD0,
			  uint   BoneId		: BLENDINDICES0,
					
			  out float4 oPos       : POSITION,
			  out float2 oUv0       : TEXCOORD0,
			  out float3 oNorm		: TEXCOORD1,
			  out float3 oLight		: TEXCOORD2,
			  out float  oDist		: TEXCOORD3)
{
	oPos.xyz = mul( Pos, mBones[BoneId] );
	oPos.w = 1.0f;
	oLight = normalize(vec_light - oPos.xyz);
	oDist = distance(vec_light, oPos.xyz);
	
	oPos = mul( oPos, mViewProj );
	oUv0 = Uv0;
	oNorm = normalize( mul ( Norm, (float3x3)mBones[BoneId] ) );
}