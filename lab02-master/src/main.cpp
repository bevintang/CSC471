#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Image.h"
#include <algorithm>

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

// Struct of box object. Will 'just' contain the triangle. Also contains
// the vectors contained inside the box object.
struct Box {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
};

// Struct to contain all vertices
struct Triangle {
    int x1;
    int y1;
    int r1;
    int g1;
    int b1;
    int x2;
    int y2;
    int r2;
    int g2;
    int b2;
    int x3;
    int y3;
    int r3;
    int g3;
    int b3;
};

// Calcultes the Barycentric Coordiantes of a given point (x,y) using
// the vectors of a Box struct as reference
void calcBary (int x, int y, Triangle myTri, float* alpha, float* beta, float* gamma){
    float areaA = .5 * ((myTri.x1 - x)*(myTri.y2 - y) - (myTri.x2 - x)*(myTri.y1 - y));
    float areaB = .5 * ((myTri.x2 - x)*(myTri.y3 - y) - (myTri.x3 - x)*(myTri.y2 - y));
    float areaC = .5 * ((myTri.x3 - x)*(myTri.y1 - y) - (myTri.x1 - x)*(myTri.y3 - y));
    float area = areaA + areaB + areaC;
    
    // Assign Barycentric Attributes
    *alpha = areaA / area;
    *beta  = areaB / area;
    *gamma = areaC / area;
}

// Checks Barycentric attributes to determine if a point is inside a triangle
bool isInside(float alpha, float beta, float gamma) {
    if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1 || gamma < 0 || gamma > 1) {
        return false;
    }
    return true;
        
}

int main(int argc, char **argv) {
	if(argc < 4) {
		cout << "Usage: Lab1 <out_image_name>.png width height" << endl;
		return 0;
	}
	// Output filename
	string filename(argv[1]);
	// Width of image
	int width = atoi(argv[2]);
	// Height of image
	int height = atoi(argv[3]);
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	auto image = make_shared<Image>(width, height);

	// Get vertices & colors
	int x_vertex1 = atoi(argv[4]);
	int y_vertex1 = atoi(argv[5]);
    int r1 = atoi(argv[6]);
    int g1 = atoi(argv[7]);
    int b1 = atoi(argv[8]);
	int x_vertex2 = atoi(argv[9]);
	int y_vertex2 = atoi(argv[10]);
    int r2 = atoi(argv[11]);
    int g2 = atoi(argv[12]);
    int b2 = atoi(argv[13]);
	int x_vertex3 = atoi(argv[14]);
	int y_vertex3 = atoi(argv[15]);
    int r3 = atoi(argv[16]);
    int g3 = atoi(argv[17]);
    int b3 = atoi(argv[18]);
    
    // Create Triangle Struct
    Triangle myTri = { x_vertex1, y_vertex1, r1, g1, b1, x_vertex2, y_vertex2,
                       r2, g2, b2, x_vertex3, y_vertex3, r3, g3, b3 };

	// Get dimensions on encapsulating Box
	int xmin = min(x_vertex1, min(x_vertex2, x_vertex3));
	int xmax = max(x_vertex1, max(x_vertex2, x_vertex3));
	int ymin = min(y_vertex1, min(y_vertex2, y_vertex3));
	int ymax = max(y_vertex1, max(y_vertex2, y_vertex3));
    
    // Create Box Object to hold dimensions
	Box myBox = { xmin, xmax, ymin, ymax };
    
    // Draw & Color Triangle
	for(int y = myBox.ymin; y <= myBox.ymax; ++y) {
		for(int x = myBox.xmin; x <= myBox.xmax; ++x) {
            float alpha = -1;
            float beta = -1;
            float gamma = -1;
            calcBary(x, y, myTri, &alpha, &beta, &gamma);
            
            unsigned char r = alpha * r1 + beta * r2 + gamma * r3;
            unsigned char g = alpha * g1 + beta * g2 + gamma * g3;
            unsigned char b = alpha * b1 + beta * b2 + gamma * b3;
            
            if (isInside(alpha, beta, gamma)) {
                image->setPixel(x, y, r, g, b);
            }
            else {
                image->setPixel(x, y, 255, 255, 255);
            }
		}
	}

	// Write image to file
	image->writeToFile(filename);
	return 0;
}
