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
	std::shared_ptr<Program> prog, pTile, psky, pShip;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;
	GLuint Texture3;

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
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
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

		planets.push_back(Planet(vec3(0, 0, 20)));
		planets.push_back(Planet(vec3(5, 0, 15)));
		planets.push_back(Planet(vec3(3, 0, 4)));

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
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0, 1.0, //LD
			1.0, -1.0, 1.0,	 //RD
			1.0, 1.0, 1.0,	 //RU
			-1.0, 1.0, 1.0,	 //LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,

		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0,
			1,
			2,
			2,
			3,
			0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

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
		str = resourceDirectory + "/sky.jpg";
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
		// Initialize the maneuver path
		smoothrender.init();
		linerender.init();
		line.push_back(vec3(0, 1, z_dist));
		line.push_back(vec3(0, 0, z_dist += 3));
		line.push_back(vec3(0, -1, z_dist += 6));
		// Dive down
		line.push_back(vec3(0, -rad, z_dist += 5));

		//climb up and to the right
		line.push_back(vec3(rad, 0, z_dist += 5));

		// top out
		line.push_back(vec3(0, rad, z_dist += 5));

		// down to 270º
		line.push_back(vec3(-rad, 0, z_dist += 5));

		// bottom out
		line.push_back(vec3(0, -rad, z_dist += 5));

		//exit
		line.push_back(vec3(0, -rad / 2, z_dist += 6));
		line.push_back(vec3(0, 0, z_dist += 6));

		linerender.re_init_line(line);
		spline(splinepoints, line, 200, 2);

		// Initialize the maneuver angles
		vec3 maneuver_axis = vec3(0, 1, 0);
		mat4 upside_down = rotate(mat4(1.0), 180.0f, maneuver_axis);
		mat4 bank_right = rotate(mat4(1.0), -90.0f, maneuver_axis);
		mat4 bank_left = rotate(mat4(1.0), 90.0f, maneuver_axis);
		mat4 level = rotate(mat4(1.0), 0.0f, maneuver_axis);
		for (int i = 0; i < 4; i++)
			Marr.push_back(level);
		Marr.push_back(bank_right);
		Marr.push_back(upside_down);
		Marr.push_back(bank_left);
		for (int i = 0; i < 4; i++)
			Marr.push_back(level);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string &resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
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

		pShip = std::make_shared<Program>();
		pShip->setVerbose(true);
		pShip->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!pShip->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pShip->addUniform("P");
		pShip->addUniform("V");
		pShip->addUniform("M");
		pShip->addUniform("campos");
		pShip->addAttribute("vertPos");
		pShip->addAttribute("vertNor");
		pShip->addAttribute("vertTex");
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

		glm::mat4 V, M, P; //View, Model and Perspective matrix

		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = vec3(0, 0, 0);
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the sky using GLSL.
		psky->bind();
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		sphere->draw(psky); //render!!!!!!!
		glEnable(GL_DEPTH_TEST);
		psky->unbind();

		mycam.pos = vec3(0, 8, -8);
		V = glm::lookAt(mycam.pos, vec3(0, 0, 0), vec3(0, 1, 0));
		// Draw the plane using GLSL.
		static PlayerShip ship;

		glm::mat4 TransPlane = glm::translate(glm::mat4(1.0f), vec3(0, 0, 0));
		glm::mat4 SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(ship.scale));
		ship.update(mousePos, frametime);
		glm::mat4 Rot = glm::rotate(mat4(1.0), (float)ship.angle, vec3(0, 1, 0));

		M = TransPlane * SPlane * Rot;

		pShip->bind();
		glUniformMatrix4fv(pShip->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pShip->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pShip->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pShip->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		shipMesh->draw(pShip); //render!!!!!!!
		pShip->unbind();
		if (ship.state == Exploding)
		{
			double duration = 0.7;
			static double exp_time = 0;
			if (exp_time > duration)
			{
				ship.respawn();
				exp_time = 0.0;
			}
			else
			{
				pTile->bind();
				static vec2 offset = vec2(0.0, 0.0);
				double anim_time = (total_time / duration * 16.0);
				int frame_num = (int)anim_time;
				double t = anim_time - (double)frame_num;
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

				exp_time += frametime;
			}
		}

		prog->bind();
		M = glm::translate(mat4(1.0), vec3(0, 0.7, 1)) * glm::lookAt(vec3(0), mycam.pos, vec3(0, 1, 0)) * glm::scale(mat4(1), vec3(0.7));
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		plane->draw(prog);
		prog->unbind();

		for (auto &planet : planets)
		{
			planet.update(total_time);
			M = glm::translate(mat4(1.0), ship.pos - planet.pos) * glm::scale(mat4(1), vec3(planet.scale)) * planet.rotMat;
			if (ship.intersects(planet))
			{
				ship.state = Exploding;
			}

			pShip->bind();
			glUniformMatrix4fv(pShip->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pShip->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pShip->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform3fv(pShip->getUniform("campos"), 1, &mycam.pos[0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			sphere->draw(pShip);
			pShip->unbind();
		}

		for (auto &asteroid : asteroids)
		{
			asteroid.update(total_time);
			M = glm::translate(mat4(1.0), ship.pos - asteroid.pos) * glm::scale(mat4(1), vec3(asteroid.scale)) * glm::rotate(mat4(1.0), (float)(total_time * asteroid.rotScalar), vec3(0, 1, 0));
			if (ship.intersects(asteroid))
			{
				ship.state = Exploding;
			}

			pShip->bind();
			glUniformMatrix4fv(pShip->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(pShip->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(pShip->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform3fv(pShip->getUniform("campos"), 1, &mycam.pos[0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture2);
			asteroidMesh->draw(pShip);
			pShip->unbind();
		}

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
