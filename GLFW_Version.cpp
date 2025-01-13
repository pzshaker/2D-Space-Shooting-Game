#include <glm/ext.hpp>
#include<iostream>
#include "Shader.h"
#include "Model.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Shader ourShader;

// spaceship
Model spaceship;
glm::vec3 spaceshipPos = glm::vec3(0.0f, -1.5f, -5.0f);
float spaceshipSpeed = 1.5f;

// bullet
std::vector<glm::vec3> bulletsPositions;
float bulletSpeed = 3.0f;
Model bulletModel;

// enemySpaceship
std::vector<glm::vec3> enemySpaceshipPositions;
Model enemySpaceship;
float enemySpaceshipSpeed = 0.5f;

// heart
Model heart;
std::vector<glm::vec3> heartPositions;
glm::vec3 heartPos = glm::vec3(2.4f, 1.0f, -5.0f);

// Score
int score = 0;

void renderSpaceShip() {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, spaceshipPos);
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	ourShader.setMat4("model", model);
	spaceship.Draw(ourShader);
}

void renderBullets() {
	for (glm::vec3 position : bulletsPositions) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::scale(model, glm::vec3(0.0005f, 0.0005f, 0.0005f));
		ourShader.setMat4("model", model);

		bulletModel.Draw(ourShader);
	}
}

void updateBullets() {
	for (int i = 0; i < bulletsPositions.size(); i++) {
		bulletsPositions[i].y += bulletSpeed * deltaTime;
		if (bulletsPositions[i].y > 1.6f) {
			bulletsPositions.erase(bulletsPositions.begin() + i);
			i--;
		}
	}
}

void renderEnemySpaceShip() {
	for (int i = 0; i < enemySpaceshipPositions.size(); i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, enemySpaceshipPositions[i]);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		ourShader.setMat4("model", model);

		enemySpaceship.Draw(ourShader);
	}
}

void addEnemySpaceShip(float xPos) {
	float yPos = 1.6f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (3.0f - 1.6f)));
	enemySpaceshipPositions.push_back(glm::vec3(xPos, yPos, -5.0f));
}

void updateEnemySpaceships() {
	for (int i = 0; i < enemySpaceshipPositions.size(); i++) {
		enemySpaceshipPositions[i].y -= enemySpaceshipSpeed * deltaTime;

		if (enemySpaceshipPositions[i].y < -1.7f) {
			addEnemySpaceShip(enemySpaceshipPositions[i].x);
			enemySpaceshipPositions.erase(enemySpaceshipPositions.begin() + i);
		}
	}
}

void checkSpaceshipCollision() {
	int heartIndex = 0;
	for (int i = 0; i < enemySpaceshipPositions.size(); i++) {
		float distance = glm::distance(spaceshipPos, enemySpaceshipPositions[i]);
		if (distance < 0.4f) {
			cout << "Player's spaceship destroyed at position: "
				<< "(" << spaceshipPos.x << ", " << spaceshipPos.y << ")" << endl;

			if (!heartPositions.empty())
			{
				enemySpaceshipPositions.erase(enemySpaceshipPositions.begin() + i);
				heartPositions.erase(heartPositions.begin() + heartIndex);
				cout << "one heart has been removed." << endl;
			}

			if (heartPositions.empty())
			{
				cout << "GAME OVER!" << endl;
				cout << "Score: " << score << endl;

				glfwTerminate();
				exit(0);
			}
		}
	}
	heartIndex++;
}

void checkBulletCollision() {
	for (int i = 0; i < enemySpaceshipPositions.size(); i++) {
		for (int j = 0; j < bulletsPositions.size(); j++) {
			float distance = glm::distance(enemySpaceshipPositions[i], bulletsPositions[j]);

			if (distance < 0.2f) {
				cout << "Enemy spaceship destroyed by Bullet at position: "
					<< "(" << enemySpaceshipPositions[i].x << ", " << enemySpaceshipPositions[i].y << ")" << endl;

				addEnemySpaceShip(enemySpaceshipPositions[i].x);
				enemySpaceshipPositions.erase(enemySpaceshipPositions.begin() + i);
				bulletsPositions.erase(bulletsPositions.begin() + j);
				score += 10;
				break;
			}
		}
	}
}

void initEnemySpaceships() {
	srand(time(0));
	float xPos = -2.4f;
	for (int i = 0; i < 7; i++) {
		float yPos = 1.6f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (3.0f - 1.6f)));
		enemySpaceshipPositions.push_back(glm::vec3(xPos, yPos, -5.0f));
		xPos += 0.8f;
	}
}

void renderHearts() {
	for (glm::vec3 position : heartPositions) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
		ourShader.setMat4("model", model);

		heart.Draw(ourShader);
	}
}

void initHearts() {
	float yPos = 1.0f;
	for (int i = 0; i < 3; i++) {
		heartPositions.push_back(glm::vec3(2.4f, yPos, -5.0f));
		yPos += 0.3f;
	}
}

void init()
{
	ourShader.use();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	ourShader.setMat4("projection", projection);
	initEnemySpaceships();
	initHearts();
}

void display(void)
{
	renderSpaceShip();
	renderBullets();
	updateBullets();
	renderEnemySpaceShip();
	updateEnemySpaceships();
	checkSpaceshipCollision();
	checkBulletCollision();
	renderHearts();
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;

	}
	std::cerr << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;

	// Initialize shader and model (ONE TIME ONLY)
	ourShader = Shader("vshader.glsl", "fshader.glsl");
	spaceship = Model("spaceship/78115.obj");
	bulletModel = Model("bullet/Star.fbx");
	enemySpaceship = Model("enemy_spaceship/neghvar.obj");
	heart = Model("heart/Heart box.obj");

	init();
	glfwSetTime(0.0f);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Process user input
		processInput(window);

		// Rendering logic
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		display();

		// Swap front and back buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Define the boundaries
	const float leftLimit = -2.4f;
	const float rightLimit = 2.4f;
	const float topLimit = 1.6f;
	const float bottomLimit = -1.6f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && spaceshipPos.y < topLimit)
		spaceshipPos.y += spaceshipSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && spaceshipPos.y > bottomLimit)
		spaceshipPos.y -= spaceshipSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && spaceshipPos.x > leftLimit)
		spaceshipPos.x -= spaceshipSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && spaceshipPos.x < rightLimit)
		spaceshipPos.x += spaceshipSpeed * deltaTime;

	// Add a new bullet when spacebar is pressed
	static bool spacePressedLastFrame = false;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressedLastFrame) {
		bulletsPositions.push_back(spaceshipPos);
	}
	spacePressedLastFrame = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
