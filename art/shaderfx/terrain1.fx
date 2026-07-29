
/*** Generated through Lumonix shaderFX  by: Arutha in 3dsmax at: 15-4-2010 16:01:04  ***/ 

// This FX shader was built to support 3ds Max's standard shader compiler. 


int texcoord0 : Texcoord 
<
	int Texcoord = 0;
	int MapChannel = 1;
	string UIType = "None"; 
>;

float LAYER0_UV_SCALE
<
	string UIType = "FloatSpinner";
	float UIMin = -999.0;
	float UIMax = 999.0;
	float UIStep = 0.1;
	string UIName = "LAYER0_UV_SCALE";
> = 1.0;
 
texture Layer0_TS
<
	string Name = "grass.jpg";
	string UIName = "Layer0_TS";
	string ResourceType = "2D";
>;
 
sampler2D Layer0_TSSampler = sampler_state
{
	Texture = <Layer0_TS>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 
texture AlphaMap0_TS : DiffuseMap
<
	string Name = "";
	string UIName = "AlphaMap0_TS";
	string ResourceType = "2D";
>;
 
sampler2D AlphaMap0_TSSampler = sampler_state
{
	Texture = <AlphaMap0_TS>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 
float LAYER1_UV_SCALE
<
	string UIType = "FloatSpinner";
	float UIMin = -999.0;
	float UIMax = 999.0;
	float UIStep = 0.1;
	string UIName = "LAYER1_UV_SCALE";
> = 1.0;
 
texture Layer1_TS
<
	string Name = "sand.jpg";
	string UIName = "Layer1_TS";
	string ResourceType = "2D";
>;
 
sampler2D Layer1_TSSampler = sampler_state
{
	Texture = <Layer1_TS>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 
float LAYER2_UV_SCALE
<
	string UIType = "FloatSpinner";
	float UIMin = -999.0;
	float UIMax = 999.0;
	float UIStep = 0.1;
	string UIName = "LAYER2_UV_SCALE";
> = 1.0;
 
texture Layer2_TS
<
	string Name = "dirt.jpg";
	string UIName = "Layer2_TS";
	string ResourceType = "2D";
>;
 
sampler2D Layer2_TSSampler = sampler_state
{
	Texture = <Layer2_TS>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 

// this function does the different types of light attenuation 
float attenuation_func(int lightattenType, float4 lightAttenuation, float3 lightVec) 
{ 
	float att = 1.0; 
	return att; 
} 
	 
// this function does the different types of cone angle 
float coneangle_func(int lightconeType, float lightHotspot, float lightFalloff, float3 lightVec, float3 lightDir) 
{ 
	float cone = 1.0; 
	return cone; 
} 

/************** light info **************/ 

float3 light1Dir : Direction 
< 
	string UIName = "Light 1 Direction"; 
	string Object = "TargetLight"; 
	string Space = "World"; 
		int refID = 1; 
> = {100.0f, 100.0f, 100.0f}; 

float3 light1Pos : POSITION 
< 
	string UIName = "Light 1 Position"; 
	string Object = "PointLight"; 
	string Space = "World"; 
		int refID = 1; 
> = {100.0f, 100.0f, 100.0f}; 

float4 light1Color : LIGHTCOLOR <int LightRef = 1; string UIWidget = "None"; > = { 1.0f, 1.0f, 1.0f, 1.0f}; 
float4 light1Attenuation : LightAttenuation <int LightRef = 1; string UIWidget = "None"; > = { 20.0f, 30.0f, 0.0f, 100.0f}; 
float light1Hotspot : LightHotSpot <int LightRef = 1; string UIWidget = "None"; > = { 43.0f }; 
float light1Falloff : LightFallOff <int LightRef = 1; string UIWidget = "None"; > = { 45.0f }; 

#define light1Type 1
#define light1attenType 0
#define light1coneType 0
#define light1CastShadows false

//---------------------------------- 

float4x4 wvp : WorldViewProjection < string UIType = "None"; >;  
float4x4 worldI : WorldInverse < string UIType = "None"; >;  
// create the light vector 
float3 lightVec_func(float3 worldSpacePos, float3 lightVector, float3x3 objTangentXf, int lightType) 
{ 
	float3 lightVec = mul(objTangentXf, (mul((lightVector - worldSpacePos), worldI).xyz)); 
	return lightVec; 
} 

// input from application 
	struct a2v { 
	float4 position		: POSITION; 

	float2 texCoord		: TEXCOORD0; 

}; 

// output to fragment program 
struct v2f { 
        float4 position    		: POSITION; 

	float2 texCoord			: TEXCOORD0; 

}; 

// Ambient Pass Vertex Shader: 
v2f av(a2v In, uniform float3 lightPos, uniform int lightType, uniform float3 lightDir) 
{ 
	v2f Out = (v2f)0; 
	Out.position = mul(In.position, wvp);				//transform vert position to homogeneous clip space 

	Out.texCoord = In.texCoord;						//pass through texture coordinates from channel 1 

	return Out; 
} 

// Ambient Pass Pixel Shader: 
float4 af(v2f In, uniform float3 lightDir, uniform float4 lightColor, uniform float4 lightAttenuation, uniform float lightHotspot, uniform float lightFalloff, uniform int lightType, uniform int lightattenType, uniform int lightconeType, uniform bool lightCastShadows, uniform int shadowPassCount) : COLOR 
{ 

	float2 LAYER0_UV = In.texCoord.xy; 
	float4 Layer0_TU = tex2D(Layer0_TSSampler, (LAYER0_UV * LAYER0_UV_SCALE).xy);
	float4 Alpha0_TU = tex2D(AlphaMap0_TSSampler, In.texCoord.xy);
	float2 LAYER1_UV = In.texCoord.xy; 
	float4 Layer1_TU = tex2D(Layer1_TSSampler, (LAYER1_UV * LAYER1_UV_SCALE).xy);
	float2 LAYER2_UV = In.texCoord.xy; 
	float4 Layer2_TU = tex2D(Layer2_TSSampler, (LAYER2_UV * LAYER2_UV_SCALE).xy);
	float3 input1 = (((Layer0_TU.rgb * Alpha0_TU.r) + (Layer1_TU.rgb * Alpha0_TU.g)) + (Layer2_TU.rgb * Alpha0_TU.b)); 

	float4 ret =  float4(input1, 1); 
	return ret; 
} 

technique Complete  
{  
	pass ambient  
    {		 
		VertexShader = compile vs_2_0 av(light1Pos,  light1Type, light1Dir); 
		ZEnable = true; 
		ZWriteEnable = true; 
		CullMode = cw; 
		ShadeMode = Gouraud;
		AlphaBlendEnable = false; 
		AlphaTestEnable = FALSE; 
		PixelShader = compile ps_2_0 af(light1Dir, light1Color, light1Attenuation, light1Hotspot, light1Falloff, light1Type, light1attenType, light1coneType, light1CastShadows, 1); 
	}  

}    