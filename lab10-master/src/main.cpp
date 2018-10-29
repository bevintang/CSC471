/**
 * Base code
 * Draws two meshes and one ground plane, one mesh has textures, as well
 * as ground plane.
 * Must be fixed to load in mesh with multiple shapes (dummy.obj)
 */

#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Particle.h"
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
	std::shared_ptr<Program> pprog;

	std::shared_ptr<Shape> shape;

	std::vector<std::shared_ptr<Particle>> particles;

	// CPU array for particles - redundant with particle structure
	// but simple
	int numP = 300;
	GLfloat points[900];
	GLfloat pointColors[1200];

	GLuint pointsbuffer;
	GLuint colorbuffer;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// OpenGL handle to texture data
	shared_ptr<Texture> texture;

	int gMat = 0;

	// Display time to control fps
	float t0_disp = 0.0f;
	float t_disp = 0.0f;

	bool keyToggles[256] = { false };
	float t = 0.0f; //reset in init
	float h = 0.01f;
	glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);

	float camRot;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		keyToggles[key] = ! keyToggles[key];

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			camRot -= 0.314f;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			camRot += 0.314f;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		CHECKED_GL_CALL(glViewport(0, 0, width, height));
	}

	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		CHECKED_GL_CALL(glClearColor(.12f, .34f, .56f, 1.f));

		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/phong_vert33.glsl",
			resourceDirectory + "/phong_frag33.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addUniform("AMBIENCE");
		prog->addUniform("DIFFUSE");
		prog->addUniform("SPECULAR");
		prog->addUniform("SHINE");
		prog->addUniform("uLight");
		prog->addUniform("eye");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");


		pprog = make_shared<Program>();
		pprog->setVerbose(true);
		pprog->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! pprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pprog->addUniform("P");
		pprog->addUniform("MV");
		pprog->addUniform("alphaTexture");
		pprog->addAttribute("vertPos");
	}

	// Code to load in the three textures
	void initTex(const std::string& resourceDirectory)
	{
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/alpha.bmp");
		texture->init();
		texture->setUnit(0);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void initParticles()
	{
		int n = numP;

		for (int i = 0; i < n; ++ i)
		{
			auto particle = make_shared<Particle>();
			particles.push_back(particle);
			particle->load();
		}
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize sphere mesh
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/bunny.obj");
		shape->resize();
		shape->init();

		// generate the VAO
		CHECKED_GL_CALL(glGenVertexArrays(1, &VertexArrayID));
		CHECKED_GL_CALL(glBindVertexArray(VertexArrayID));

		// generate vertex buffer to hand off to OGL - using instancing
		CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));

		CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
	}

	// Note you could add scale later for each particle - not implemented
	void updateGeom()
	{
		glm::vec3 pos;
		glm::vec4 col;

		// go through all the particles and update the CPU buffer
		for (int i = 0; i < numP; i++)
		{
			pos = particles[i]->getPosition();
			col = particles[i]->getColor();
			points[i * 3 + 0] = pos.x;
			points[i * 3 + 1] = pos.y;
			points[i * 3 + 2] = pos.z;
			pointColors[i * 4 + 0] = col.r + col.a / 10.f;
			pointColors[i * 4 + 1] = col.g + col.g / 10.f;
			pointColors[i * 4 + 2] = col.b + col.b / 10.f;
			pointColors[i * 4 + 3] = col.a;
		}

		// update the GPU data
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	/* note for first update all particles should be "reborn"
	 * which will initialize their positions */
	void updateParticles()
	{
		// update the particles
		for (auto particle : particles)
		{
			particle->update(t, h, g, keyToggles);
		}
		t += h;

		// Sort the particles by Z
		auto temp = make_shared<MatrixStack>();
		temp->rotate(camRot, vec3(0, 1, 0));

		ParticleSorter sorter;
		sorter.C = temp->topMatrix();
		std::sort(particles.begin(), particles.end(), sorter);
	}

	void SetMaterial(int i)
	{
		switch (i)
		{
		case 0: //shiny blue plastic
			glUniform3f(prog->getUniform("AMBIENCE"), 0.02f, 0.04f, 0.2f);
			glUniform3f(prog->getUniform("DIFFUSE"), 0.0f, 0.16f, 0.9f);
			glUniform3f(prog->getUniform("SPECULAR"), 0.14, 0.2, 0.8);
		    glUniform1f(prog->getUniform("SHINE"), 120.0);
			break;
		case 1: // flat grey
			glUniform3f(prog->getUniform("AMBIENCE"), 0.13f, 0.13f, 0.14f);
			glUniform3f(prog->getUniform("DIFFUSE"), 0.3f, 0.3f, 0.4f);
			glUniform3f(prog->getUniform("SPECULAR"), 0.3, 0.3, 0.4);
		    glUniform1f(prog->getUniform("SHINE"), 4.0);
			break;
		case 2: //brass
			glUniform3f(prog->getUniform("AMBIENCE"), 0.3294f, 0.2235f, 0.02745f);
			glUniform3f(prog->getUniform("DIFFUSE"), 0.7804f, 0.5686f, 0.11373f);
			glUniform3f(prog->getUniform("SPECULAR"), 0.9922, 0.941176, 0.80784);
		    glUniform1f(prog->getUniform("SHINE"), 27.9);
			break;
		case 3: //ducky feathers
			glUniform3f(prog->getUniform("AMBIENCE"), 0.09, 0.08, 0.03);
		    glUniform3f(prog->getUniform("DIFFUSE"), 0.72, 0.4, 0.1);
		    glUniform3f(prog->getUniform("SPECULAR"), 0.18, 0.16, 0.0);
		    glUniform1f(prog->getUniform("SHINE"), 4.0);
		    break;
	    case 4: // shiny flesh
			glUniform3f(prog->getUniform("AMBIENCE"), 0.5, 0.2, 0.05);
		    glUniform3f(prog->getUniform("DIFFUSE"), 0.6, 0.5, 0.1);
		    glUniform3f(prog->getUniform("SPECULAR"), 0.7, 0.5, 0.1);
		    glUniform1f(prog->getUniform("SHINE"), 2000);
			break;
		}
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width / (float) height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto MV = make_shared<MatrixStack>();
		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);


		// Draw bunny
		MV->pushMatrix();
		MV->loadIdentity();
		MV->pushMatrix();
		MV->translate(vec3(-.25, .25, -2));
		
		prog->bind();
			MV->pushMatrix();	
			MV->scale(0.5);
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE,value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
			glUniform3f(prog->getUniform("uLight"), 0, 10, 2);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			shape->draw(prog);
		prog->unbind();

		MV->popMatrix();
		MV->popMatrix();

		MV->pushMatrix();
		MV->loadIdentity();

		// camera rotate
		MV->rotate(camRot, vec3(0, 1, 0));

		// Draw Particles
		pprog->bind();
		updateParticles();
		updateGeom();

		texture->bind(pprog->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(pprog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(pprog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix())));

		CHECKED_GL_CALL(glEnableVertexAttribArray(0));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0));

		CHECKED_GL_CALL(glEnableVertexAttribArray(1));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 1));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 1));
		// Draw the points !
		CHECKED_GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, numP));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 0));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(1));
		pprog->unbind();

		// Pop matrix stacks.
		MV->popMatrix();
		P->popMatrix();
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
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initParticles();
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
