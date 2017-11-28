/*
ZJ Wood CPE 471 Lab 3 base code - I. Dunn class re-write
*/

#include <iostream>
#include <glad/glad.h>
#include <vector>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

static float PI = 3.14159;
static float toRad = PI / 180;

class Application : public EventCallbacks
{



public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> backProg; // program for background gen
	std::shared_ptr<Program> pedalProg; // program for sun pedals gen

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

   	// Components used to generate background shape
   	GLuint BackVertexArrayID;
   	GLuint BackVertexBufferID;

   	// Components used to generate pedal shapes
   	GLuint PedalVertexArrayID;
   	GLuint PedalVertexBufferID;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
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

	// Creates a vertex buffer to represent a circle with at least 80 vertices
	// along the circumference.
	void fillCircleBuffer(std::vector<GLfloat> &g_vertex_circle_buffer_data){
   		float x, y;
   		float z = 2;
   		float deg = 0;
   		float tempDeg = 0;
   		float rad = 0;
   		float inc = 360 / 80; 	// increment of degrees necessary to fill
   								// 80 vertices around circumference

   		while (deg <= 360){
   			tempDeg = deg;

   			// Generate 2 outer vertices sitting on circumference
   			for (int i = 0; i < 2; i++) {
   				rad = toRad * tempDeg;
	   			x = cos(rad);
	   			y = sin(rad);
	   			g_vertex_circle_buffer_data.push_back(x);
	   			g_vertex_circle_buffer_data.push_back(y);
	   			g_vertex_circle_buffer_data.push_back(z);
	   			tempDeg += inc;
   			}
   			// Make third vertex (0,0,0)
   			g_vertex_circle_buffer_data.push_back(0);
   			g_vertex_circle_buffer_data.push_back(0);
   			g_vertex_circle_buffer_data.push_back(2);

   			deg+=inc;
   		}

	}

	// Creates a vertex buffer to represent the pedals of a sun 
	// with at least 40 vertices along the circumference
	void fillCPedalBuffer(std::vector<GLfloat> &g_vertex_pedal_buffer_data){
   		float x, y;
   		float deg = 0;
   		float tempDeg = 0;
   		float rad = 0;
   		float inc = 360 / 40; 	// increment of degrees necessary to fill
   								// 80 vertices around circumference
   		float outer = 0.75;
   		float inner = 0.5;

   		while (deg <= 360){
   			tempDeg = deg;

   			// Generate outer vertex sitting far beyond circumference
			rad = toRad * tempDeg;
   			x = outer * cos(rad);
   			y = outer * sin(rad);
   			g_vertex_pedal_buffer_data.push_back(x);
   			g_vertex_pedal_buffer_data.push_back(y);
   			g_vertex_pedal_buffer_data.push_back(1);
   			tempDeg += inc;

   			// Generate inner vertex sitting between outer and center
			rad = toRad * tempDeg;
   			x = inner * cos(rad);
   			y = inner * sin(rad);
   			g_vertex_pedal_buffer_data.push_back(x);
   			g_vertex_pedal_buffer_data.push_back(y);
   			g_vertex_pedal_buffer_data.push_back(-1);

   			// Make third vertex (0,0,0)
   			g_vertex_pedal_buffer_data.push_back(0);
   			g_vertex_pedal_buffer_data.push_back(0);
   			g_vertex_pedal_buffer_data.push_back(-1);
   			tempDeg -= inc;

   			// Generate outer vertex sitting far beyond circumference
			rad = toRad * tempDeg;
   			x = outer * cos(rad);
   			y = outer * sin(rad);
   			g_vertex_pedal_buffer_data.push_back(x);
   			g_vertex_pedal_buffer_data.push_back(y);
   			g_vertex_pedal_buffer_data.push_back(1);
   			tempDeg -= inc;

   			// Generate inner vertex sitting between outer and center
			rad = toRad * tempDeg;
   			x = inner * cos(rad);
   			y = inner * sin(rad);
   			g_vertex_pedal_buffer_data.push_back(x);
   			g_vertex_pedal_buffer_data.push_back(y);
   			g_vertex_pedal_buffer_data.push_back(-1);

   			// Make third vertex (0,0,0)
   			g_vertex_pedal_buffer_data.push_back(0);
   			g_vertex_pedal_buffer_data.push_back(0);
   			g_vertex_pedal_buffer_data.push_back(-1);

   			deg+=inc;
   		}

	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		// Generate the VAO of background
		glGenVertexArrays(1, &BackVertexArrayID);
		glBindVertexArray(BackVertexArrayID);
		glGenBuffers(1, &BackVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, BackVertexBufferID);

		static const GLfloat g_back_vertex_buffer_data[] =
		{
			-2.0f, -1.0f, -2.0f,
			2.0f, -1.0f, -2.0f,
			2.0f, 1.0f, -2.0f,

			-2.0f, 1.0f, -2.0f,
			-2.0f, -1.0f, -2.0f,
			2.0f, 1.0f, -2.0f,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_back_vertex_buffer_data), g_back_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
		glBindVertexArray(0);

		//generate the VAO of sun's circle
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);
		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		// Declare new vertex buffer to represent a circle
		std::vector<GLfloat> g_vertex_circle_buffer_data;
		// Fill buffer
		fillCircleBuffer(g_vertex_circle_buffer_data);
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_vertex_circle_buffer_data.size(), 
					 &g_vertex_circle_buffer_data.front(), GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		glBindVertexArray(0);

