/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#define RESOURCEDIR "../resources"

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"

#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
#include "SpaceGame.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> sphere;
shared_ptr<Shape> plane;
shared_ptr<Shape> shipMesh;
shared_ptr<Shape> asteroidMesh;
shared_ptr<Shape> ufoMesh;

PlayerShip ship;
vector<Planet> planets;
vector<Asteroid> asteroids;

vec2 mousePos;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10 * ftime;
		}
		else if (s == 1)
		{
			speed = -10 * ftime;
		}
		float yangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}

	void lookAt(glm::vec3 point)
	{
		glm::vec3 direction = normalize(point - pos);
		rot.x = acos(dot(vec3(direction.x, 0, 0), vec3(1, 0, 0)));
		rot.y = acos(dot(vec3(0, direction.y, 0), vec3(0, 1, 0)));
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:
	WindowManager *windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, pBilboard, pTile, psky, pMesh, pWaypoint, pNumbers, pParticles;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint PointPositionsID, PointScaleID, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;
	GLuint Texture3;
	GLuint Texture4;
	GLuint TextureDeliveries;
	GLuint TextureTimeLeft;
	GLuint TextureParticle;

	double total_time = 0.0;

	//line
	Line linerender;
	Line smoothrender;
	vector<vec3> line;
	vector<vec3> splinepoints;
	vector<mat4> Marr;

	void
	keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}

		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			if (smoothrender.is_active())
				smoothrender.reset();
			else
			{
				smoothrender.re_init_line(splinepoints);
			}
		}
	}

	void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		double posX, posY;
		glfwGetWindowSize(window, &width, &height);
		glfwGetCursorPos(window, &posX, &posY);
		mousePos.x = posX / (double)width;
		mousePos.y = posY / (double)height;
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX << " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, PointPositionsID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6, sizeof(float) * 2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string &resourceDirectory)
	{

		// Initialize mesh.
		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();

		plane = make_shared<Shape>();
		plane->loadMesh(resourceDirectory + "/plane.obj");
		plane->resize();
		plane->init();
		vector<vec3> positions;
		positions.push_back(vec3(0, 0, 0));
		positions.push_back(vec3(0, 0, -2)); // Reserve the middle.
		int num_planets = 10;
		int failed_attempts = 0;
		int choice_thresh = 200;
		double dist_range = 25.0;
		// Poisson disc sample the planets
		while (planets.size() < num_planets)
		{
			double x = (double)rand() / RAND_MAX * dist_range * 2.0 - dist_range;
			double y = 0.0;
			double z = (double)rand() / RAND_MAX * dist_range * 2.0 - dist_range;
			vec3 pos = vec3(x, y, z);
			bool valid = true;
			for (auto &position : positions)
			{
				valid &= distance(pos, position) > 10.0f;
			}

			if (!valid)
			{
				failed_attempts++;
				if (failed_attempts > choice_thresh)
					break;
				continue;
			}
			failed_attempts = 0;

			positions.push_back(pos);
			planets.push_back(Planet(pos));
		}

		ship.chooseNewDestination(planets);

		asteroidMesh = make_shared<Shape>();
		asteroidMesh->loadMesh(resourceDirectory + "/asteroid.obj");
		asteroidMesh->resize();
		asteroidMesh->init();

		vector<vec3> line;

		for (auto &planet : planets)
		{
			for (int i = 0; i < 5; i++)
			{
				line.push_back(vec3(1, 0, 0));
				line.push_back(vec3(0, 0, 1));
				line.push_back(vec3(-1, 0, 0));
				line.push_back(vec3(0, 0, -1));
				line.push_back(vec3(1, 0, 0));
			}

			for (auto &point : line)
			{
				float r = (float)rand() / RAND_MAX;
				point = point * vec3(1.5 * planet.scale + r * 1.5) + planet.pos;
			}
			Asteroid a;
			a.setPath(line);
			asteroids.push_back(a);
			line.clear();
		}

		shipMesh = make_shared<Shape>();
		string mtldir = resourceDirectory + "/Viper-mk-IV-fighter/";
		shipMesh->loadMesh(resourceDirectory + "/Viper-mk-IV-fighter/Viper-mk-IV-fighter.obj", &mtldir, stbi_load);

		shipMesh->resize();
		shipMesh->init();

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &PointPositionsID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, PointPositionsID);

		GLfloat pointPositions[] = {
			-1.0,
			-1.0,
			1.0,
			1.0,
			-1.0,
			1.0,
			1.0,
			1.0,
			1.0,
			-1.0,
			1.0,
			1.0,
		};
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(pointPositions), pointPositions, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		//color
		GLfloat pointScale[] = {
			0.5,
			0.7,
			0.9,
			1.0,
		};
		glGenBuffers(1, &PointScaleID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, PointScaleID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pointScale), pointScale, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/explosion.jpg";
		strcpy(filepath, str.c_str());
		unsigned char *data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/background.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture3
		str = resourceDirectory + "/delivered.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture3);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/numbers.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture4);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/deliveries.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureDeliveries);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureDeliveries);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/time-left.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureTimeLeft);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureTimeLeft);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/alpha.bmp";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureParticle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureParticle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex"); //tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		GLuint Tex3Location = glGetUniformLocation(prog->pid, "tex3");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
		glUniform1i(Tex3Location, 2);
		float rad = 3.0;
		float z_dist = -6;
	}

	//General OGL initialization - set OGL state here
	void init(const std::string &resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		pBilboard = std::make_shared<Program>();
		pBilboard->setVerbose(true);
		pBilboard->setShaderNames(resourceDirectory + "/bilboard_vertex.glsl", resourceDirectory + "/bilboard_frag.glsl");
		if (!pBilboard->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pBilboard->addUniform("P");
		pBilboard->addUniform("V");
		pBilboard->addUniform("M");
		pBilboard->addUniform("campos");
		pBilboard->addUniform("flip");
		pBilboard->addAttribute("vertPos");
		pBilboard->addAttribute("vertNor");
		pBilboard->addAttribute("vertTex");

		pNumbers = std::make_shared<Program>();
		pNumbers->setVerbose(true);
		pNumbers->setShaderNames(resourceDirectory + "/numbers_vertex.glsl", resourceDirectory + "/numbers_frag.glsl");
		if (!pNumbers->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pNumbers->addUniform("P");
		pNumbers->addUniform("V");
		pNumbers->addUniform("M");
		pNumbers->addUniform("numberDisplay");
		pNumbers->addUniform("campos");
		pNumbers->addAttribute("vertPos");
		pNumbers->addAttribute("vertNor");
		pNumbers->addAttribute("vertTex");

		pParticles = std::make_shared<Program>();
		pParticles->setVerbose(true);
		pParticles->setShaderNames(resourceDirectory + "/particle_vert.glsl", resourceDirectory + "/particle_frag.glsl");
		if (!pParticles->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pParticles->addUniform("P");
		pParticles->addUniform("V");
		pParticles->addUniform("M");
		pParticles->addUniform("pointColor");
		pParticles->addUniform("campos");
		pParticles->addAttribute("pointPos");
		pParticles->addAttribute("pointScale");

		pWaypoint = std::make_shared<Program>();
		pWaypoint->setVerbose(true);
		pWaypoint->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/waypoint_fragment.glsl");
		if (!pWaypoint->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pWaypoint->addUniform("P");
		pWaypoint->addUniform("V");
		pWaypoint->addUniform("M");
		pWaypoint->addUniform("campos");
		pWaypoint->addUniform("col");
		pWaypoint->addAttribute("vertPos");
		pWaypoint->addAttribute("vertNor");
		pWaypoint->addAttribute("vertTex");

		pTile = std::make_shared<Program>();
		pTile->setVerbose(true);
		pTile->setShaderNames(resourceDirectory + "/tile_vertex.glsl", resourceDirectory + "/tile_fragment.glsl");
		if (!pTile->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pTile->addUniform("P");
		pTile->addUniform("V");
		pTile->addUniform("M");
		pTile->addUniform("tile_offset");
		pTile->addUniform("prev_offset");
		pTile->addUniform("t");
		pTile->addUniform("campos");
		pTile->addAttribute("vertPos");
		pTile->addAttribute("vertNor");
		pTile->addAttribute("vertTex");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pMesh = std::make_shared<Program>();
		pMesh->setVerbose(true);
		pMesh->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!pMesh->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pMesh->addUniform("P");
		pMesh->addUniform("V");
		pMesh->addUniform("M");
		pMesh->addUniform("campos");
		pMesh->addUniform("opacity");
		pMesh->addAttribute("vertPos");
		pMesh->addAttribute("vertNor");
		pMesh->addAttribute("vertTex");
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();
		total_time += frametime;

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P, P_Ortho; //View, Model and Perspective matrix

		M = glm::mat4(1);
		// Apply orthographic projection....
		P_Ortho = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P_Ortho = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		// Position Cam
		mycam.pos = vec3(0, 8, -8);
		V = glm::lookAt(mycam.pos, vec3(0, 0, 0), vec3(0, 1, 0));

		// draw universe plane
		glDisable(GL_DEPTH_TEST);
		pBilboard->bind();
		M = scale(mat4(1.0), vec3(2.0));
		glUniformMatrix4fv(pBilboard->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		plane->draw(pBilboard);
		pBilboard->unbind();
		glEnable(GL_DEPTH_TEST);

		// Draw Ship

		glm::mat4 TransPlane = glm::translate(glm::mat4(1.0f), vec3(0, 0, 0));
		glm::mat4 SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(ship.scale));
		ship.update(mousePos, frametime);
		glm::mat4 Rot = glm::rotate(mat4(1.0), (float)ship.angle, vec3(0, 1, 0));

		M = TransPlane * SPlane * ship.rotMat;

		pMesh->bind();
		glUniformMatrix4fv(pMesh->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pMesh->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pMesh->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1f(pMesh->getUniform("opacity"), ship.opacity);
		glUniform3fv(pMesh->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		shipMesh->draw(pMesh); //render!!!!!!!
		pMesh->unbind();
		static double anim_time = 0;

		// Handle different ship states
		if (ship.state == Exploding)
		{
			double duration = 0.7;

			if (anim_time > duration)
			{
				ship.respawn();
				ship.chooseNewDestination(planets);
				anim_time = 0.0;
			}
			else
			{
				pTile->bind();
				static vec2 offset = vec2(0.0, 0.0);
				double tile_time = (total_time / duration * 16.0);
				int frame_num = (int)tile_time;
				double t = tile_time - (double)frame_num;
				offset.x = (frame_num % 4);
				offset.y = frame_num / 4;
				static vec2 prev_offset = vec2(0.0, 0.0);
				prev_offset.x = (frame_num - 1) % 4;
				prev_offset.y = (frame_num - 1) / 4;
				glUniform2f(pTile->getUniform("tile_offset"), offset.x / 4.0, offset.y / 4.0);
				glUniform2f(pTile->getUniform("prev_offset"), prev_offset.x / 4.0, prev_offset.y / 4.0);
				glUniform1f(pTile->getUniform("t"), t);
				M = glm::translate(mat4(1.0), vec3(0, 0.7, 0)) * glm::lookAt(vec3(0), mycam.pos, vec3(0, 1, 0));
				glUniformMatrix4fv(pTile->getUniform("P"), 1, GL_FALSE, &P[0][0]);
				glUniformMatrix4fv(pTile->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				glUniformMatrix4fv(pTile->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture);
				plane->draw(pTile);
				pTile->unbind();

				anim_time += frametime;
			}
		}
		else if (ship.state == Delivering)
		{
			double duration = 1.0;
			ship.opacity = 0.5;
			// Animation over
			if (anim_time == 0.0)
			{
				ship.chooseNewDestination(planets);
				ship.time_left = 30.0;
				ship.points += 1;
			}
			else if (anim_time > duration)
			{
				// Flip ship around
				ship.state = Flying;
				anim_time = 0.0;
				ship.opacity = 1.0;
			}
			else //Play animation
			{
				// Show delivered message
				pBilboard->bind();
				M = glm::translate(mat4(1.0), vec3(0, 0.9, 1)) * glm::lookAt(vec3(0), mycam.pos, vec3(0, 1, 0)) * glm::scale(mat4(1), vec3(0.7));
				glUniformMatrix4fv(pBilboard->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
				glUniformMatrix4fv(pBilboard->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				glUniformMatrix4fv(pBilboard->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture3);
				plane->draw(pBilboard);
				pBilboard->unbind();

				// Draw shield
				M = glm::scale(mat4(1), vec3(ship.scale + 0.2));
				pWaypoint->bind();
				glUniformMatrix4fv(pWaypoint->getUniform("P"), 1, GL_FALSE, &P[0][0]);
				glUniformMatrix4fv(pWaypoint->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				glUniformMatrix4fv(pWaypoint->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glUniform4f(pWaypoint->getUniform("col"), 0.47f, 0.870f, 0.96f, 0.4f);
				glUniform3fv(pWaypoint->getUniform("campos"), 1, &mycam.pos[0]);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture2);
				sphere->draw(pWaypoint);
				pWaypoint->unbind();
			}
			if (ship.state == Delivering)
				anim_time += frametime;
		}
		// Draw Particles

		M = mat4(1.0);
		pParticles->bind();
		glUniformMatrix4fv(pParticles->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pParticles->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pParticles->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pParticles->getUniform("campos"), 1, &mycam.pos[0]);
		glBindVertexArray(VertexArrayID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureParticle);
		glDrawArrays(GL_POINTS, 0, 3);
		pParticles->unbind();

		// Draw the planets
		for (auto &planet : planets)
		{
			planet.update(total_time);
			M = glm::translate(mat4(1.0), ship.pos - planet.pos) * glm::scale(mat4(1), vec3(planet.scale)) * planet.rotMat;
			if (ship.intersects(planet))
			{
				ship.state = Exploding;
			}

			pMesh->bind();
			glUniformMatrix4fv(pMesh->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pMesh->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pMesh->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform3fv(pMesh->getUniform("campos"), 1, &mycam.pos[0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			sphere->draw(pMesh);
			pMesh->unbind();
		}

		// Draw planet waypoint
		if (ship.destination)
		{
			M = glm::translate(mat4(1.0), ship.pos - ship.destination->pos) * glm::scale(mat4(1), vec3(ship.destination->scale + 1.0));
			pWaypoint->bind();
			glUniformMatrix4fv(pWaypoint->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pWaypoint->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pWaypoint->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform4f(pWaypoint->getUniform("col"), 0.0f, 0.7f, 0.0f, 0.4f);
			glUniform3fv(pWaypoint->getUniform("campos"), 1, &mycam.pos[0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			sphere->draw(pWaypoint);
			pWaypoint->unbind();
		}

		// Draw direction waypoint
		vec3 planetDir = normalize(ship.destination->pos - ship.pos);
		M = glm::translate(mat4(1.0), -planetDir * vec3(1.1)) * glm::scale(mat4(1), vec3(0.07));
		pWaypoint->bind();
		glUniformMatrix4fv(pWaypoint->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pWaypoint->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pWaypoint->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform4f(pWaypoint->getUniform("col"), 0.0f, 0.7f, 0.0f, 0.7f);
		glUniform3fv(pWaypoint->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		sphere->draw(pWaypoint);
		pWaypoint->unbind();

		// Draw asteroids
		for (auto &asteroid : asteroids)
		{
			asteroid.update(total_time);
			M = glm::translate(mat4(1.0), ship.pos - asteroid.pos) * glm::scale(mat4(1), vec3(asteroid.scale)) * glm::rotate(mat4(1.0), (float)(total_time * asteroid.rotScalar), vec3(0, 1, 0));
			if (ship.intersects(asteroid))
			{
				ship.state = Exploding;
			}

			pMesh->bind();
			glUniformMatrix4fv(pMesh->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pMesh->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pMesh->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform3fv(pMesh->getUniform("campos"), 1, &mycam.pos[0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			asteroidMesh->draw(pMesh);
			pMesh->unbind();
		}

		// Draw GUI
		glDisable(GL_DEPTH_TEST);
		// Draw timer
		pNumbers->bind();
		M = translate(mat4(1.0), vec3(-1.3, -1.3, 0)) * scale(mat4(1.0), vec3(0.07));
		glUniformMatrix4fv(pNumbers->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pNumbers->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pNumbers->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(pNumbers->getUniform("numberDisplay"), (int)ship.time_left);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture4);
		plane->draw(pNumbers);
		pNumbers->unbind();

		// Draw timer label
		pBilboard->bind();
		M = translate(mat4(1.0), vec3(0.0, 0.1, 0.0)) * M;
		glUniformMatrix4fv(pBilboard->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(pBilboard->getUniform("flip"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureTimeLeft);
		plane->draw(pBilboard);
		pBilboard->unbind();

		// Draw points counter
		pNumbers->bind();
		M = translate(mat4(1.0), vec3(1.3, -1.3, 0)) * scale(mat4(1.0), vec3(0.07));
		glUniformMatrix4fv(pNumbers->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pNumbers->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pNumbers->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(pNumbers->getUniform("numberDisplay"), ship.points);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture4);
		plane->draw(pNumbers);
		pNumbers->unbind();

		// Draw points label
		pBilboard->bind();
		M = translate(mat4(1.0), vec3(0.0, 0.1, 0.0)) * M;
		glUniformMatrix4fv(pBilboard->getUniform("P"), 1, GL_FALSE, &P_Ortho[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pBilboard->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(pBilboard->getUniform("flip"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureDeliveries);
		plane->draw(pBilboard);
		pBilboard->unbind();

		glEnable(GL_DEPTH_TEST);

		//draw the lines

		// glm::vec3 linecolor = glm::vec3(1, 0, 0);
		// linerender.draw(P, V, linecolor);
		// linecolor = glm::vec3(0, 1, 1);
		// smoothrender.draw(P, V, linecolor);
	}
};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = RESOURCEDIR; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
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
