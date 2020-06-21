sampler s_diff : register(s0);

static const float amb = 0.3f;
static const float intensivity = 750.0f;

float4 ps_main(float2 Uv0   : TEXCOORD0,
	float3 Norm : TEXCOORD1,
	float3 Light : TEXCOORD2,
	float  Dist : TEXCOORD3) : COLOR
{
	//return tex2D( s_diff, Uv0 ) * ( amb + max ( 0 , dot( Norm, Light ) * (saturate( (intensivity-Dist) /intensivity) ) ) );
	return tex2D(s_diff, Uv0) * (amb+saturate(dot(Norm, Light))* (saturate((intensivity - Dist) / intensivity)));
}