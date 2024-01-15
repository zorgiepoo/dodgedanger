
#import <ImageIO/ImageIO.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#import "texture.h"
#import "quit.h"

TextureData loadTextureData(const char *filePath)
{
	NSURL *fileURL = [NSURL fileURLWithPath:@(filePath)];
	NSDictionary *options = @{};
	
	CGImageSourceRef imageSource = CGImageSourceCreateWithURL((CFURLRef)fileURL, (CFDictionaryRef)options);
	
	if (imageSource == NULL)
	{
		fprintf(stderr, "Couldn't create image source for texture: %s\n", filePath);
		ZGQuit();
	}
	
	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	
	CFRelease(imageSource);
	
	if (image == NULL)
	{
		fprintf(stderr, "Couldn't create image for texture: %s\n", filePath);
		ZGQuit();
	}
	
	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);
	size_t bytesPerRow = CGImageGetBytesPerRow(image);
	size_t bytesPerPixel = bytesPerRow / width;
	assert(bytesPerPixel == 4);
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	uint8_t *pixelData = calloc(height * width * bytesPerPixel, sizeof(uint8_t));
	assert(pixelData != NULL);
	
	CGBitmapInfo bitmapInfo = (CGBitmapInfo)kCGImageAlphaPremultipliedLast;
	size_t bitsPerComponent = 8;
	
	CGContextRef context = CGBitmapContextCreate(pixelData, width, height, bitsPerComponent, bytesPerRow, colorSpace, bitmapInfo);
	assert(context != NULL);
	
	CGColorSpaceRelease(colorSpace);
	
	CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
	CGContextDrawImage(context, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height), image);
	CGContextRelease(context);
	CFRelease(image);
	
	TextureData textureData = {.pixelData = pixelData, .width = (int32_t)width, .height = (int32_t)height, .pixelFormat = PIXEL_FORMAT_RGBA32};
	
	return textureData;
}

void freeTextureData(TextureData textureData)
{
	if (textureData.context != NULL)
	{
		CGContextRelease(textureData.context);
	}
	else
	{
		free(textureData.pixelData);
	}
}
