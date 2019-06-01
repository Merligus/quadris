#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
int SCR_WIDTH = 1366;
int SCR_HEIGHT = 768;
bool collapse = false;
int time;
bool paused, menu, player_1, options;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, PiecePtr p, Grid *g);
int initConfig(GLFWwindow *w);
void initVertexArray(unsigned int *B, unsigned int *A);
int pop_bag(std::vector<int> *bag, int nth);
void keyInputCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
bool keyInputEvent(int key, int action, int mods);
void windowResizeCallBack(GLFWwindow* window, int width, int height);
bool windowResizeEvent(int width, int height);
void scrollCallBack(GLFWwindow* window, double xOffSet, double yOffSet);
bool scrollEvent(double xOffset, double yOffset);
void mouseButtonCallBack(GLFWwindow* window, int button, int actions, int mods);
bool mouseButtonInputEvent(int button, int actions, int mods);
void mouseMoveCallBack(GLFWwindow* window, double xPos, double yPos);
bool mouseMoveEvent(double xPos, double yPos);
void cursorEnterWindowCallBack(GLFWwindow* window, int entered);
bool cursorEnterWindowEvent(int entered);
static void ShowAppControlOverlay(bool *p_open);
static void ShowAppPointOverlay(float points);
static void ShowAppPauseOverlay(GLFWwindow* window);

