//Lab 1
//modified from http://learnopengl.com/

#include "stdafx.h"

#include "..\glew\glew.h"	// include GL Extension Wrangler
#include "..\glfw\glfw3.h"	// include GLFW helper library
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "CImg.h"
#include "Camera.h"
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#define STP 0.5f
using namespace std;


// Properties
const GLuint WIDTH = 2000, HEIGHT = 2000;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Camera
Camera  camera(glm::vec3(0.0f, 50.0f, 0.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float ry = 0.0f;
float rx = 0.0f;
int mapHeight, mapWidth;
glm::mat4 model;
// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "COMP371_A1", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	// Options, removes the mouse cursor for a more immersive experience
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// enable alpha support
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Build and compile our shader program
	// Vertex shader

	// Read the Vertex Shader code from the file
	string vertex_shader_path = "vertex.shader";
	string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, ios::in);

	if (VertexShaderStream.is_open()) {
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	string fragment_shader_path = "fragment.shader";
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);

	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory?\n", fragment_shader_path.c_str());
		getchar();
		exit(-1);
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(vertexShader, 1, &VertexSourcePointer, NULL);
	glCompileShader(vertexShader);
	// Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShader);
	// Check for compile time errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Link shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader); //free up memory
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);
	cimg_library::CImg<unsigned char> image("heightmap_big.bmp");
	cimg_library::CImgDisplay main_disp(image, "image");
	mapHeight = image.height();
	mapWidth = image.width();
	std::cout << image.height() << "   " << image.width() << "colors" << image.spectrum() << std::endl;

	std::vector<GLfloat>vertexData;
	std::vector<GLfloat>colorData;
	for (int i = 0; i < mapHeight; i++)
	{
		for (int j = 0; j < mapWidth; j++)
		{
			vertexData.push_back((GLfloat)j); // X-Axis
			vertexData.push_back((GLfloat)image(j, i, 0, 0)); // Y-Axis
			vertexData.push_back((GLfloat)i); // Z-Axis
		}
	}

	for (int i = 0; i < mapHeight; i++)
	{
		for (int j = 0; j < mapWidth; j++)
		{
			colorData.push_back((GLfloat)j); // X-Axis
			colorData.push_back((GLfloat)image(j, i, 0, 0)); // Y-Axis
			colorData.push_back((GLfloat)i); // Z-Axis
		}
	}

	vector<GLuint>vertexIndices;
	for (int i = 0; i < mapHeight - 1; i++)
	{
		for (int j = 0; j < mapWidth - 1; j++)
		{
			int index = (mapWidth * j) + i;

			// Top triangle
			vertexIndices.push_back(index);                // V0
			vertexIndices.push_back(index + mapWidth + 1); // V3
			vertexIndices.push_back(index + 1);            // V1

														   // Bottom triangle
			vertexIndices.push_back(index);                // V0
			vertexIndices.push_back(index + mapWidth);     // V2
			vertexIndices.push_back(index + mapWidth + 1); // V3
		}
	}

	// Initialize VAO, and VBOs
	GLuint VAO, VBO, VBOColor, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBOColor);
	glGenBuffers(1, &EBO);
	// Bind VAO
	glBindVertexArray(VAO);

	// Bind and implement VBO for vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), &vertexData.front(), GL_STATIC_DRAW);

	// Bind and implement EBO for indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLfloat), &vertexIndices.front(), GL_STATIC_DRAW);

	// Connecting X, Y, and Z to shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Bind Color buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBOColor);
	glBufferData(GL_ARRAY_BUFFER, colorData.size()* sizeof(GLfloat), &colorData.front(), GL_STATIC_DRAW);

	// Connecting X, Y, and Z to shader
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Unbind VBOColor
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind VAO
	glBindVertexArray(0);
	//model = glm::translate(model, glm::vec3(-mapWidth/ 2, -255/2,- mapHeight / 2));
	
	model = glm::mat4(1.0f);
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		DoMovement();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Camera and View transformation
		glm::mat4 projection;
		projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

		// Create camera transformation
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get their uniform location
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, vertexData.size());
		glDrawElements(GL_TRIANGLES, vertexIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	}

	if (keys[GLFW_KEY_A])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (keys[GLFW_KEY_UP]) {
		model = glm::translate(model, glm::vec3((mapWidth) / 2, 255 / 2, (mapHeight) / 2));
		model = glm::rotate(model, 0.025f, glm::vec3(1.0f, 0.0f, 0.f));
		model = glm::translate(model, glm::vec3(-(mapWidth) / 2, -255 / 2, -(mapHeight) / 2));
	}
	if (keys[GLFW_KEY_DOWN]) {
		model = glm::translate(model, glm::vec3((mapWidth) / 2, 255 / 2, (mapHeight) / 2));
		model = glm::rotate(model, 0.025f, glm::vec3(-1.0f, 0.0f, 0.f));
		model = glm::translate(model, glm::vec3(-(mapWidth) / 2, -255 / 2, -(mapHeight) / 2));
	}
	if (keys[GLFW_KEY_LEFT]) {
		model = glm::translate(model, glm::vec3((mapWidth) / 2, 255 / 2, (mapHeight) / 2));
		model = glm::rotate(model, 0.025f, glm::vec3(0.0f, -1.0f, 0.f));
		model = glm::translate(model, glm::vec3(-(mapWidth) / 2, -255 / 2, -(mapHeight) / 2));
	}
	if (keys[GLFW_KEY_RIGHT]) {
		model = glm::translate(model, glm::vec3((mapWidth) / 2, 255 / 2, (mapHeight) / 2));
		model = glm::rotate(model, 0.025f, glm::vec3(0.0f, 1.0f, 0.f));
		model = glm::translate(model, glm::vec3(-(mapWidth) / 2, -255 / 2, -(mapHeight) / 2));
	}
	if (keys[GLFW_KEY_TAB]) {
		model = glm::mat4(1.0f);
	}
	if (keys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (keys[GLFW_KEY_T]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (keys[GLFW_KEY_L]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}


void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
	camera.ProcessMouseScroll(yOffset);
}