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
	float rotation = 0;		// rotation of meshes
	float xLight = -2;		// x position of light source
	int shaderToggle = 0;	// controls which shader will be used
	int matToggle = 0;		// controls which material will be displayed

	WindowManager * windowManager = nullptr;

	// Programs
	std::shared_ptr<Program> leftProg; // program for left sphere

	// Shapes
	std::shared_ptr<Shape> shape;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Rotation control
		else if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			rotation += 0.05;
		}

		// X-position Light Control
		else if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			xLight -= 0.2;
		}
		else if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			xLight += 0.2;
		}

		// Shader control
		else if (key == GLFW_KEY_P && action == GLFW_PRESS){
			shaderToggle += 1;
			if (shaderToggle == 4)
				shaderToggle = 0;
		}

		// Material control
		else if (key == GLFW_KEY_M && action == GLFW_PRESS){
			matToggle += 1;
			if (matToggle == 4)
				matToggle = 0;
		}
	}

	void setMaterial(int matToggle) {
	  	switch (matToggle) {
	  	case 0: // shiny blue plastic
		    glUniform3f(leftProg->getUniform("AMBIENCE"), 0.02, 0.04, 0.2);
		    glUniform3f(leftProg->getUniform("DIFFUSE"), 0.0, 0.16, 0.9);
		    glUniform3f(leftProg->getUniform("SPECULAR"), 0.14, 0.2, 0.8);
		    glUniform1f(leftProg->getUniform("SHINE"), 120.0);
		    break;
	  	case 1: // flat geometry
		    glUniform3f(leftProg->getUniform("AMBIENCE"), 0.13, 0.13, 0.14);
		    glUniform3f(leftProg->getUniform("DIFFUSE"), 0.3, 0.3, 0.4);
		    glUniform3f(leftProg->getUniform("SPECULAR"), 0.3, 0.3, 0.4);
		    glUniform1f(leftProg->getUniform("SHINE"), 4.0);
		    break;
	  	case 2: // brass
		    glUniform3f(leftProg->getUniform("AMBIENCE"), 0.3294, 0.2235, 0.02745);
		    glUniform3f(leftProg->getUniform("DIFFUSE"), 0.7804, 0.5686, 0.11373);
		    glUniform3f(leftProg->getUniform("SPECULAR"), 0.9922, 0.941176, 0.80784);
		    glUniform1f(leftProg->getUniform("SHINE"), 27.9);
		    break;
	   	case 3: // chocolate
	   		glUniform3f(leftProg->getUniform("AMBIENCE"), 0.1, 0.1, 0.1);
		    glUniform3f(leftProg->getUniform("DIFFUSE"), 0.20, 0.08, 0.08);
		    glUniform3f(leftProg->getUniform("SPECULAR"), 0.7, 0.2, 0.2);
		    glUniform1f(leftProg->getUniform("SHINE"), 1000);
		    break;
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

	std::string getVertShader(int shaderToggle){
		switch(shaderToggle){
		// Normal
		case 0:
			return "/nor_vert33.glsl";
		// Gouraud
		case 1:
			return "/gouraud_vert33.glsl";
		// Phong
		case 2:
			return "/phong_vert33.glsl";
		// Silhouette
		default:
			return "/sil_vert33.glsl";
		}
	}

	std::string getFragShader(int shaderToggle){
		switch(shaderToggle){
		// Normal
		case 0:
			return "/nor_frag33.glsl";
		// Gouraud
		case 1:
			return "/gouraud_frag33.glsl";
		// Phong
		case 2:
			return "/phong_frag33.glsl";
		// Silhouette
		default:
			return "/sil_frag33.glsl";
		}
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(std::string& objPath)
	{
		// Initialize mesh.
		shape = std::make_shared<Shape>();
		shape->loadMesh(objPath);
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

		std::string vertShader = getVertShader(shaderToggle);
		std::string fragShader = getFragShader(shaderToggle);

		// Initialize Left Program
		leftProg = std::make_shared<Program>();
     	leftProg->setVerbose(true);
		leftProg->setShaderNames(resourceDirectory + vertShader, resourceDirectory + fragShader);
		leftProg->init();
		leftProg->addUniform("P");
		leftProg->addUniform("MV");
		leftProg->addUniform("AMBIENCE");
		leftProg->addUniform("DIFFUSE");
		leftProg->addUniform("SPECULAR");
		leftProg->addUniform("SHINE");
		leftProg->addUniform("uLight");
		leftProg->addUniform("eye");
		leftProg->addAttribute("vertPos");
		leftProg->addAttribute("vertNor");
	}

	void reInit(int shaderToggle, const std::string& resourceDirectory){
		// Update string names
		std::string vertShader = getVertShader(shaderToggle);
		std::string fragShader = getFragShader(shaderToggle);

		leftProg->setShaderNames(resourceDirectory + vertShader, resourceDirectory + fragShader);
		leftProg->init();
		leftProg->addUniform("P");
		leftProg->addUniform("MV");
		leftProg->addUniform("AMBIENCE");
		leftProg->addUniform("DIFFUSE");
		leftProg->addUniform("SPECULAR");
		leftProg->addUniform("SHINE");
		leftProg->addUniform("uLight");
		leftProg->addUniform("eye");
		leftProg->addAttribute("vertPos");
		leftProg->addAttribute("vertNor");
	}

	/**** DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render(const std::string& resourceDirectory)
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Reinitialize Left and Right Program to grab new shader
		reInit(shaderToggle, resourceDirectory);

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
		// Draw Left Object
		leftProg->bind();
		setMaterial(matToggle);
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.5, 0, -5));
			MV->rotate(rotation - 1, glm::vec3(0, 1, 0));	// rotate on key press
			MV->scale(0.45);
			glUniformMatrix4fv(leftProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(leftProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(leftProg->getUniform("uLight"), xLight, 2, 2);
			glUniform3f(leftProg->getUniform("eye"), 0.0, 0.0, 0.0);
			shape->draw(leftProg);
		MV->popMatrix();

		// Draw Right object
		MV->pushMatrix();
			MV->translate(glm::vec3(0.5, 0, -5));
			MV->rotate(rotation + 1, glm::vec3(0, 1, 0));	// rotate on key press
			MV->scale(0.45);
			glUniformMatrix4fv(leftProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(leftProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(leftProg->getUniform("uLight"), xLight, 2, 2);
			glUniform3f(leftProg->getUniform("eye"), 0.0, 0.0, 0.0);
			shape->draw(leftProg);
		MV->popMatrix();
		leftProg->unbind();

		// Pop matrix stacks.
		P->popMatrix();
	}

};

int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	std::string objPath = "../resources/bunny.obj";
	if (argc == 2)
	{
		objPath = argv[1];
	}
	if (argc > 2){
		fprintf(stderr, "Usage: [program3] [OBJ FILE PATH]\n");
		return 1;
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
	application->initGeom(objPath);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render(resourceDir);
		// Swap front and back buffer
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
