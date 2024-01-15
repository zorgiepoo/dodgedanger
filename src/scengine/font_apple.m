
#include "font.h"

#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

static CTFontRef gFontRef;
static CGColorSpaceRef gRGBColorSpace;

void initFontFromFile(const char *filePath, int pointSize)
{
    NSURL *fileURL = [NSURL fileURLWithFileSystemRepresentation:filePath isDirectory:NO relativeToURL:nil];
	NSArray *fontDescriptors = (NSArray *)CFBridgingRelease(CTFontManagerCreateFontDescriptorsFromURL((CFURLRef)fileURL));
	assert(fontDescriptors.count > 0);
	
	gFontRef = CTFontCreateWithFontDescriptor((CTFontDescriptorRef)fontDescriptors[0], (CGFloat)pointSize, NULL);
    assert(gFontRef != nil);
    
	gRGBColorSpace = CGColorSpaceCreateDeviceRGB();
}

void initFontWithName(const char *name, int pointSize)
{
    NSString *fontName = @(name);
    
    gFontRef = CTFontCreateWithName((CFStringRef)fontName, (CGFloat)pointSize, NULL);
    assert(gFontRef != nil);
    
    gRGBColorSpace = CGColorSpaceCreateDeviceRGB();
}

// Info on how to grab raw pixel data from drawn text using CoreText - https://stackoverflow.com/a/41798782/871119
TextureData createTextData(const char *string)
{
	NSMutableAttributedString *text = [[NSMutableAttributedString alloc] initWithString:@(string)];
	NSRange textRange = NSMakeRange(0, text.length);

	[text beginEditing];
	[text addAttribute:(NSString *)kCTFontAttributeName value:(__bridge id)gFontRef range:textRange];
	[text addAttribute:(NSString *)kCTForegroundColorFromContextAttributeName value:@YES range:textRange];
	[text endEditing];

	CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString((CFAttributedStringRef)text);

	CGSize constraints = CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX);
	CFRange cfTextRange = CFRangeMake(textRange.location, textRange.length);
	CGSize frameSize = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, cfTextRange, NULL, constraints, NULL);

	CGPathRef path = CGPathCreateWithRect(CGRectMake(0.0, 0.0, frameSize.width, frameSize.height), NULL);
	CTFrameRef frame = CTFramesetterCreateFrame(framesetter, cfTextRange, path, NULL);
	
	CFRelease(framesetter);
	CGPathRelease(path);
	
	CGBitmapInfo bitmapInfo = (CGBitmapInfo)kCGImageAlphaPremultipliedLast;

	CGContextRef context = CGBitmapContextCreate(NULL, (size_t)ceil(frameSize.width), (size_t)ceil(frameSize.height), 8, (size_t)ceil(frameSize.width) * 4, gRGBColorSpace, bitmapInfo);

	CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
	CTFrameDraw(frame, context);
	
	CFRelease(frame);

	return (TextureData){.pixelData = CGBitmapContextGetData(context), .context = context, .width = (int32_t)CGBitmapContextGetWidth(context), .height = (int32_t)CGBitmapContextGetHeight(context), .pixelFormat = PIXEL_FORMAT_RGBA32};
}
