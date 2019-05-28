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
#include <vector>
#include <random>

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;
bool collapse = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Piece *p, Grid *g, double *ENDGAME);
int initConfig(GLFWwindow *w);
void initVertexArray(unsigned int *B, unsigned int *A);
int pop_bag(std::vector<int> *bag, int nth);

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

	std::random_device random_rotation, random_type;
	Grid g(shader);
	Piece *currentPiece, *nextPiece1, *nextPiece2, *nextPiece3, *nextPiece4, *nextPiece5, *nextPiece6;
	std::vector<int> bag;
	for (int i = 0; i < 7; i++)
		bag.push_back(i);
	currentPiece = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece1 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece2 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece3 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece4 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece5 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	nextPiece6 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
	
	glm::mat4 posicaoNextPiece = glm::mat4(g.model);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(15.5f, 21.0f, -10.0f));
	nextPiece1->setModel(posicaoNextPiece);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
	nextPiece2->setModel(posicaoNextPiece);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
	nextPiece3->setModel(posicaoNextPiece);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
	nextPiece4->setModel(posicaoNextPiece);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
	nextPiece5->setModel(posicaoNextPiece);
	posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
	nextPiece6->setModel(posicaoNextPiece);
	
	g.start(currentPiece);
	int time = (int)glfwGetTime();
	double ENDGAME = glfwGetTime();
	float scale = 1.0f;
	bool endgame = false;
	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		if (g.lost && endgame && glfwGetTime() - ENDGAME >= 0.8f)
		{
			std::cout << "LOSE\n";
		}
		else
		{
			// input
		// -----
			processInput(window, currentPiece, &g, &ENDGAME);

			// render
			// ------
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if ((endgame && g.change && glfwGetTime() - ENDGAME >= 0.6f) || collapse)
			{
				time = (int)(scale * glfwGetTime());
				g.change = false;
				g.fallAllTheWay();
				g.change = false;
				endgame = false;
				g.lineComplete();
				delete currentPiece;
				currentPiece = nextPiece1;
				nextPiece1 = nextPiece2;
				nextPiece2 = nextPiece3;
				nextPiece3 = nextPiece4;
				nextPiece4 = nextPiece5;
				nextPiece5 = nextPiece6;
				if (g.lose())
				{
					std::cout << "LOSE\n";
					continue;
				}
				g.start(currentPiece);
				nextPiece6 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
				glm::mat4 posicaoNextPiece = glm::mat4(g.model);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(15.5f, 21.0f, -10.0f));
				nextPiece1->setModel(posicaoNextPiece);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
				nextPiece2->setModel(posicaoNextPiece);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
				nextPiece3->setModel(posicaoNextPiece);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
				nextPiece4->setModel(posicaoNextPiece);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
				nextPiece5->setModel(posicaoNextPiece);
				posicaoNextPiece = glm::translate(posicaoNextPiece, glm::vec3(0.0f, -4.5f, 0.0f));
				nextPiece6->setModel(posicaoNextPiece);
			}

			if (!endgame && g.change)
			{
				ENDGAME = glfwGetTime();
				endgame = true;
			}
			else if (endgame && !g.change)
				endgame = false;

			if ((int)(scale * glfwGetTime()) > time)
			{
				time = (int)(scale * glfwGetTime());
				g.fall();
			}

			// render boxes
			glBindVertexArray(VAO);
			g.draw(shader);
			nextPiece1->draw(shader);
			nextPiece2->draw(shader);
			nextPiece3->draw(shader);
			nextPiece4->draw(shader);
			nextPiece5->draw(shader);
			nextPiece6->draw(shader);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (collapse)
		{
			time = (int)(scale * glfwGetTime());
			collapse = false;
		}
	}
	delete currentPiece;
	delete nextPiece1;

	system("PAUSE");
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
void processInput(GLFWwindow *window, Piece *p, Grid *g, double *ENDGAME)
{
	static bool key_a_release = true, key_d_release = true, key_i_release = true, key_o_release = true, key_p_release = true;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
		key_a_release = true;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
		key_d_release = true;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
		key_i_release = true;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
		key_o_release = true;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
		key_p_release = true;

	
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && key_a_release)
	{
		g->translate(false);
		key_a_release = false;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g->fall();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && key_d_release)
	{
		g->translate(true);
		key_d_release = false;
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && key_i_release)
	{
		key_i_release = false;
		collapse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && key_o_release)
	{
		g->rotate(true);
		*ENDGAME = glfwGetTime();
		key_o_release = false;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && key_p_release)
	{
		g->rotate(false);
		*ENDGAME = glfwGetTime();
		key_p_release = false;
	}
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

int pop_bag(std::vector<int> *bag, int nth)
{
	int chosen, index = nth % bag->size(), temp;
	chosen = temp = bag->at(index);
	bag->at(index) = bag->back();
	bag->at(bag->size() - 1) = temp;
	bag->pop_back();
	if (bag->empty())
		for (int i = 0; i < 7; i++)
			bag->push_back(i);
	return chosen;
}
