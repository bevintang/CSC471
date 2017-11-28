/*
ZJ Wood CPE 471 Lab 3 base code - I. Dunn class re-write
*/

#include <stdlib.h>
#include <iostream>
#include <glad/glad.h>
#include <vector>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Application : public EventCallbacks
{

public:
	float lRotation = 0;	// rotation of left sphere
	float rRotation = 0;

	WindowManager * windowManager = nullptr;

	// Programs
	std::shared_ptr<Program> leftProg; // program for left sphere
	std::shared_ptr<Program> rightProg; // program for right sphere

	// Shapes
	std::shared_ptr<Shape> shape;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Left Sphere rotation control
		else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			lRotation -= 0.1;
		}
		else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			lRotation += 0.1;
		}

		// Right Sphere rotation control
		else if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			rRotation -= 0.1;
		}
		else if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			rRotation += 0.1;
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = std::make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color to white.
		glClearColor(1.0f, 1.0f, 1.0, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize Left Program
		leftProg = std::make_shared<Program>();
     	leftProg->setVerbose(true);
		leftProg->setShaderNames(resourceDirectory + "/globe_vert33.glsl", resourceDirectory + "/globe_frag33.glsl");
		leftProg->init();
		leftProg->addUniform("P");
		leftProg->addUniform("MV");
		leftProg->addAttribute("vertPos");
		leftProg->addAttribute("vertNor");

		// Initialize Right Program
		rightProg = std::make_shared<Program>();
		rightProg->setVerbose(true);
		rightProg->setShaderNames(resourceDirectory + "/right_vert33.glsl", resourceDirectory + "/right_frag33.glsl");
		rightProg->init();
		rightProg->addUniform("P");
		rightProg->addUniform("MV");
		rightProg->addAttribute("vertPos");
		rightProg->addAttribute("vertNor");
	}

	/**** DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks
		auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();
		// Apply orthographic projection.
		P->pushMatrix();
		if (width > height)
		{
			P->ortho(-1*aspect, 1*aspect, -1, 1, -2, 100.0f);
		}
		else
		{
			P->ortho(-1, 1, -1*1/aspect, 1*1/aspect, -2, 100.0f);
		}

		MV->loadIdentity();

		// Draw Left Sphere
		leftProg->bind();
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.5, 0, 0));
			MV->rotate(lRotation, glm::vec3(0, 1, 0));	// rotate on key press
			MV->scale(0.3);
			glUniformMatrix4fv(leftProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(leftProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(leftProg);
		MV->popMatrix();
		leftProg->unbind();

		// Draw Right Sphere
		rightProg->bind();
		MV->pushMatrix();
			MV->translate(glm::vec3(0.5, 0, 0));
			MV->rotate(rRotation, glm::vec3(0, 1, 0));	// rotate on key press
			MV->scale(0.3);
			glUniformMatrix4fv(rightProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(rightProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(rightProg);
		MV->popMatrix();
		rightProg->unbind();

		// Pop matrix stacks.
		P->popMatrix();
	}

};

int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();
		// Swap front and back buffer
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
