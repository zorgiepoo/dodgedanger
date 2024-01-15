
#include <metal_stdlib>
#include <simd/simd.h>
#include "metal_indices.h"
#include "float.h"

using namespace metal;

vertex float4 positionVertexShader(const ushort vertexID [[ vertex_id ]], const device ZGFloat4 *vertices [[ buffer(METAL_BUFFER_VERTICES_INDEX) ]], constant ZGFloat_matrix4x4 &modelViewProjection [[ buffer(METAL_BUFFER_MODELVIEW_PROJECTION_INDEX) ]])
{
	return float4(modelViewProjection * vertices[vertexID]);
}

fragment ZGFloat4 positionFragmentShader(constant ZGFloat4 &color [[ buffer(METAL_BUFFER_COLOR_INDEX) ]])
{
	return color;
}

typedef struct
{
	float4 position [[position]];
	float2 textureCoordinate;
} TextureRasterizerData;

vertex TextureRasterizerData texturePositionVertexShader(const ushort vertexID [[ vertex_id ]], const device ZGFloat4 *vertices [[ buffer(METAL_BUFFER_VERTICES_INDEX) ]], constant ZGFloat_matrix4x4 &modelViewProjection [[ buffer(METAL_BUFFER_MODELVIEW_PROJECTION_INDEX) ]], const device ZGFloat2 *textureCoordinates [[ buffer(METAL_BUFFER_TEXTURE_COORDINATES_INDEX) ]])
{
	TextureRasterizerData output;
	
	output.position = float4(modelViewProjection * vertices[vertexID]);
	output.textureCoordinate = float2(textureCoordinates[vertexID]);
	
	return output;
}

fragment ZGFloat4 texturePositionFragmentShader(const TextureRasterizerData input [[stage_in]], const texture2d<ZGFloat> texture [[ texture(METAL_TEXTURE_INDEX) ]], constant ZGFloat4 &color [[ buffer(METAL_BUFFER_COLOR_INDEX) ]])
{
	constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
	
	const ZGFloat4 colorSample = texture.sample(textureSampler, input.textureCoordinate);
	
	return color * ZGFloat4(colorSample);
}
