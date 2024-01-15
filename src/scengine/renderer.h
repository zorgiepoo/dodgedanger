
#pragma once

#include "math_3d.h"
#include "renderer_types.h"

void createRenderer(Renderer *renderer, RendererCreateOptions options);

void updateViewport(Renderer *renderer, int32_t windowWidth, int32_t windowHeight);

void renderFrame(Renderer *renderer, void (*drawFunc)(Renderer *, void *), void *context);

TextureObject textureFromPixelData(Renderer *renderer, const void *pixels, int32_t width, int32_t height, PixelFormat pixelFormat);

void deleteTexture(Renderer *renderer, TextureObject texture);

BufferObject createIndexBufferObject(Renderer *renderer, const void *data, uint32_t size);

BufferObject rectangleIndexBufferObject(Renderer *renderer);

BufferArrayObject createVertexArrayObject(Renderer *renderer, const void *vertices, uint32_t verticesSize);

BufferArrayObject createVertexAndTextureCoordinateArrayObject(Renderer *renderer, const void *verticesAndTextureCoordinates, uint32_t verticesSize, uint32_t textureCoordinatesSize);

void drawVertices(Renderer *renderer, mat4_t modelViewMatrix, RendererMode mode, BufferArrayObject vertexArrayObject, uint32_t vertexCount, color4_t color, RendererOptions options);

void drawVerticesFromIndices(Renderer *renderer, mat4_t modelViewMatrix, RendererMode mode, BufferArrayObject vertexArrayObject, BufferObject indicesBufferObject, uint32_t indicesCount, color4_t color, RendererOptions options);

void drawTextureWithVertices(Renderer *renderer, mat4_t modelViewMatrix, TextureObject texture, RendererMode mode, BufferArrayObject vertexAndTextureArrayObject, uint32_t vertexCount, color4_t color, RendererOptions options);

void drawTextureWithVerticesFromIndices(Renderer *renderer, mat4_t modelViewMatrix, TextureObject texture, RendererMode mode, BufferArrayObject vertexAndTextureArrayObject, BufferObject indicesBufferObject, uint32_t indicesCount, color4_t color, RendererOptions options);

void pushDebugGroup(Renderer *renderer, const char *debugGroupName);
void popDebugGroup(Renderer *renderer);
