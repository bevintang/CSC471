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

static float PI = 3.14159;
static float TO_RAD = PI / 180;
float frontLegRotation = 0;
float snowFall = 2;
bool frontLegForward = false;

float incLeg(float count, bool* legForward){
	if (count < -1){
		*legForward = false;
	}
	else if (count > 1){
		*legForward = true;
	}

	if (*legForward)
		return -0.04;
	else
		return 0.04;
}

float incSnow(float* snowFall){
	if (*snowFall < -1.75){
		*snowFall = 1.75;
	}
	return -0.01;
}

class Application : public EventCallbacks
{

public:
	float rotation = 0;


	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> cylProg; 	// program for cylinder gen
	std::shared_ptr<Program> globeProg; // program for globe gen
	std::shared_ptr<Program> torsoProg; // program for torso gen
	std::shared_ptr<Program> legProg; 	// program for leg gen
	std::shared_ptr<Program> snowProg;  // program for snow gen

	// Shape to be used (from obj file)
	std::shared_ptr<Shape> shape;

	// Contains cylinder vertex information for OpenGL
	GLuint VertexArrayID;
	GLuint VertexBufferID;

   	// Components used to generate background shape
   	GLuint BackVertexArrayID;
   	GLuint BackVertexBufferID;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			rotation -= 0.1;
		}
		else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			rotation += 0.1;
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

	/*
		Fills a buffer with vertices that represent a cylinder (with caps).

		The generation is centered around a unit circle, whose face is drawn
		on the x-z plane. The x-y plane will show the face of the cylinder.

		The faces will be created by 2 triangles, and will require x and z
		coordiantes from two spots along the circumference of the unit circle.
	*/
	void fillCylinderBuffer(std::vector<GLfloat> &g_vertex_cylinder_buffer_data){
   		float x = 0;			// with y, will house the face coordiantes
   		float y = 0.5;			// fixed unit height of cylinder
   		float z = 0;			// with combination of x, will represent circle
   		float nextX = 0;		// future x coordinate
   		float nextZ = 0;		// future z coordiante
   		float deg = 0;			// current rotational position of circle
   		float nextDeg = 0;		// future rotational position
   		float rad = 0;			// deg equivalent in radians
   		float nextRad = 0;		// nextDeg equivalent in radians
   		float inc = 360 / 40; 	// increment of degrees necessary

   		/*
   			** Generate each face of the cylinder: **

   			Each cylinder will be comprised of two triangles, forming a rectangle:

   			The First Triangle will have 1 vertex along the top of the cylinder &
   			2 along the bottom of the cylinder.

   			The Second Triangle will have 2 vertexes along the top of the cylinder
   			& 1 along the bottom.
   		*/
   		while (deg <= 360){
   			// Calculate coordinates for current rotational position
			rad = TO_RAD * deg;
			x = cos(rad);
   			z = sin(rad);

   			// Calculatre coordiantes for next rotational position
   			nextDeg = deg + inc;		// update next rotational position
   			nextRad = TO_RAD * nextDeg;	// convert degrees to radians
   			nextX = cos(nextRad);
   			nextZ = sin(nextRad);
   			

   			// Add vertices of 1st Triangle to VertBuffer
   			g_vertex_cylinder_buffer_data.push_back(x);		// top-left vertex
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(z);

			g_vertex_cylinder_buffer_data.push_back(x);		// bottom-left
   			g_vertex_cylinder_buffer_data.push_back(-y);
   			g_vertex_cylinder_buffer_data.push_back(z);

   			g_vertex_cylinder_buffer_data.push_back(nextX);	// bottom-right
   			g_vertex_cylinder_buffer_data.push_back(-y);
   			g_vertex_cylinder_buffer_data.push_back(nextZ);

   			// Add vertices of 2nd Triangle to VertBuffer
   			g_vertex_cylinder_buffer_data.push_back(x);		// top-left
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(z);

   			g_vertex_cylinder_buffer_data.push_back(nextX);	// top-right
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(nextZ);

   			g_vertex_cylinder_buffer_data.push_back(nextX);	// bottom-right
   			g_vertex_cylinder_buffer_data.push_back(-y);
   			g_vertex_cylinder_buffer_data.push_back(nextZ);

   			// Add top cap
   			g_vertex_cylinder_buffer_data.push_back(0);		// circle center	
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(0);

   			g_vertex_cylinder_buffer_data.push_back(x);		// top-left	
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(z);

   			g_vertex_cylinder_buffer_data.push_back(nextX);	// top-right
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(nextZ);

   			// Add bottom cap
   			g_vertex_cylinder_buffer_data.push_back(0);		// circle center	
   			g_vertex_cylinder_buffer_data.push_back(y);
   			g_vertex_cylinder_buffer_data.push_back(0);

   			g_vertex_cylinder_buffer_data.push_back(x);		// bottom-left	
   			g_vertex_cylinder_buffer_data.push_back(-y);
   			g_vertex_cylinder_buffer_data.push_back(z);

   			g_vertex_cylinder_buffer_data.push_back(nextX);	// bottom-right
   			g_vertex_cylinder_buffer_data.push_back(-y);
   			g_vertex_cylinder_buffer_data.push_back(nextZ);

   			deg+=inc;	// Continue Loop
   		}

	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = std::make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

		// ** Generate the VAO of a cylinder **
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);
		glGenBuffers(1, &VertexBufferID);				
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);	

		// Declare new vertex buffer to represent a cylinder
		std::vector<GLfloat> g_vertex_cylinder_buffer_data;
		fillCylinderBuffer(g_vertex_cylinder_buffer_data);		// fill buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_vertex_cylinder_buffer_data.size(), 
					 &g_vertex_cylinder_buffer_data.front(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		glBindVertexArray(0);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color to white.
		glClearColor(0.1f, 0.05f, 0.4, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

     	// Initialize Cylinder program.
     	cylProg = std::make_shared<Program>();
     	cylProg->setVerbose(true);
		cylProg->setShaderNames(resourceDirectory + "/cylinder_vert33.glsl", resourceDirectory + "/cylinder_frag33.glsl");
		cylProg->init();
		cylProg->addUniform("P");
		cylProg->addUniform("MV");
		cylProg->addAttribute("vertPos");

		// Initialize Globe Program
		globeProg = std::make_shared<Program>();
     	globeProg->setVerbose(true);
		globeProg->setShaderNames(resourceDirectory + "/globe_vert33.glsl", resourceDirectory + "/globe_frag33.glsl");
		globeProg->init();
		globeProg->addUniform("P");
		globeProg->addUniform("MV");
		globeProg->addAttribute("vertPos");
		globeProg->addUniform("uWindowSize");
     	globeProg->addAttribute("vertPos");
     	globeProg->addUniform("center");

     	// Initialize torso program.
     	torsoProg = std::make_shared<Program>();
     	torsoProg->setVerbose(true);
		torsoProg->setShaderNames(resourceDirectory + "/torso_vert33.glsl", resourceDirectory + "/torso_frag33.glsl");
		torsoProg->init();
		torsoProg->addUniform("P");
		torsoProg->addUniform("MV");
		torsoProg->addAttribute("vertPos");

		// Initialize leg program.
     	legProg = std::make_shared<Program>();
     	legProg->setVerbose(true);
		legProg->setShaderNames(resourceDirectory + "/leg_vert33.glsl", resourceDirectory + "/leg_frag33.glsl");
		legProg->init();
		legProg->addUniform("P");
		legProg->addUniform("MV");
		legProg->addAttribute("vertPos");

		// Initialize snow program.
     	snowProg = std::make_shared<Program>();
     	snowProg->setVerbose(true);
		snowProg->setShaderNames(resourceDirectory + "/snow_vert33.glsl", resourceDirectory + "/snow_frag33.glsl");
		snowProg->init();
		snowProg->addUniform("P");
		snowProg->addUniform("MV");
		snowProg->addAttribute("vertPos");
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
		MV->rotate(rotation, glm::vec3(0, 1, 0));

		// Draw Sphere
		globeProg->bind();
		MV->pushMatrix();
			glUniformMatrix4fv(globeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(globeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform2f(globeProg->getUniform("uWindowSize"), (float) width, (float) height);
      		glUniform2f(globeProg->getUniform("center"), width/2.0, height/2.0);
			shape->draw(globeProg);
		MV->popMatrix();
		globeProg->unbind();

		// Draw Snow
		snowProg -> bind();
		MV->pushMatrix();
		MV->scale(0.02);
		float snowflakes[20] = {
			-0.34377635984857,
			-0.21203065813148,
			0.06595765290128,
			0.087890105828592,
			-0.88018736330801,
			0.41395774549523,
			0.92307620585108,
			-0.89290741826077,
			0.66125891993812,
			0.14330479555917,
			-0.72122394280565,
			0.83475478078926,
			0.13368540310007,
			0.2436373723874,
			0.61582609713815,
			0.72066904591428,
			-0.36050521366322,
			0.30296787214604,
			-0.17474713231192,
			-0.76881351404302
		};

		float depth[20] = {
			0.41274674023164,
			1.3036685354559,
			-0.60780836530393,
			0.41802184908559,
			-2.3249818930984,
			-0.88151554664667,
			0.37248229019925,
			2.5623975170135,
			1.7376861249738,
			-2.3128129091639,
			2.5466980251701,
			-2.454436680048,
			-1.119887803737,
			1.7226756786568,
			1.9503617915094,
			-2.239755268786,
			-0.92124274462519,
			-1.6845203808902,
			-1.9947193367382,
			1.7813923465001
		};

		for (int i = 0; i < 20; i++){
			MV->translate(glm::vec3(20*snowflakes[i], snowFall, 9*depth[i]));
			glUniformMatrix4fv(snowProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(snowProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(snowProg);	
		}
		MV->popMatrix();
		snowProg -> unbind();

		// *** Draw Yellow Features of Ducky *** //
		torsoProg->bind();

		// Draw head
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.25, 0.4, 0));
			MV->scale(0.2);
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw belly
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(0.4);
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();
		
		// Draw front wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0.3));
			MV->rotate(-35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw back wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, -0.3));
			MV->rotate(35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw Tail
		MV->pushMatrix();
			MV->translate(glm::vec3(0.2, 0.15, 0));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(glm::vec3(0.3, 0.22, 0.1));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		torsoProg->unbind();

		// Draw eyes using a cylinder
		glBindVertexArray(VertexArrayID);
		cylProg->bind();
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.25, 0.45, 0));
			MV->scale(glm::vec3(0.075, 0.1, 0.20));
			glUniformMatrix4fv(cylProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(cylProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glDrawArrays(GL_TRIANGLES, 0, 1440);
		MV->popMatrix();
		cylProg->unbind();

		// *** Draw Peach Features of Ducky *** //
		legProg->bind();

		// Draw top of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.4, 0.37, 0));
			MV->scale(glm::vec3(0.25, 0.04, 0.12));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw bottom of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.38, 0.35, 0));
			MV->rotate(0.1, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.04, 0.10));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Move whole front leg
		MV->rotate(frontLegRotation, glm::vec3(0, 0, 1));

		// Draw front thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, 0.2));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw front knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, 0.2));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw front calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, 0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw front foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, 0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Move whole back leg
		MV->rotate(-2*frontLegRotation, glm::vec3(0, 0, 1));

		// Draw back thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, -0.2));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw back knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, -0.2));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw back calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, -0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		// Draw back foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, -0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			glUniformMatrix4fv(torsoProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(torsoProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(torsoProg);
		MV->popMatrix();

		legProg->unbind();

		// Unbind vertex array
		glBindVertexArray(0);

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
		// Update Leg Rotations:
		frontLegRotation += incLeg(frontLegRotation, &frontLegForward);
		// Update snowfall rate:
		snowFall += incSnow(&snowFall);
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
