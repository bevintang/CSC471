#include <iostream>
#include <cassert>
#include "Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

Image::Image(int w, int h) :
	width(w),
	height(h),
	comp(3),
	pixels(width*height*comp, 0)
{
}

Image::~Image()
{
}

void Image::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	// The pixel data is laid out row by row. Each row consists of 'width'
	// columns, and each column consists of 3 unsigned chars.

	// First check for bounds
	if(y < 0 || y >= height) {
		cout << "Row " << y << " is out of bounds" << endl;
		return;
	}
	if(x < 0 || x >= width) {
		cout << "Col " << x << " is out of bounds" << endl;
		return;
	}

	// Since the origin (0, 0) of the image is the upper left corner, we need
	// to flip the row to make the origin be the lower left corner.
	y = height - y - 1;
	// index corresponding to row and col, (assuming single component image)
	int index = y*width + x;
	// Multiply by 3 to get the index for the rgb components.
	assert(index >= 0);
	assert(3*index + 2 < (int)pixels.size());
	pixels[3*index + 0] = r;
	pixels[3*index + 1] = g;
	pixels[3*index + 2] = b;
}

void Image::writeToFile(const string &filename)
{
	// The distance in bytes from the first byte of a row of pixels to the
	// first byte of the next row of pixels
	int stride_in_bytes = width*comp*sizeof(unsigned char);
	int rc = stbi_write_png(filename.c_str(), width, height, comp, &pixels[0], stride_in_bytes);
	if(rc) {
		cout << "Wrote to " << filename << endl;
	} else {
		cout << "Couldn't write to " << filename << endl;
	}
}
