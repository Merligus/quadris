#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_s.h>
#include "pieces.h"
#include "grid.h"

#include <iostream>
#include <list>

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
int initConfig(GLFWwindow *w);
void initVertexArray(unsigned int *B, unsigned int *A);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Quadris", NULL, NULL);
	if(initConfig(window) != 0)
		return -1;
	glfwMaximizeWindow(window);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader shader("transform.vs", "transform.fs");

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	shader.use();

	unsigned int VBO, VAO;
	initVertexArray(&VBO, &VAO);

	// create transformations
	glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -26.0f));
	// pass transformation matrices to the shader
	shader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
	shader.setMat4("view", view);

	Grid g(shader);
	Piece p((Piece::types::I), shader);
	bool key_a_release, key_d_release, key_s_release, key_q_release, key_e_release;
	key_a_release = key_d_release = key_s_release = key_q_release = key_e_release = true;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		
		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
			key_a_release = true;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
			key_d_release = true;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
			key_s_release = true;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
			key_q_release = true;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
			key_e_release = true;

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && key_a_release)
		{
			p.translate(glm::vec3(-1.0f, 0.0f, 0.0f));
			key_a_release = false;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && key_d_release)
		{
			p.translate(glm::vec3(1.0f, 0.0f, 0.0f));
			key_d_release = false;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && key_s_release)
		{
			p.translate(glm::vec3(0.0f, -1.0f, 0.0f));
			key_s_release = false;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && key_q_release)
		{
			p.rotate(glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);
			key_q_release = false;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && key_e_release)
		{
			p.rotate(glm::vec3(0.0f, 0.0f, 1.0f), -90.0f);
			key_e_release = false;
		}

		// render boxes
		glBindVertexArray(VAO);
		g.draw(shader);
		p.draw(shader);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

int initConfig(GLFWwindow *w)
{
	// glfw window creation
	// --------------------
	if (w == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(w);
	glfwSetFramebufferSizeCallback(w, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	return 0;
}

void initVertexArray(unsigned int *B, unsigned int *A)
{
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float square[] = {
		// positions          // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // top left 
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f // top right
	};

	glGenVertexArrays(1, A);
	glGenBuffers(1, B);

	glBindVertexArray(*A);

	glBindBuffer(GL_ARRAY_BUFFER, *B);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}