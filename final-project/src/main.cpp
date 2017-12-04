/* Lab 6 base code - transforms using matrix stack built on glm
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <cmath>
#include <cstdlib>
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
	std::shared_ptr<Program> cubeProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> sphereShape;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	// Phong vectors
	vec3 eyePos = vec3(0, 0, 0);
	vec3 center = vec3(0, 0, -1);
	vec3 up = vec3(0, 1, 0);

	// Duck animation
	float frontLegRotation = 0;
	bool frontLegForward = false;
	float duckHeight = -.7;
	bool legMoving = true;

	//  ** Model control ** //
	float cubePos = -15;
	float duckPos = 0;
	float duckAngle = -1.57;
	int leftType = randType();
	int middleType = randType();
	int rightType = randType();
	bool inShadow = false;
	const int CUBE = 6969;
	const int LINE = 9696;
	const int PIT = 6966;
	const float LEFT = -0.55;
	const float RIGHT = 0.55;
	const float MIDDLE = 0;
	const float STANDING = -.7;
	const float SLIDING = -.975;

	// Jump control
	float jumpHeight = 0;
	float apexFrames = 0;
	bool jumping = false;
	bool falling = false;

	// Game details
	int score = -1;
	int milestone = 5;
	float speed = 0.10;
	int startFrames = 0;
	int deathFrames = 0;
	int playTime = 0;
	int arrowTime = 0;
	bool gameover = false;
	bool speedingUp = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !jumping && !gameover)
		{
			if (duckHeight >= SLIDING){
				duckHeight = STANDING;
				jumping = true;
			}
			frontLegRotation = 1.5;
			legMoving = false;
		}
		else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			legMoving = true;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && !gameover)
		{
			duckPos += LEFT;
			if (duckPos < LEFT)
				duckPos = LEFT;
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && !gameover)
		{
			duckPos += RIGHT;
			if (duckPos > RIGHT)
				duckPos = RIGHT;
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			duckHeight = STANDING;
			legMoving = true;
		}
		else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT) && !jumping && !gameover) 
		{
			duckHeight = SLIDING;
			frontLegRotation = 1.5;
			legMoving = false;
		}

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			gameover = false;
			score = -1;
			startFrames = 0;
			deathFrames = 0;
			playTime = 0;
			arrowTime = 0;
			cubePos = -15;
			duckPos = 0;
			duckHeight = STANDING;
			jumpHeight = 0;
			legMoving = true;
			jumping = false;
			falling = false;
			speed = 0.1;
			speedingUp = false;
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
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);


		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/phong_vert33.glsl", resourceDirectory + "/phong_frag33.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addUniform("view");
		prog->addUniform("uLight");
		prog->addUniform("eye");
		prog->addUniform("AMBIENCE");
		prog->addUniform("DIFFUSE");
		prog->addUniform("SPECULAR");
		prog->addUniform("SHINE");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("MV");
		cubeProg->addUniform("uColor");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/cube.obj");
		shape->resize();
		shape->init();

		sphereShape = make_shared<Shape>();
		sphereShape->loadMesh(resourceDirectory + "/sphere.obj");
		sphereShape->resize();
		sphereShape->init();
	}

	void drawDigit(shared_ptr<MatrixStack>& P, shared_ptr<MatrixStack>& MV, int digit) {
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		if (digit == 0 || digit == 2 || digit == 3 || digit == 5 || digit == 6 || digit == 7 || digit == 8 || digit == 9) {
			//segment 1
			MV->pushMatrix();
			MV->translate(vec3(0, 0, -10));
			MV->scale(vec3(0.2, 0.05, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 0 || digit == 4 || digit == 5 || digit == 6 || digit == 8 || digit == 9) {
			//segment 2
			MV->pushMatrix();
			MV->translate(vec3(-0.15, -0.15, -10));
			MV->scale(vec3(0.05, 0.2, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 0 || digit == 1 || digit == 2 || digit == 3 || digit == 4 || digit == 7 || digit == 8 || digit == 9) {
			//segment 3
			MV->pushMatrix();
			MV->translate(vec3(0.15, -0.15, -10));
			MV->scale(vec3(0.05, 0.2, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 2 || digit == 3 || digit == 4 || digit == 5 || digit== 6 || digit == 8 || digit == 9) {
			//segment 4
			MV->pushMatrix();
			MV->translate(vec3(0, -0.3, -10));
			MV->scale(vec3(0.2, 0.05, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 0 || digit == 2 || digit == 6 || digit == 8) {
			//segment 5
			MV->pushMatrix();
			MV->translate(vec3(-0.15, -0.45, -10));
			MV->scale(vec3(0.05, 0.2, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 0 || digit == 1 || digit == 3 || digit == 4 || digit == 5 || digit == 6 || digit == 7 || digit == 8 || digit == 9) {
			//segment 6
			MV->pushMatrix();
			MV->translate(vec3(0.15, -0.45, -10));
			MV->scale(vec3(0.05, 0.2, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}

		if (digit == 0 || digit == 2 || digit == 3 || digit == 5 || digit == 6 || digit == 8) {
			//segment 7
			MV->pushMatrix();
			MV->translate(vec3(0, -0.6, -10));
			MV->scale(vec3(0.2, 0.05, 0.005));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 0);
			shape->draw(cubeProg);
			MV->popMatrix();
		}
	}

	void drawScore(shared_ptr<MatrixStack>& P, shared_ptr<MatrixStack>& MV) {
		int digitOnes = score % 10;
		int digitTens = (score % 100) / 10;
		int digitHundreds = (score % 1000) / 100;

		if (score < 10) {
			MV->pushMatrix();
			MV->translate(vec3(-2.25, 2.3, 0));
			drawDigit(P, MV, digitOnes);
			MV->popMatrix();
		}
		else if (score < 100) {
			MV->pushMatrix();
			MV->translate(vec3(-2.75, 2.3, 0));
			drawDigit(P, MV, digitTens);
			MV->popMatrix();

			MV->pushMatrix();
			MV->translate(vec3(-2.25, 2.3, 0));
			drawDigit(P, MV, digitOnes);
			MV->popMatrix();
		}
		else {
			MV->pushMatrix();
			MV->translate(vec3(-3.25, 2.3, 0));
			drawDigit(P, MV, digitHundreds);
			MV->popMatrix();

			MV->pushMatrix();
			MV->translate(vec3(-2.75, 2.3, 0));
			drawDigit(P, MV, digitTens);
			MV->popMatrix();

			MV->pushMatrix();
			MV->translate(vec3(-2.25, 2.3, 0));
			drawDigit(P, MV, digitOnes);
			MV->popMatrix();
		}
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
		case 1: // shadowy feathers
			glUniform3f(prog->getUniform("AMBIENCE"), 0.1, 0.1, 0.0);
		    glUniform3f(prog->getUniform("DIFFUSE"), 0.2, 0.2, 0.0);
		    glUniform3f(prog->getUniform("SPECULAR"), 0.4, 0.4, 0.0);
		    glUniform1f(prog->getUniform("SHINE"), 0.0);
			break;
		case 2: // shadowy flesh
			glUniform3f(prog->getUniform("AMBIENCE"), 0.1, 0.05, 0.05);
		    glUniform3f(prog->getUniform("DIFFUSE"), 0.2, 0.1, 0.05);
		    glUniform3f(prog->getUniform("SPECULAR"), 0.3, 0.15, 0.1);
		    glUniform1f(prog->getUniform("SHINE"), 0);
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
		    glUniform1f(prog->getUniform("SHINE"), 300);
			break;
		}
	}

	float incLeg(float count, bool* legForward){
		if (count < -1.5){
			*legForward = false;
		}
		else if (count > 1.5){
			*legForward = true;
		}

		if (*legForward && legMoving)
			return -0.25;
		else if (*legForward == false && legMoving)
			return 0.25;
		else
			return 0;
	}

	void incJumpHeight(){
		if (jumping && !falling){
			jumpHeight += 0.15;
			if (jumpHeight > 1.9){
				apexFrames += 1;
				jumpHeight -= 0.12;
			}
			if (apexFrames >= 15 - 15*speed){
				legMoving = false;
				apexFrames = 0;
				falling = true;
			}
		}

		else if (jumping && falling){
			jumpHeight -= 0.1;
			if (jumpHeight <= 0){
				jumping = false;
				falling = false;
				legMoving = true;
			}
		}
	}

	void drawDuck(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV, vec3 eyePos){
		// Increment duck movement variables
		frontLegRotation += incLeg(frontLegRotation, &frontLegForward);

		// Whole duck control
		incJumpHeight();
		MV->translate(glm::vec3(0, jumpHeight, 0));
		MV->rotate(-0.3, glm::vec3(0, 0, 1));

		// Draw head
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.25, 0.4, 0));
			MV->scale(0.2);
			if (inShadow)
				SetMaterial(1);
			else
				SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 3, 0);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw belly
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(0.4);
			if (inShadow)
				SetMaterial(1);
			else
				SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 3, 0);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();
		
		// Draw front wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, 0.3));
			MV->rotate(-35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			if (inShadow)
				SetMaterial(1);
			else
				SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 3, 0);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back wing
		MV->pushMatrix();
			MV->translate(glm::vec3(0, 0.1, -0.3));
			MV->rotate(35, glm::vec3(0, 1, 0));
			MV->scale(glm::vec3(1, 0.5, 0.25));
			MV->scale(0.2);
			if (inShadow)
				SetMaterial(1);
			else
				SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 3, 0);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw Tail
		MV->pushMatrix();
			MV->translate(glm::vec3(0.2, 0.15, 0));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(1, 0.75, 0.75));
			MV->scale(glm::vec3(0.3, 0.22, 0.1));
			if (inShadow)
				SetMaterial(1);
			else
				SetMaterial(3);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), 0, 3, 0);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw top of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.4, 0.37, 0));
			MV->scale(glm::vec3(0.25, 0.04, 0.12));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw bottom of bill
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.38, 0.35, 0));
			MV->rotate(0.1, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.04, 0.10));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Move whole front leg
		MV->rotate(frontLegRotation, glm::vec3(0, 0, 1));

		// Draw front thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, 0.15));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, 0.15));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, 0.15));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw front foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, 0.15));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Move whole back leg
		MV->rotate(-2*frontLegRotation, glm::vec3(0, 0, 1));

		// Draw back thigh
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.05, -0.2, -0.15));
			MV->rotate(45, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.22, 0.12, 0.1));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back knee
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.16, -0.36, -0.15));
			MV->scale(glm::vec3(0.05, 0.075, 0.05));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back calf
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.09, -0.42, -0.15));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.075, 0.15, 0.075));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();

		// Draw back foot
		MV->pushMatrix();
			MV->translate(glm::vec3(-0.075, -0.54, -0.15));
			MV->rotate(10, glm::vec3(0, 0, 1));
			MV->scale(glm::vec3(0.12, 0.05, 0.055));
			if (inShadow)
				SetMaterial(2);
			else
				SetMaterial(4);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE,value_ptr(lookAt(eyePos, center, up)));
			glUniform3f(prog->getUniform("uLight"), -1, 100, 1);
			glUniform3f(prog->getUniform("eye"), 0, 0, 0);
			sphereShape->draw(prog);
		MV->popMatrix();
	}

	bool crashed(int cubeType, float side){
		// Standing collision
		if (duckHeight == STANDING && duckPos == side && !jumping && (cubePos > -2.4 && cubePos < -2.2)){
			return true;
		}
		// Sliding collision
		else if (duckHeight == SLIDING && duckPos == side && !jumping && (cubePos > -2.4 && cubePos < -2.2) && cubeType == CUBE)
			return true;
		// Jumping collision
		else if (duckPos == side && jumping && abs(jumpHeight - (-(10 + cubePos)/10)) < 2.4 && (cubePos > -2.4 && cubePos < -2.3)){
			return true;
		}

		return false;
	}

	void drawObstacle(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV, int cubeType, float side){
		MV->pushMatrix();
		MV->loadIdentity();
			MV->rotate(.1, vec3(1, 0, 0));
			MV->translate(vec3(side, -(10 + cubePos)/10, cubePos));
			if (cubeType == LINE){
				MV->translate(vec3(0, .3, 0));
			}
			MV->scale(vec3(.25, .35, .1));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), .58, .1, 0.05);
			shape->draw(cubeProg);
		MV->popMatrix();

		// draw shadow for LINE type
		if (cubeType == LINE){
			MV->pushMatrix();
				MV->rotate(.1, vec3(1, 0, 0));
				MV->translate(vec3(side, -(10 + cubePos)/10 - .4, cubePos + 2));
				MV->rotate(-.1, vec3(1, 0, 0));
				MV->scale(vec3(.25, .25, .1));
				glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				glUniform3f(cubeProg->getUniform("uColor"), 0, .1, 0);
				shape->draw(cubeProg);
			MV->popMatrix();

			// Set duck shadows
			if (duckHeight == SLIDING && duckPos == side && (cubePos > -2.6 && cubePos < -2))
				inShadow = true;
			else
				inShadow = false;
		}

		// Check collision
		if (crashed(cubeType, side)){
			gameover = true;
			deathFrames += 1;
			MV->pushMatrix();
				MV->rotate(.1, vec3(1, 0, 0));
				MV->translate(vec3(duckPos, -1.2, 0));
				MV->scale(.25);
				glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				glUniform3f(cubeProg->getUniform("uColor"), .12, .34, .56);
				shape->draw(cubeProg);
			MV->popMatrix();
		} 
		
	}

	void drawArrows(shared_ptr<MatrixStack> &P, shared_ptr<MatrixStack> &MV){
		// Draw top arrows
		MV->pushMatrix();
			MV->rotate(.15, vec3(1, 0, 0));
			MV->translate(vec3(.28, 1, cubePos + 5));
			MV->rotate(-.3, vec3(0, 0, 1));
			MV->scale(vec3(.3, .03, .01));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 1);
			shape->draw(cubeProg);
		MV->popMatrix();
		MV->pushMatrix();
			MV->rotate(.15, vec3(1, 0, 0));
			MV->translate(vec3(-.28, 1, cubePos + 5));
			MV->rotate(.3, vec3(0, 0, 1));
			MV->scale(vec3(.3, .03, .01));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 1);
			shape->draw(cubeProg);
		MV->popMatrix();

		//Draw bottom arrows
		MV->pushMatrix();
			MV->rotate(.15, vec3(1, 0, 0));
			MV->translate(vec3(.32, .85, cubePos + 5));
			MV->rotate(-.3, vec3(0, 0, 1));
			MV->scale(vec3(.35, .04, .01));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 1);
			shape->draw(cubeProg);
		MV->popMatrix();
		MV->pushMatrix();
			MV->rotate(.15, vec3(1, 0, 0));
			MV->translate(vec3(-.32, .85, cubePos + 5));
			MV->rotate(.3, vec3(0, 0, 1));
			MV->scale(vec3(.35, .04, .01));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 1, 1, 1);
			shape->draw(cubeProg);
		MV->popMatrix();
	}

	int randType(){
		int type = rand() % 2;

		if (type == 1)
			return CUBE;
		else
			return LINE;
	}

	void render()
	{
		// Increment obstacle movement
		if (!gameover)
			cubePos += speed;
		if (cubePos > 0){
			cubePos = -11;
			score += 1;
			speedingUp = false;
			leftType = randType();
			middleType = randType();
			rightType = randType();
		}

		// Increase speed every 10 points
		if (score == milestone){
			speed += .025;
			milestone += 5;
			speedingUp = true;
			arrowTime = 0;
		}

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

		// Draw Ground
		cubeProg->bind();
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		// Translate the whole scene
		MV->loadIdentity();
		MV->translate(vec3(0, 0, -2));

		// draw floor cube
		MV->pushMatrix();
			MV->translate(vec3(0, -2, 0));
			MV->rotate(.2, vec3(1, 0, 0));
			MV->scale(vec3(1, 1, 15));
			glUniformMatrix4fv(cubeProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glUniform3f(cubeProg->getUniform("uColor"), 0, .3, .02);
			shape->draw(cubeProg);		
		MV->popMatrix();

		// Draw obstacles
		startFrames += 1;
		if (startFrames > 150){
			drawObstacle(P, MV, leftType, LEFT);
			drawObstacle(P, MV, middleType, MIDDLE);			
			drawObstacle(P, MV, rightType, RIGHT);
		}

		// Draw scoreboard
		MV->pushMatrix();
			MV->translate(vec3(5, 1.15, 5));
			drawScore(P, MV);
		MV->popMatrix();

		// Draw "speed up" arrows
		if (speedingUp){
			drawArrows(P, MV);
			arrowTime += 1;
		}
		
		cubeProg->unbind();

		// Draw duck
		prog->bind();
		MV->pushMatrix();
			if (!gameover){
				duckAngle = -1.57;
			}
			else{
				jumping = false;
				duckAngle = 0;
				if (deathFrames > 20)
					duckHeight -= 0.1;
			}
			MV->translate(vec3(duckPos, duckHeight, 0));
			MV->rotate(duckAngle, vec3(0, 1, 0));
			MV->scale(0.5);
			drawDuck(P, MV, eyePos);
		MV->popMatrix();
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
	windowManager->init(640, 768);
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
