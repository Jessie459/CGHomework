#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "ship.h"
#include "particle_generator.h"

#include <iostream>
using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);
bool checkCollision(glm::vec3 position1, float size1, glm::vec3 position2, float size2);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int CHEST_MAX_OFFSET = 100;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int scene_number = 1;
unsigned int chestOffset[6];
bool stencil = false;
Ship ship(camera.Position + glm::vec3(0.0f, -0.8f, -1.0f));

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ComputerGraphicsProject", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// build and compile shaders
	// -------------------------
	Shader aircraft_shader("shaders/model_lighting.vs", "shaders/model_lighting.fs");
	Shader depth_shader("shaders/depth.vs", "shaders/depth.fs");
	Shader shadow_shader("shaders/shadow.vs", "shaders/shadow.fs");
	Shader scenery_shader("shaders/skybox.vs", "shaders/skybox.fs");

	Shader aircraft_env_shader("shaders/model_environment.vs", "shaders/model_environment.fs");
	Shader chest_shader("shaders/model_texture.vs", "shaders/model_texture.fs");
	Shader cloud_shader("shaders/skybox.vs", "shaders/skybox.fs");
	Shader explode_shader("shaders/explode.vs", "shaders/explode.fs", "shaders/explode.gs");
	Shader stencil_shader("shaders/stencil.vs", "shaders/stencil.fs");

	Shader shader("shaders/model_texture.vs", "shaders/model_texture.fs");
	Shader galaxy_shader("shaders/skybox.vs", "shaders/skybox.fs");
	Shader particle_shader("shaders/particle.vs", "shaders/particle.fs");

	// shader configuration
	// --------------------
	shadow_shader.use();
	shadow_shader.setInt("diffuseMap", 0);
	shadow_shader.setInt("shadowMap", 1);

	particle_shader.use();
	particle_shader.setInt("sprite", 0);

	scenery_shader.use();
	scenery_shader.setInt("skybox", 0);

	aircraft_env_shader.use();
	aircraft_env_shader.setInt("skybox", 0);

	cloud_shader.use();
	cloud_shader.setInt("skybox", 0);

	galaxy_shader.use();
	galaxy_shader.setInt("skybox", 0);

	// load models
	// -----------
	Model aircraft("objects/E-45-Aircraft/E 45 Aircraft_obj.obj");
	Model chest("objects/Pirate_A_Chest_A/Pirate_A_Chest_A.FBX");
	Model earth("objects/earth/earth.obj");
	Model moon("objects/月球/月球.obj");
	Model star1("objects/太阳/太阳.obj");
	Model star2("objects/水星/水星.obj");
	Model star3("objects/金星/金星.obj");
	Model star4("objects/火星/火星.obj");
	Model star5("objects/木星/木星.obj");
	Model star6("objects/土星/土星.obj");
	Model star7("objects/天王星/天王星.obj");
	Model star8("objects/海王星/海王星.obj");

	float aircraftSize = aircraft.getCubeBoundingBox();
	float chestSize = chest.getCubeBoundingBox();

	// load textures
	// -------------
	unsigned int diffuseMap = loadTexture("textures/grass.jpg");
	unsigned int particle_texture = loadTexture("textures/particle.png");

	vector<std::string> scenery_faces
	{
		"textures/lake/right.jpg",
		"textures/lake/left.jpg",
		"textures/lake/top.jpg",
		"textures/lake/down.jpg",
		"textures/lake/back.jpg",
		"textures/lake/front.jpg"
	};
	unsigned int sceneryTexture = loadCubemap(scenery_faces);

	vector<std::string> cloud_faces
	{
		"textures/cloud/right.jpg",
		"textures/cloud/left.jpg",
		"textures/cloud/top.jpg",
		"textures/cloud/down.jpg",
		"textures/cloud/back.jpg",
		"textures/cloud/front.jpg"
	};
	unsigned int cloudTexture = loadCubemap(cloud_faces);

	vector<std::string> galaxy_faces
	{
		"textures/ame_nebula/purplenebula_rt.tga",
		"textures/ame_nebula/purplenebula_lf.tga",
		"textures/ame_nebula/purplenebula_up.tga",
		"textures/ame_nebula/purplenebula_dn.tga",
		"textures/ame_nebula/purplenebula_ft.tga",
		"textures/ame_nebula/purplenebula_bk.tga"
	};
	unsigned int galaxyTexture = loadCubemap(galaxy_faces);

	// congifure skybox vertices
	// -------------------------
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

	// configure plane vertices
	// ------------------------
	float planeVertices[] = {
		// Positions           // Normals        // Texture Coords
		 25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		-25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 0.0f,  0.0f,

		 25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		 25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f,  25.0f
	};
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// particle system
	// ---------------
	ParticleGenerator *generator = new ParticleGenerator(100);

	// light position
	glm::vec3 lightPos(5.0f, 5.0f, 0.0f);
	// chest position
	glm::vec3 chestPositions[] = {
		glm::vec3(0.0f, 0.0f, -5.0f),
		glm::vec3(5.0f, 0.0f, -3.0f),
		glm::vec3(5.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 5.0f),
		glm::vec3(-5.0f, 0.0f, 3.0f),
		glm::vec3(-5.0f, 0.0f, -3.0f)
	};
	// chest offset
	memset(chestOffset, 0, sizeof(chestOffset));

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		switch (scene_number)
		{
		case 1:
		{
			// disable stencil writing
			// -----------------------
			glStencilMask(0x00);

			// set model, projection, view and lightSpaceMatrix matrices
			// ---------------------------------------------------------
			glm::mat4 model;
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			glm::mat4 lightProjection, lightView;
			glm::mat4 lightSpaceMatrix;
			lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, 0.1f, 100.0f);
			lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
			lightSpaceMatrix = lightProjection * lightView;

			// configure uniform variables
			// ---------------------------
			aircraft_shader.use();
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.2f));
			aircraft_shader.setMat4("model", model);
			aircraft_shader.setMat4("view", view);
			aircraft_shader.setMat4("projection", projection);
			aircraft_shader.setVec3("viewPos", camera.Position);
			aircraft_shader.setVec3("lightPos", lightPos);

			shadow_shader.use();
			model = glm::mat4(1.0f);
			shadow_shader.setMat4("model", model);
			shadow_shader.setMat4("view", view);
			shadow_shader.setMat4("projection", projection);
			shadow_shader.setVec3("viewPos", camera.Position);
			shadow_shader.setVec3("lightPos", lightPos);
			shadow_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			depth_shader.use();
			model = glm::mat4(1.0f);
			depth_shader.setMat4("model", model);
			depth_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			// render depth of scene to texture from light's perspective
			// ---------------------------------------------------------
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseMap);
			// draw plane
			depth_shader.use();
			glBindVertexArray(planeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// draw aircraft
			aircraft_shader.use();
			aircraft.Draw(aircraft_shader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// reset viewport
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render scene as normal using the generated depth map
			// ----------------------------------------------------
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			// draw plane
			shadow_shader.use();
			glBindVertexArray(planeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			// draw aircraft
			aircraft_shader.use();
			aircraft.Draw(aircraft_shader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// draw scenery skybox
			// -------------------
			glDepthFunc(GL_LEQUAL);
			scenery_shader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			scenery_shader.setMat4("view", view);
			scenery_shader.setMat4("projection", projection);
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, sceneryTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);
			break;
		}
		case 2:
		{
			// disable stencil writing
			// -----------------------
			glStencilMask(0x00);

			// configure view and projection matrices
			// --------------------------------------
			glm::mat4 model, view, projection;
			view = camera.GetViewMatrix();
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			aircraft_env_shader.use();
			aircraft_env_shader.setMat4("view", view);
			aircraft_env_shader.setMat4("projection", projection);

			chest_shader.use();
			chest_shader.setMat4("view", view);
			chest_shader.setMat4("projection", projection);

			explode_shader.use();
			explode_shader.setMat4("view", view);
			explode_shader.setMat4("projection", projection);

			cloud_shader.use();
			cloud_shader.setMat4("projection", projection);

			stencil_shader.use();
			stencil_shader.setMat4("view", view);
			stencil_shader.setMat4("projection", projection);

			// set aircraft position
			// ---------------------
			glm::vec3 aircraftPosition = camera.Position + camera.Front * 2.0f + glm::vec3(0.0f, -0.5f, 0.0f);

			// draw chests
			// -----------
			for (unsigned int i = 0; i < 6; i++)
			{
				if (chestOffset[i] < CHEST_MAX_OFFSET)
				{
					model = glm::mat4(1.0f);
					model = glm::translate(model, chestPositions[i]);
					model = glm::translate(model, glm::vec3(0.0f, sin(glfwGetTime()), 0.0f));
					model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					model = glm::scale(model, glm::vec3(0.01f));

					// check collision
					bool collision = checkCollision(aircraftPosition, aircraftSize * 0.2f, chestPositions[i], chestSize * 0.01f);
					if (collision || chestOffset[i] > 0)
					{
						chestOffset[i]++;
						float offset = (float)chestOffset[i] * 0.1f;
						explode_shader.use();
						explode_shader.setMat4("model", model);
						explode_shader.setFloat("offset", offset);
						chest.Draw(explode_shader);
					}
					else
					{
						chest_shader.use();
						chest_shader.setMat4("model", model);
						chest.Draw(chest_shader);
					}
				}
			}

			// draw cloud skybox
			// -----------------
			glDepthFunc(GL_LEQUAL);
			cloud_shader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			cloud_shader.setMat4("view", view);
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cloudTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);

			// draw aircraft and its outline
			// -----------------------------
			if (stencil)
			{
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);
			}

			// draw aircraft
			aircraft_env_shader.use();
			model = glm::mat4(1.0f);
			model = glm::translate(model, aircraftPosition);
			model = glm::rotate(model, glm::radians(270.0f - camera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.2f));
			aircraft_env_shader.setMat4("model", model);
			aircraft_env_shader.setVec3("cameraPos", camera.Position);
			aircraft.Draw(aircraft_env_shader);

			if (stencil)
			{
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00);

				// draw outline
				glDisable(GL_DEPTH_TEST);
				stencil_shader.use();
				model = glm::mat4(1.0f);
				model = glm::translate(model, aircraftPosition);
				model = glm::rotate(model, glm::radians(270.0f - camera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.22f));
				stencil_shader.setMat4("model", model);
				aircraft.Draw(stencil_shader);
				glStencilMask(0xFF);
				glEnable(GL_DEPTH_TEST);
			}
			break;
		}
		case 3:
		{
			// disable stencil writing
			// -----------------------
			glStencilMask(0x00);

			// don't forget to enable shader before setting uniforms
			shader.use();

			// view/projection transformations
			glm::mat4 model;
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			// render the aircraft
			ship.Position = camera.Position + glm::vec3(0.0f, -0.8f, -1.0f);
			glm::quat keyquat = glm::quat(glm::vec3(glm::radians(camera.Pitch), glm::radians(-(camera.Yaw + 90)), ship.Roll));
			glm::mat4 rot = glm::mat4_cast(keyquat);
			model = glm::mat4(1.0f);
			model = glm::translate(model, ship.Position);
			model = model * rot;
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			shader.setMat4("model", model);
			shader.setMat4("view", view);
			shader.setMat4("projection", projection);
			aircraft.Draw(shader);

			// render the star1
			glm::mat4 starp1 = glm::mat4(1.0f);
			starp1 = glm::translate(starp1, glm::vec3(0.0f, 0.0f, 0.0f));
			starp1 = glm::rotate(starp1, (float)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
			shader.setMat4("model", starp1);
			star1.Draw(shader);

			// render the star2
			glm::mat4 starp2 = glm::rotate(starp1, (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 0.0f)); // 公转
			starp2 = glm::translate(starp2, glm::vec3(0.0f, 0.0f, -6.0f));
			starp2 = glm::rotate(starp2, (float)glfwGetTime() * 2, glm::vec3(0.0f, 1.0f, 1.0f)); // 自转
			shader.setMat4("model", starp2);
			star2.Draw(shader);

			// render the star3
			glm::mat4 starp3 = glm::rotate(starp1, (float)(glfwGetTime()*0.9), glm::vec3(0.5f, 1.0f, 0.5f));
			starp3 = glm::translate(starp3, glm::vec3(0.0f, 0.0f, -12.0f));
			starp3 = glm::rotate(starp3, (float)(glfwGetTime()*2.5), glm::vec3(1.0f, 1.0f, 0.0f));
			shader.setMat4("model", starp3);
			star3.Draw(shader);

			// render the earth
			glm::mat4 emodel = glm::rotate(starp1, (float)(glfwGetTime()*0.8), glm::vec3(0.5f, 1.0f, 0.0f));
			emodel = glm::translate(emodel, glm::vec3(0.0f, -0.5f, -20.0f));
			emodel = glm::rotate(emodel, (float)(glfwGetTime()*0.5), glm::vec3(0.0f, 1.0f, 0.5f));
			shader.setMat4("model", emodel);
			earth.Draw(shader);

			// render the moon
			glm::mat4 mmodel = glm::rotate(emodel, (float)(glfwGetTime() * 2), glm::vec3(0.0f, 0.8f, 0.3f));
			mmodel = glm::translate(mmodel, glm::vec3(0.0f, 0.0f, -4.0f));
			mmodel = glm::rotate(mmodel, (float)(glfwGetTime()*1.0), glm::vec3(0.7f, 0.3f, 0.0f));
			shader.setMat4("model", mmodel);
			moon.Draw(shader);

			// render the star4
			glm::mat4 starp4 = glm::rotate(starp1, (float)(glfwGetTime()*0.6), glm::vec3(1.0f, 1.0f, 1.0f));
			starp4 = glm::translate(starp4, glm::vec3(0.0f, 0.0f, -30.0f));
			starp4 = glm::rotate(starp4, (float)(glfwGetTime()*3.5), glm::vec3(0.0f, 1.0f, 0.2f));
			shader.setMat4("model", starp4);
			star4.Draw(shader);

			// render the star5
			glm::mat4 starp5 = glm::rotate(starp1, (float)(glfwGetTime()*0.5), glm::vec3(0.5f, 1.4f, 0.3f));
			starp5 = glm::translate(starp5, glm::vec3(0.0f, 0.0f, -38.0f));
			starp5 = glm::rotate(starp5, (float)(glfwGetTime()*4.0), glm::vec3(0.0f, 1.0f, 0.2f));
			shader.setMat4("model", starp5);
			star5.Draw(shader);

			// render the star6
			glm::mat4 starp6 = glm::rotate(starp1, (float)(glfwGetTime()*0.6), glm::vec3(0.8f, 1.4f, 0.6f));
			starp6 = glm::translate(starp6, glm::vec3(0.0f, 0.0f, -50.0f));
			starp6 = glm::rotate(starp6, (float)(glfwGetTime()*3.0), glm::vec3(0.0f, 1.0f, 0.8f));
			shader.setMat4("model", starp6);
			star6.Draw(shader);

			// render the star7
			glm::mat4 starp7 = glm::rotate(starp1, (float)(glfwGetTime()*0.7), glm::vec3(0.2f, 1.0f, 2.0f));
			starp7 = glm::translate(starp7, glm::vec3(0.0f, 0.0f, -60.0f));
			starp7 = glm::rotate(starp7, (float)(glfwGetTime()*2.0), glm::vec3(0.0f, 1.0f, 0.1f));
			shader.setMat4("model", starp7);
			star7.Draw(shader);

			// render the star8
			glm::mat4 starp8 = glm::rotate(starp1, (float)(glfwGetTime()*0.7), glm::vec3(0.3f, 0.3f, 0.3f));
			starp8 = glm::translate(starp8, glm::vec3(0.0f, 0.0f, -75.0f));
			starp8 = glm::rotate(starp8, (float)(glfwGetTime()*1.0), glm::vec3(0.0f, 1.0f, 0.5f));
			shader.setMat4("model", starp8);
			star8.Draw(shader);

			// draw particles
			generator->Update(deltaTime, 2);
			generator->Draw(particle_shader, particle_texture);

			// draw skybox as last
			glDepthFunc(GL_LEQUAL);
			galaxy_shader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			galaxy_shader.setMat4("view", view);
			galaxy_shader.setMat4("projection", projection);
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, galaxyTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);

			break;
		}
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		scene_number = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		scene_number = 2;
		memset(chestOffset, 0, sizeof(chestOffset));
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		scene_number = 3;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (scene_number == 2)
	{
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			stencil = true;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
			stencil = false;
	}

	if (scene_number == 3)
	{
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			ship.ProcessKeyboard(ROLLLEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			ship.ProcessKeyboard(ROLLRIGHT, deltaTime);
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

bool checkCollision(glm::vec3 position1, float size1, glm::vec3 position2, float size2)
{
	bool collisionX = true;
	bool collisionY = true;
	bool collisionZ = true;

	if (position1.x + size1 < position2.x - size2 || position1.x - size1 > position2.x + size2)
		collisionX = false;
	if (position1.y + size1 < position2.y - size2 || position1.y - size1 > position2.y + size2)
		collisionY = false;
	if (position1.z + size1 < position2.z - size2 || position1.z - size1 > position2.z + size2)
		collisionZ = false;

	return collisionX && collisionY && collisionZ;
}