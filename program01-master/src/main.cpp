/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

// Struct to hold the vertices's coordiantes of a triangle
struct Triangle {
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
};

// Calcultes the Barycentric Coordiantes of a given point (x,y) using
// the vectors of a Box struct as reference
void calcBary (int x, int y, Triangle myTri, double* alpha, double* beta, double* gamma){
    double areaA = .5 * ((myTri.x1 - x)*(myTri.y2 - y) - (myTri.x2 - x)*(myTri.y1 - y));
    double areaB = .5 * ((myTri.x2 - x)*(myTri.y3 - y) - (myTri.x3 - x)*(myTri.y2 - y));
    double areaC = .5 * ((myTri.x3 - x)*(myTri.y1 - y) - (myTri.x1 - x)*(myTri.y3 - y));
    double area = areaA + areaB + areaC;
    
    // Assign Barycentric Attributes
    *alpha = areaA / area;
    *beta  = areaB / area;
    *gamma = areaC / area;
}

// Checks Barycentric attributes to determine if a point is inside a triangle
bool isInside(double alpha, double beta, double gamma) {
    if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1 || gamma < 0 || gamma > 1) {
        return false;
    }
    return true;
    
}

int main(int argc, char **argv)
{
	if(argc < 6 || argc > 6) {
      cout << "Usage: raster meshfile imagefile x-pixels y-pixels mode" << endl;
      return 0;
   }
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);

	//set g_width and g_height appropriately
    g_width = atoi(argv[3]);
    g_height = atoi(argv[4]);
    
    // mode select
    int mode = atoi(argv[5]);

   //create an image
	auto image = make_shared<Image>(g_width, g_height);

	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) {
		cerr << errStr << endl;
	} else {
 		//keep this code to resize your object to be within -1 -> 1
   	resize_obj(shapes); 
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	cout << "Number of triangles: " << triBuf.size()/3 << endl;
    
    // Create z-buffer and initialize
    vector<vector<float> > zBuf;
    for(int i = 0; i < g_width; i++) {
        vector<float> col(g_height, 0);
        zBuf.push_back(col);
    }
    
	//Iterate through each triangle and rasterize it
    for (int i = 0; i < triBuf.size(); i += 3){ // "For each triangle"
        // Calculate world coordinate translation constants
        float L = -g_width/g_height;
        float R = g_width/g_height;
        int B = -1;
        int T = 1;
        
        // Case where height is > width
        if (g_height > g_width){
            L = -1;
            R = 1;
            B = -g_height/g_width;
            T = g_height/g_width;
        }
        float C = g_width/(R-L);
        float D = -1 * C * L;
        float E = g_height/(T-B);
        float F = -1 * E * B;
        
        // get vertices of 1 triangle using Xp = C * Xw + D, Yp = E * Yw + F
        float x1 = C * posBuf[3*triBuf[i]] + D;
        float y1 = E * posBuf[3*triBuf[i] + 1] + F;
        float z1 = posBuf[3*triBuf[i] + 2];
        float x2 = C * posBuf[3*triBuf[i+1]] + D;
        float y2 = E * posBuf[3*triBuf[i+1] + 1] + F;
        float z2 = posBuf[3*triBuf[i+1] + 2];
        float x3 = C * posBuf[3*triBuf[i+2]] + D;
        float y3 = E * posBuf[3*triBuf[i+2] + 1] + F;
        float z3 = posBuf[3*triBuf[i+2] + 2];
        
        // Store vertices in Triangle struct
        Triangle myTri = {x1, y1, x2, y2, x3, y3};
        
        // create box bounds
        float xmin = min(min(x1, x2), x3);
        float xmax = max(max(x1, x2), x3);
        float ymin = min(min(y1, y2), y3);
        float ymax = max(max(y1, y2), y3);
        
        // set pixels that exist inside the triangles
        for (int y = ymin; y < ymax; y++){
            for (int x = xmin; x < xmax; x++){
                // Calculate Barycentric Coordiantes:
                double alpha = -1;
                double beta = -1;
                double gamma = -1;
                calcBary(x, y, myTri, &alpha, &beta, &gamma);
                double z = alpha * z3 + beta * z1 + gamma * z2 + 1;
                
                // Color the pixel if it exists inside its triangle
                if (isInside(alpha, beta, gamma)) {
                    if (z > zBuf[x][y]){
                        zBuf[x][y] = z;
                    }
                    float r = 0;
                    float g = 0;
                    float b = 0;
                    
                    // Color pixel depending on mode
                    if (mode == 1){
                        r = 255/2 * zBuf[x][y]; // Rasterize depth with the color red and z-buffer
                    }
                    else {  // Rasterize a gradient between green (top) and blue (bottom)
                        r = 19 * y/g_height + 128 - (128*y/g_height);
                        g = 84 * y/g_height + 208 - (208*y/g_height);//255 * y/g_height;
                        b = 122 * y/g_height + 199 - (199*y/g_height);//255 - (255*y/g_height);
                    }
                    image -> setPixel(x, y, r, g, b);
                }
            }
        }
    }
	
	//write out the image
   image->writeToFile(imgName);

    return 0;
}
