/* Lab 5 base code - transforms using local matrix functions
	to be written by students -
	CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

// used for helper in perspective
#include "glm/glm.hpp"

using namespace std;
using namespace glm;

class Matrix
{

public:

	static void printMat(float *A, const char *name = 0)
	{
		// OpenGL uses col-major ordering:
		// [ 0  4  8 12]
		// [ 1  5  9 13]
		// [ 2  6 10 14]
		// [ 3  7 11 15]
		// The (i, j)th element is A[i+4*j].

		if (name)
		{
			printf("%s=[\n", name);
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				printf("%- 5.2f ", A[i + 4*j]);
			}
			printf("\n");
		}

		if (name)
		{
			printf("];");
		}
		printf("\n");
	}

	static void createZeroMat(float *M)
	{
		// set all values to zero
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				M[i + 4*j] = 0;
			}
		}
	}

	static void createIdentityMat(float *M)
	{
		// set all values to zero
		createZeroMat(M);
		// overwrite diagonal with 1s
		M[0] = M[5] = M[10] = M[15] = 1;
	}

	static void createTranslateMat(float *T, float x, float y, float z)
	{
		createIdentityMat(T);

		// Make right column x, y, z
		T[12] = x;
		T[13] = y;
		T[14] = z;
	}

	static void createScaleMat(float *S, float x, float y, float z)
	{
		createZeroMat(S);

		// Assign scalar values to diagonal
		S[0] = x;
		S[5] = y;
		S[10] = z;
		S[15] = 1;	// set (4,4) to 1

	}

	static void createRotateMatX(float *R, float radians)
	{
		createIdentityMat(R);
		R[5] = R[10] = cos(radians);
		R[9] = -sin(radians);
		R[6] = sin(radians);
	}

	static void createRotateMatY(float *R, float radians)
	{
		createIdentityMat(R);
		R[0] = R[10] = cos(radians);
		R[8] = sin(radians);
		R[2] = -sin(radians);
	}

	static void createRotateMatZ(float *R, float radians)
	{
		createIdentityMat(R);
		R[0] = R[5] = cos(radians);
		R[1] = sin(radians);
		R[4] = -sin(radians);
	}

	static void multMat(float *C, const float *A, const float *B)
	{
		float c = 0;

		for (int k = 0; k < 4; ++k)
		{
			// Process kth column of C
			for (int i = 0; i < 4; ++i)
			{
				// Process ith row of C.
				// The (i,k)th element of C is the dot product
				// of the ith row of A and kth col of B.
				c = 0;

				// vector dot product
				for (int j = 0; j < 4; ++j)
				{
					c += A[i + 4*j] * B[j + 4*k];
				}

				C[i + 4*k] = c;
			}
		}
	}

	static void createPerspectiveMat(float *m, float fovy, float aspect, float zNear, float zFar)
	{
		// http://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.opengl/doc/openglrf/gluPerspective.htm%23b5c8872587rree
		float f = 1.0f / glm::tan(0.5f * fovy);

		m[ 0] = f / aspect;
		m[ 1] = 0.0f;
		m[ 2] = 0.0f;
		m[ 3] = 0.0f;
		m[ 4] = 0;

		m[ 5] = f;
		m[ 6] = 0.0f;
		m[ 7] = 0.0f;
		m[ 8] = 0.0f;

		m[ 9] = 0.0f;
		m[10] = (zFar + zNear) / (zNear - zFar);
		m[11] = -1.0f;
		m[12] = 0.0f;

		m[13] = 0.0f;
		m[14] = 2.0f * zFar * zNear / (zNear - zFar);
		m[15] = 0.0f;
	}

};

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/cube.obj");
		shape->resize();
		shape->init();
	}

	void render()
	{
		// Local modelview matrix use this for lab 5
		float MV[16] = {0};
		float P[16] = {0};
		float S[16] = {0};
		float Rx[16] = {0};	//rotate matrix about x
		float Ry[16] = {0};	//rotate matrix about y
		float Rz[16] = {0};	//rotate matrix about z
		float R[16] = {0};	//rotate matrix total
		float RS[16] = {0};	//transformation total of scale and rotate
		float T[16] = {0};	//translate matrix
		float TRS[16] = {0};//translate, rotate, scale matrix

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the local matrices for lab 5
		float aspect = width/(float)height;
		Matrix::createPerspectiveMat(P, 70.0f, aspect, 0.1f, 100.0f);
		Matrix::createTranslateMat(T, -4.75, 0, -5);
		Matrix::createScaleMat(S, 0.25, 2.0, 0.25);
		Matrix::createRotateMatX(Rx, 0);
		Matrix::createRotateMatY(Ry, -0.05);
		Matrix::createRotateMatY(Rz, 0);
		Matrix::multMat(RS, Ry, S);
		Matrix::multMat(TRS, T, RS);
		Matrix::createRotateMatY(Ry, -0.5);
		Matrix::multMat(MV, Ry, TRS);

		// Draw left H leg
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
		shape->draw(prog);
		prog->unbind();

		// Draw right H leg
		prog->bind();

		Matrix::createTranslateMat(T, -3.50, 0, -5);
		Matrix::createScaleMat(S, 0.25, 2.0, 0.25);
		Matrix::createRotateMatX(Rx, 0);
		Matrix::createRotateMatY(Ry, -0.03);
		Matrix::createRotateMatY(Rz, 0);		
		Matrix::multMat(RS, Ry, S);
		Matrix::multMat(TRS, T, RS);
		Matrix::createRotateMatY(Ry, -0.5);
		Matrix::multMat(MV, Ry, TRS);

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
		shape->draw(prog);
		prog->unbind();

		// Draw H hyphen
		prog->bind();

		Matrix::createTranslateMat(T, -4.00, 0, -5);
		Matrix::createScaleMat(S, 0.25, 1.0, 0.25);
		Matrix::createRotateMatX(Rx, 0.0);
		Matrix::createRotateMatY(Ry, 0.0);
		Matrix::createRotateMatZ(Rz, 1.2);		
		Matrix::multMat(R, Rz, Ry);
		Matrix::multMat(RS, R, S);
		Matrix::multMat(TRS, T, RS);
		Matrix::createRotateMatY(Ry, -0.5);
		Matrix::multMat(MV, Ry, TRS);

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
		shape->draw(prog);
		prog->unbind();

		// Draw I
		prog->bind();

		Matrix::createTranslateMat(T, -2.0, 0, -5);
		Matrix::createScaleMat(S, 0.25, 2.0, 0.25);
		Matrix::createRotateMatX(Rx, 0.0);
		Matrix::createRotateMatY(Ry, 0.025);
		Matrix::createRotateMatZ(Rz, 0.0);		
		Matrix::multMat(R, Rz, Ry);
		Matrix::multMat(RS, R, S);
		Matrix::multMat(TRS, T, RS);
		Matrix::createRotateMatY(Ry, -0.5);
		Matrix::multMat(MV, Ry, TRS);

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
		shape->draw(prog);
		prog->unbind();
	}
};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