		// Generate the VAO of sun pedals
		glGenVertexArrays(1, &PedalVertexArrayID);
		glBindVertexArray(PedalVertexArrayID);
		glGenBuffers(1, &PedalVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, PedalVertexBufferID);
		std::vector<GLfloat> g_vertex_pedal_buffer_data;
		fillCPedalBuffer(g_vertex_pedal_buffer_data);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_vertex_pedal_buffer_data.size(), 
					 &g_vertex_pedal_buffer_data.front(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
		glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the background program.
		backProg = std::make_shared<Program>();
		backProg->setVerbose(true);
		backProg->setShaderNames(resourceDirectory + "/back_vert33.glsl", resourceDirectory + "/back_frag33.glsl");
		backProg->init();
		backProg->addUniform("P");
		backProg->addUniform("MV");
     	backProg->addUniform("uWindowSize");
     	backProg->addAttribute("vertPos");
     	backProg->addUniform("center");

		// Initialize the sun circle program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/center_vert33.glsl", resourceDirectory + "/center_frag33.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addAttribute("vertPos");
		prog->addUniform("center");
		prog->addUniform("uWindowSize");

		// Initialize the sun pedal program.
		pedalProg = std::make_shared<Program>();
		pedalProg->setVerbose(true);
		pedalProg->setShaderNames(resourceDirectory + "/pedal_vert33.glsl", resourceDirectory + "/pedal_frag33.glsl");
		pedalProg->init();
		pedalProg->addUniform("P");
		pedalProg->addUniform("MV");
		pedalProg->addUniform("uTime");
		pedalProg->addUniform("center");
		pedalProg->addUniform("uWindowSize");
		pedalProg->addAttribute("vertPos");
	}

	/****DRAW
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

		// Create the matrix stacks - please leave these alone for now
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
		MV->pushMatrix();

		

		// ** Draw the sun pedals using GLSL. **
		glBindVertexArray(PedalVertexArrayID);	// CHECK ON THIS LATER ******
		pedalProg->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(pedalProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(pedalProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniform1f(pedalProg->getUniform("uTime"), glfwGetTime());
		glUniform2f(pedalProg->getUniform("center"), width/2.0, height/2.0);
		glUniform2f(pedalProg->getUniform("uWindowSize"), width, height);

		//actually draw from vertex 0, 3 vertices
		glDrawArrays(GL_TRIANGLES, 0, 720);
		pedalProg->unbind();


		// ** Draw the sun circle using GLSL. **
		glBindVertexArray(VertexArrayID);	// CHECK ON THIS LATER ******
		prog->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniform2f(prog->getUniform("center"), width/2.0, height/2.0);
		glUniform2f(prog->getUniform("uWindowSize"), (float) width, (float) height);

		//actually draw from vertex 0, 3 vertices
		glDrawArrays(GL_TRIANGLES, 0, 720);
		prog->unbind();


		// ** Draw the background using GLSL **
		glBindVertexArray(BackVertexArrayID);
		backProg->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(backProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(backProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
      	glUniform2f(backProg->getUniform("uWindowSize"), (float) width, (float) height);
      	glUniform2f(backProg->getUniform("center"), width/2.0, height/2.0);

      	//actually draw from vertex 0, 3 vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		backProg->unbind();

		glBindVertexArray(0);

		// Pop matrix stacks.
		MV->popMatrix();
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
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
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
