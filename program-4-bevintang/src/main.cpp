/*
Base code
Currently will make 2 FBOs and textures (only uses one in base code)
and writes out frame as a .png (Texture_output.png)

Winter 2017 - ZJW (Piddington texture write)
2017 integration with pitch and yaw camera lab (set up for texture mapping lab)
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:
	const float PI = 3.14159;

	// Public variables
	float theta = 0;	// yaw control
	float phi = 0;		// pitch control
	float radius = 1;
	float x,y,z;		// center coordiantes

	// Camera
	vec3 center;
	vec3 up = vec3(0, 1, 0);
	vec3 eye = vec3(0, 0.5, 0);
	vec3 forward;
	const vec3 speed = vec3(0.15, 0.15, 0.15);	// speed relative to max step

	// Cursor
	int lastX = 0;
	int lastY = 0;

	// Duck animation
	float frontLegRotation = 0;
	bool frontLegForward = false;
	float runRotation = 0;

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;
	std::shared_ptr<Program> torsoProg;
	std::shared_ptr<Program> legProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> floorShape;
	shared_ptr<Shape> sphereShape;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	bool FirstTime = true;
	bool Moving = false;
	int gMat = 0;

	bool mouseDown = false;

	float incLeg(float count, bool* legForward){
		if (count < -1){
			*legForward = false;
		}
		else if (count > 1){
			*legForward = true;
		}

		if (*legForward)
			return -0.01;
		else
			return 0.01;
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_M && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			gMat = (gMat + 1) % 4;
		}

		// Movement control
		else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			// Move forward
			eye += speed * forward;
			if (eye.y < 0)
				eye.y = 0;
			center += speed * forward;
		}
		else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			// Move backward
			eye -= speed * forward;
			if (eye.y < 0)
				eye.y = 0;
			center -= speed * forward;
		}
		else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			// Move left
			eye += speed * cross(up, forward);
			center += speed * cross(up, forward);
		}
		else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			// Move left
			eye -= speed * cross(up, forward);
			center -= speed * cross(up, forward);
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		double deltaX = xpos - lastX;	// difference of lastX and curX
		double deltaY = ypos - lastY;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// Update yaw and pitch
		theta += deltaX / 500;
		phi += -deltaY / 500;

		// Restrict pitch to 180 deg of view
		if (phi > 1.5)	// 1.5 rad ~ 90 degrees
			phi = 1.5;
		else if (phi < -1.5)
			phi = -1.5;

		// update lastX and lastY
		lastX += deltaX;
		lastY += deltaY;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
			Moving = true;
		}

		if (action == GLFW_RELEASE)
		{
			Moving = false;
			mouseDown = false;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glfwGetFramebufferSize(windowManager->getHandle(), &lastX, &lastY);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.50f, 0.8f, 0.95f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

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
		prog->addUniform("view");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		//create two frame buffer objects to toggle between
		glGenFramebuffers(2, frameBuf);
		glGenTextures(2, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

		//more FBO set up
		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers);

		//create another FBO so we can swap back and forth
		createFBO(frameBuf[1], texBuf[1]);
		//this one doesn't need depth

		//set up the shaders to blur the FBO just a placeholder pass thru now
		//next lab modify and possibly add other shaders to complete blur
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(
			resourceDirectory + "/pass_vert.glsl",
			resourceDirectory + "/tex_fragH.glsl");
		if (! texProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		texProg->addUniform("P");
		texProg->addUniform("MV");
		texProg->addUniform("view");
		texProg->addUniform("texBuf");
		texProg->addUniform("pov");
		texProg->addAttribute("vertPos");
	 }

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize the obj mesh VBOs etc
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/bunny.obj");
		shape->resize();
		shape->init();

		// Initialize Floor mesh
		floorShape = make_shared<Shape>();
		floorShape->loadMesh(resourceDirectory + "/cube.obj");
		floorShape->resize();
		floorShape->init();

		// Initialize sphere mesh
		sphereShape = make_shared<Shape>();
		sphereShape->loadMesh(resourceDirectory + "/sphere.obj");
		sphereShape->resize();
		sphereShape->init();

		//Initialize the geometry to render a quad to the screen
		initQuad();
	}

	/**** geometry set up for a quad *****/
	void initQuad()
	{
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	}

	/* Helper function to create the framebuffer object and
		associated texture to write to */
	void createFBO(GLuint& fb, GLuint& tex)
	{
		//initialize FBO
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}

	void renderMirror(GLuint inTex, shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV, int pov)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inTex);

		// example applying of 'drawing' the FBO texture - change shaders
		texProg->bind();
			glUniform1i(texProg->getUniform("texBuf"), 0);
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(texProg->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
			glUniformMatrix4fv(texProg->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eye, center, up)));
			glUniform1f(texProg->getUniform("pov"), pov);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
		texProg->unbind();
	}

	void drawDuck(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV, vec3 eyePos){

		// Draw head
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.25, 0.4, 0));
			MV->scale(0.2);
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw belly
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(0.4);
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();
		
		// Draw front wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0.3));
			MV->rotate(-35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, -0.3));
			MV->rotate(35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw Tail
		MV->pushMatrix();
			MV->translate(glm::vec3(0.2, 0.15, 0));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(glm::vec3(0.3, 0.22, 0.1));
			SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw top of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.4, 0.37, 0));
			MV->scale(glm::vec3(0.25, 0.04, 0.12));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 1000, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw bottom of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.38, 0.35, 0));
			MV->rotate(0.1, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.04, 0.10));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Move whole front leg
		MV->rotate(frontLegRotation, glm::vec3(0, 0, 1));

		// Draw front thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, 0.2));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, 0.2));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, 0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, 0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Move whole back leg
		MV->rotate(-2*frontLegRotation, glm::vec3(0, 0, 1));

		// Draw back thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, -0.2));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, -0.2));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, -0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, -0.2));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 100, 100);
			glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
			sphereShape->draw(prog);
		MV->popMatrix();
	}

	void renderScene(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV, int pov){
		vec3 eyePos;

		if (pov == 0)
			eyePos = eye;
		else
			eyePos = vec3(0, 2, -10);

		//Draw our scene - two meshes - right now to a texture
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		// globl transforms for 'camera' (you will fix this now!)
		MV->pushMatrix();
			MV->loadIdentity();
			// Increment duck movement variables
			frontLegRotation += incLeg(frontLegRotation, &frontLegForward);
			runRotation -= 0.005;

			float dx, dz, dTheta = 0;
			for (int i = 0; i < 20; i++)
			{
				dx = (4.5f) * sin(dTheta);
				dz = (4.5f) * cos(dTheta);
				/* draw left mesh */
				MV->pushMatrix();
				MV->rotate(runRotation, vec3(0, 1 ,0));
				MV->translate(vec3(dx, 0.f, dz));
				MV->rotate(-0.5 + dTheta + frontLegRotation/10, vec3(0, 1, 0));
				drawDuck(P, MV, eyePos);
				MV->popMatrix();
				dTheta += 6.28f / 20.f;
			}

			// Draw bunnies
			MV->loadIdentity();
			float tx, tz, theta = 0;
			for (int i = 0; i < 10; i++)
			{
				tx = (2.f) * sin(theta);
				tz = (2.f) * cos(theta);
				MV->pushMatrix();
				MV->rotate(-runRotation, vec3(0, 1 ,0));
				MV->translate(vec3(tx, 0.f, tz));
				MV->rotate(3.14f + theta, vec3(0, 1, 0));
				MV->scale(0.6);
				SetMaterial(i % 4);
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
				glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
				glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
				glUniform3f(prog->getUniform("eye"), eye.x, eye.y, eye.z);
				shape->draw(prog);
				MV->popMatrix();
				theta += 6.28f / 10.f;
			}

			// Draw floor
			MV->pushMatrix();
			MV->loadIdentity();
				MV->translate(vec3(0, -20.6, 0));
				MV->scale(vec3(20, 20, 20));
				glUniform3f(prog->getUniform("AMBIENCE"), 0.0f, 0.05f, 0.0f);
				glUniform3f(prog->getUniform("DIFFUSE"), 0.0f, 0.3f, 0.0f);
				glUniform3f(prog->getUniform("SPECULAR"), 0.3, 0.7, 0.0);
			    glUniform1f(prog->getUniform("SHINE"), 100.0);
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
				glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
				glUniform3f(prog->getUniform("uLight"), 0, 100, 0);
				glUniform3f(prog->getUniform("eye"), 0, 10, 0);
				floorShape->draw(prog);
			MV->popMatrix();
		MV->popMatrix();

		prog->unbind();
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		if (Moving)
		{
			//set up to render to buffer
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width/(float)height;

		// Setup yaw and pitch of camera for lookAt()
		x = radius*cos(phi)*cos(theta);
		y = radius*sin(phi);
		z = radius*cos(phi)*sin(theta);
		center = eye + vec3(x, y, z);
		forward = normalize(center - eye);

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto MV = make_shared<MatrixStack>();

		// Apply perspective projection
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		// Draw screen from static camera POV
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderScene(P, MV, 1);

		// Ready frame buffer and texture
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[1]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderMirror(texBuf[0], P, MV, 1);

		// Draw whole scene with mirror	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		MV->pushMatrix();
			// ** DRAW MONITOR ** //
			MV->pushMatrix();
				MV->translate(vec3(0, 1.5, -1.5));
				MV->rotate(.6, vec3(1, 0, 0));
				MV->scale(vec3(2, 1, 1));
				renderMirror(texBuf[0], P, MV, 1);	
			MV->popMatrix();
			// ** DRAW THE REST ** //
			MV->pushMatrix();
				renderScene(P, MV, 0);
			MV->popMatrix();
		MV->popMatrix();

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
			glUniform3f(prog->getUniform("AMBIENCE"), 0.6, 0.5, 0.0);
		    glUniform3f(prog->getUniform("DIFFUSE"), 0.7, 0.7, 0.0);
		    glUniform3f(prog->getUniform("SPECULAR"), 0.8, 0.8, 0.0);
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
	windowManager->init(1440, 900);
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
