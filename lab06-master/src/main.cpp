/* Lab 6 base code - transforms using matrix stack built on glm
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

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

	float sTheta;
	float totTranslation = 0;
	float uRot = 0; // upper arm rotation
	float fRot = 0;	// forearm rotation
	bool animated = false;
	bool swingingDown = true;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			totTranslation -= 0.1;
		}

		else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			totTranslation += 0.1;
		}

		else if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			uRot -= 0.1;
		}

		else if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			uRot += 0.1;
		}

		else if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			fRot -= 0.1;
		}

		else if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			fRot += 0.1;
		}

		else if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			if (animated)
				animated = false;
			else 
				animated = true;
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

		sTheta = 0;

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);


		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
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
		// Check on animation and update angle of "hello"
		fRot += incRot(fRot, animated, &swingingDown);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto P = make_shared<MatrixStack>();
		auto MV = make_shared<MatrixStack>();
		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		// Translate the whole robot
		MV->loadIdentity();
		MV->translate(vec3(totTranslation, 0, 0));

		// draw bottom cube
		MV->pushMatrix();
			// draw the bottom cube with these 'global transforms'
			MV->translate(vec3(0, 0, -5));
			MV->scale(vec3(0.75, 0.75, 0.75));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			shape->draw(prog);
			
			// ** Draw right arm ** //

			// Draw right upper arm
			MV->pushMatrix();
				MV->translate(vec3(1.5, 0.30, 0));
				MV->rotate(-45, vec3(0, 0, 1));
				MV->scale(vec3(0.65, 0.25, 0.25));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();

			// Draw right forearm
			MV->pushMatrix();
				MV->translate(vec3(2.3, -.55, 0));
				MV->rotate(-0.5, vec3(0, 0, 1));
				MV->scale(vec3(0.55, 0.25, 0.25));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();

			// Draw right hand
			MV->pushMatrix();
				MV->translate(vec3(3.0, -1.02, 0));
				MV->rotate(-0.5, vec3(0, 0, 1));
				MV->scale(0.3);
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();


			// ** Draw left arm and animate ** //

			// Rotate whole arm
			MV->translate(vec3(-1, 0.75, 0));
			MV->rotate(uRot, vec3(0, 0, 1));
			MV->translate(vec3(-0.5, -0.5, 0));		// shoulder joint

			// Draw left upper arm
			MV->pushMatrix();
				MV->rotate(45, vec3(0, 0, 1));
				MV->scale(vec3(0.65, 0.25, 0.25));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();

			// Rotate forearm and hand
			MV->translate(vec3(-0.3, -0.75, 0));	// forearm position
			MV->rotate(fRot, vec3(0, 0, 1));
			MV->translate(vec3(-.5, 0.0, 0));	// elbow joint

			// Draw left forearm
			MV->pushMatrix();
				MV->rotate(0.5, vec3(0, 0, 1));
				MV->scale(vec3(0.55, 0.25, 0.25));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();

			// Draw left hand
			MV->pushMatrix();
				MV->translate(vec3(-.75, -.35, 0));
				MV->rotate(0.5, vec3(0, 0, 1));
				MV->scale(0.3);
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);
			MV->popMatrix();
		MV->popMatrix();

		// draw top cube - aka head
		MV->pushMatrix();
			// play with these options
			MV->translate(vec3(0, 1.1, -5));
			MV->rotate(0.5, vec3(0, 1, 0));
			MV->scale(vec3(0.5, 0.5, 0.5));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			shape->draw(prog);
		MV->popMatrix();


		prog->unbind();

		// Pop matrix stacks.
		P->popMatrix();
	}

	float incRot(float fRot, bool animated, bool* swingingDown) {
	// check if animation is necessary
	if (!animated)
		return 0;

	// Restrict angle of movement to 45 deg and update direction
	if (fRot < -1)
		*swingingDown = false;
	else if (fRot > 0)
		*swingingDown = true;

	// update rotation based on direction
	if (*swingingDown)
		return -0.025;
	else
		return 0.025;
	}
};

int main(int argc, char *argv[])
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