int main()
{
	GLFWmonitor* monitor;
	int displayWidth;
	int displayHeight;

	glfwInit();
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw: initialize and configure
	// ------------------------------
	// Create the GLFW window.
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Quadris", NULL, NULL);
	if(initConfig(window) != 0)
		return -1;
	monitor = glfwGetPrimaryMonitor();
	if (monitor == nullptr)
		return -1;
	glfwMaximizeWindow(window);
	if (window == nullptr)
		return -1;
	glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
	glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);

	const auto v = glfwGetVideoMode(monitor);
	const auto x = (v->width - SCR_WIDTH) >> 1;
	const auto y = (v->height - SCR_HEIGHT) >> 1;

	glfwSetWindowPos(window, x, y);
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, keyInputCallBack);
	glfwSetWindowSizeCallback(window, windowResizeCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, mouseMoveCallBack);
	glfwSetCursorEnterCallback(window, cursorEnterWindowCallBack);
	glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
	// Setup Dear ImGui binding->
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init("#version 330");

	//// Setup style.
	auto& style = ImGui::GetStyle();

	style.WindowTitleAlign = { 0.5f, 0.5f };
	style.ItemSpacing = { 7.0f, 10.0f };
	style.WindowBorderSize = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ChildBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;
	style.FrameRounding = 7.0f;
	style.WindowRounding = 3.0f;
	style.GrabRounding = 7.0f;
	style.Alpha = 1.0f;
	ImGui::StyleColorsClassic();
	// Clear error buffer.
	while (glGetError() != GL_NO_ERROR);
	glfwSwapInterval(1);

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

	Grid *g;
	g = new Grid(shader);

	std::random_device random_rotation, random_type;
	std::vector<int> bag;
	for (int i = 0; i < 7; i++)
		bag.push_back(i);

	PiecePtr currentPiece(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece1(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece2(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece3(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece4(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece5(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));
	PiecePtr nextPiece6(new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4)));

	glm::mat4 posicaoNextPiece = glm::mat4(g->getModel());
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

	g->start(&currentPiece);
	time = (int)(g->scale * glfwGetTime());
	
	float deltaTime;
	bool control_window = true;
	paused = false;
	menu = true;
	player_1 = false;
	options = false;

	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	ImFont* font_tetris = io.Fonts->AddFontFromFileTTF("../../Include/misc/fonts/ProggyTiny.ttf", 20.0f);
	ImFont* font_control = io.Fonts->AddFontFromFileTTF("../../Include/misc/fonts/Cousine-Regular.ttf", 20.0f);
	//ImFont* font_control = io.Fonts->AddFontFromFileTTF("../../Include/misc/fonts/Roboto-Medium.ttf", 20.0f);
	//ImFont* font_points = io.Fonts->AddFontFromFileTTF("../../Include/misc/fonts/DroidSans.ttf", 20.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		deltaTime = 1000.0f / ImGui::GetIO().Framerate;
		// Pool and handle events.
		glfwPollEvents();
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();		

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (menu)
		{
			ImGui::SetNextWindowPosCenter();
			if (ImGui::Begin("MENU", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				ImGui::PushItemWidth(-1);
				ImGui::SetWindowSize(ImVec2(400, 180));
				if (ImGui::Button("1 JOGADOR", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
				{
					ImGui::CloseCurrentPopup();
					player_1 = true;
					paused = true;
					menu = false;
				}
				if (ImGui::Button("2 JOGADORES", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
				{
					/*ImGui::CloseCurrentPopup();
					player_1 = false;
					paused = true;
					menu = false;*/
				}
				if (ImGui::Button("ESCOLHAS", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
				{
					ImGui::CloseCurrentPopup();
					menu = false;
					options = true;
				}
				if (ImGui::Button("SAIR DO JOGO", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
					glfwSetWindowShouldClose(window, GL_TRUE);
				ImGui::PopItemWidth();
			}
			ImGui::End();
		}
		else if (options)
		{
			ImGui::SetNextWindowPosCenter();
			if (ImGui::Begin("ESCOLHAS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				static int level = g->getLevel();
				ImGui::SetWindowSize(ImVec2(400, 105));
				ImGui::SliderInt("LEVEL", &level, 0, 5);
				ImGui::PushItemWidth(-1);
				if (ImGui::Button("SALVAR", ImVec2(ImGui::GetWindowSize().x / 2.0f - 15.0f, 0.0f)))
				{
					ImGui::CloseCurrentPopup();
					menu = true;
					options = false;
					g->setLevel(level);
				}
				ImGui::SameLine(0, 15.0f);
				if (ImGui::Button("CANCELAR", ImVec2(ImGui::GetWindowSize().x / 2.0f - 15.0f, 0.0f)))
				{
					ImGui::CloseCurrentPopup();
					menu = true;
					options = false;
					level = g->getLevel();
				}
				ImGui::PopItemWidth();
			}
			ImGui::End();
		}
		else if(player_1)
		{
			if (control_window)
			{
				ImGui::PushFont(font_control);
				ShowAppControlOverlay(&control_window);
				ImGui::PopFont();
			}

			ImGui::PushFont(font_tetris);
			ShowAppPointOverlay(g->getPoints());
			ImGui::PopFont();

			if (paused)
			{
				/*static bool demo = true;
				if (demo)
					ImGui::ShowDemoWindow(&demo);
				else
					paused = false;*/

				ShowAppPauseOverlay(window);
			}
			else if (g->lost)
			{
				ImGui::SetNextWindowPosCenter();
				if (ImGui::Begin("DERROTA", &g->lost, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNav))
				{
					ImGui::SetWindowSize(ImVec2(400, 130));

					ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 110) / 2.0f - 15.0f);
					ImGui::Text("PERDEU KKK!\n\n");
					ImGui::PushItemWidth(-1);
					if (ImGui::Button("MENU", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
					{
						delete g;
						g = new Grid(shader);

						for (int i = 0; i < 7; i++)
							bag.push_back(i);
						currentPiece = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece1 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece2 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece3 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece4 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece5 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						nextPiece6 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));

						glm::mat4 posicaoNextPiece = glm::mat4(g->getModel());
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

						g->start(&currentPiece);
						time = (int)(g->scale * glfwGetTime());

						ImGui::CloseCurrentPopup();
						paused = false;
						menu = true;
					}
					ImGui::SetItemDefaultFocus();
					if (ImGui::Button("SAIR DO JOGO", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
					{
						glfwSetWindowShouldClose(window, GL_TRUE);
					}
					ImGui::PopItemWidth();
				}
				ImGui::End();
			}
			else
			{
				// input
				// -----
				processInput(window, currentPiece, g);

				// render
				// ------
				if ((g->endgame && g->change && glfwGetTime() - g->ENDGAME >= 0.6f) || collapse)
				{
					time = (int)(g->scale * glfwGetTime());
					g->change = false;
					g->fallAllTheWay();
					g->change = false;
					g->endgame = false;
					g->lineComplete();
					currentPiece.~PiecePtr();
					currentPiece = nextPiece1;
					nextPiece1 = nextPiece2;
					nextPiece2 = nextPiece3;
					nextPiece3 = nextPiece4;
					nextPiece4 = nextPiece5;
					nextPiece5 = nextPiece6;
					if (!g->lose())
					{
						g->start(&currentPiece);
						nextPiece6 = new Piece(shader, (Piece::types)pop_bag(&bag, (random_type() % 7)), (Piece::rotation)(random_rotation() % 4));
						glm::mat4 posicaoNextPiece = glm::mat4(g->getModel());
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
				}

				if (!g->endgame && g->change)				// peça deu colisão embaixo e não estava previamente no endgame
				{
					g->ENDGAME = glfwGetTime();
					g->endgame = true;
				}
				else if (g->endgame && !g->change)		// peça estava no endgame mas mudou de ideia sobre a colisão
					g->endgame = false;

				if ((int)(g->scale * glfwGetTime()) > time || g->scaleBack)
				{
					if (g->scaleBack)
						g->scaleBack = false;
					time = (int)(g->scale * glfwGetTime());
					g->fall();
				}
			}
			if (collapse)
			{
				time = (int)(g->scale * glfwGetTime());
				collapse = false;
			}

			// render boxes
			glBindVertexArray(VAO);
			g->draw(shader);
			nextPiece1->draw(shader);
			nextPiece2->draw(shader);
			nextPiece3->draw(shader);
			nextPiece4->draw(shader);
			nextPiece5->draw(shader);
			nextPiece6->draw(shader);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		ImGui::Render();
		glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
		glViewport(0, 0, displayWidth, displayHeight);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		
	}
	delete g;
	currentPiece.~PiecePtr();
	nextPiece1.~PiecePtr();
	nextPiece2.~PiecePtr();
	nextPiece3.~PiecePtr();
	nextPiece4.~PiecePtr();
	nextPiece5.~PiecePtr();
	nextPiece6.~PiecePtr();

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
void processInput(GLFWwindow *window, PiecePtr p, Grid *g)
{
	static bool key_a_release = true, key_d_release = true, key_i_release = true, key_o_release = true, key_p_release = true;

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
		g->scale = g->fastScale;
	else
	{
		if (g->scale == g->fastScale)
			g->scaleBack = true;
		g->scale = g->normalScale;
	}
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
		g->ENDGAME = glfwGetTime();
		key_o_release = false;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && key_p_release)
	{
		g->rotate(false);
		g->ENDGAME = glfwGetTime();
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

void keyInputCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static bool key_esc_release = true;
	if (action == GLFW_PRESS)
	{
		if (mods == GLFW_MOD_ALT && key == GLFW_KEY_F4)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
			return;
		}
		if (key == GLFW_KEY_ESCAPE && key_esc_release)
		{
			paused = !paused;
			key_esc_release = false;
			return;
		}
	}
	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			key_esc_release = true;
			return;
		}
	}
	if (!keyInputEvent(key, action, mods))
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

bool keyInputEvent(int key, int action, int mods)
{
	(void)key;
	(void)action;
	(void)mods;
	return false;
}

void windowResizeCallBack(GLFWwindow* window, int w, int h)
{
	SCR_WIDTH = w;
	SCR_HEIGHT = h;
	windowResizeEvent(SCR_WIDTH, SCR_HEIGHT);
}

bool windowResizeEvent(int width, int height)
{
	(void)width;
	(void)height;
	return false;
}

void scrollCallBack(GLFWwindow* window, double xOffSet, double yOffSet)
{
	scrollEvent(xOffSet, yOffSet);
	ImGui_ImplGlfw_ScrollCallback(window, xOffSet, yOffSet);
}

bool scrollEvent(double xOffset, double yOffset)
{
	(void)xOffset;
	(void)yOffset;
	return false;
}

void mouseButtonCallBack(GLFWwindow* window,
	int button,
	int actions,
	int mods)
{
	mouseButtonInputEvent(button, actions, mods);
}

bool mouseButtonInputEvent(int button, int actions, int mods)
{
	(void)button;
	(void)actions;
	(void)mods;
	return false;
}

void mouseMoveCallBack(GLFWwindow* window, double xPos, double yPos)
{
	mouseMoveEvent(xPos, yPos);
}

bool mouseMoveEvent(double xPos, double yPos)
{
	(void)xPos;
	(void)yPos;
	return false;
}

void cursorEnterWindowCallBack(GLFWwindow* window, int entered)
{
	cursorEnterWindowEvent(entered);
}

bool cursorEnterWindowEvent(int entered)
{
	(void)entered;
	return false;
}

static void ShowAppControlOverlay(bool *p_open)
{
	const float DISTANCE = 10.0f;
	ImVec2 window_pos = ImVec2(DISTANCE, DISTANCE);
	ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
	if (ImGui::Begin("Controles", p_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("A, S e D movimentam o quadro");
		ImGui::Text("O e P rotacionam o quadro");
		ImGui::Text("I desce o quadro");
	}
	ImGui::End();
}

static void ShowAppPointOverlay(float points)
{
	const float DISTANCE = 10.0f;
	ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - DISTANCE, DISTANCE);
	ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
	float aux;
	float size = 0.0f;
	aux = points;
	do
	{
		aux /= 10.0f;
		size += 1.0f;
	} while (aux >= 1.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	if (ImGui::Begin("PONTOS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::SetWindowSize(ImVec2(175, 75));
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 10*size - 15.0f);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.0f", points);
	}
	ImGui::End();
}

static void ShowAppPauseOverlay(GLFWwindow* window)
{
	ImGui::SetNextWindowPosCenter();
	ImGui::SetNextWindowBgAlpha(0.6f); // Transparent background
	if (ImGui::Begin("PAUSE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::PushItemWidth(-1);
		ImGui::SetWindowSize(ImVec2(400, 140));
		if (ImGui::Button("CONTINUAR", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
		{
			paused = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("MENU", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
		{
			// are you sure?
			ImGui::CloseCurrentPopup();
			paused = false;
			menu = true;
		}
		if (ImGui::Button("SAIR DO JOGO", ImVec2(ImGui::GetWindowSize().x - 15.0f, 0.0f)))
			glfwSetWindowShouldClose(window, GL_TRUE);
		ImGui::PopItemWidth();
	}
	ImGui::End();
}