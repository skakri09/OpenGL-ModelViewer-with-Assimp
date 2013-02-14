#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include <memory>

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>

#include "Timer.h"
#include "GLUtils/GLUtils.hpp"
#include "Model.h"

struct font_atlas{
	unsigned int texID;
	int width;
	int height;

	struct character{
		float ax; //size x
		float ay; //size y

		float bw; //bitmap width
		float bh; //bitmap height

		float bl; //bitmap left
		float bt; //bitmap top

		float tx; //texture offset x
		float ty; //texture offset y
	} characters[255];
};

/**
 * This class handles the game logic and display.
 * Uses SDL as the display manager, and glm for 
 * vector and matrix computations
 */
class GameManager {
public:

	/**
	 * Constructor
	 */
	GameManager();

	/**
	 * Destructor
	 */
	~GameManager();

	/**
	 * Initializes the game, including the OpenGL context
	 * and data required
	 */
	void init();

	/**
	 * The main loop of the game. Runs the SDL main loop
	 */
	void play();

	/**
	 * Quit function
	 */
	void quit();

	/**
	 * Function that handles rendering into the OpenGL context
	 */
	void render();

protected:
	/**
	 * Creates the OpenGL context using SDL
	 */
	void createOpenGLContext();

	/**
	 * Sets states for OpenGL that we want to keep persistent
	 * throughout the game
	 */
	void setOpenGLStates();

	/**
	 * Creates the matrices for the OpenGL transformations,
	 * perspective, etc.
	 */
	void createMatrices();

	/**
	 * Compiles, attaches, links, and sets uniforms for
	 * a simple OpenGL program
	 */
	void createSimpleProgram();

	void renderText(std::string text, font_atlas* atlas, float x, float y, float scalex, float scaley);

	void createAtlas(font_atlas* atlas, int font_size, std::string filename);



	static const unsigned int window_width = 800;
	static const unsigned int window_height = 600;

private:

	GLuint vbo; //< Vertex buffer object
	GLuint vao;

	//GLuint program; //< OpenGL shader program
	std::shared_ptr<GLUtils::Program> program;

	font_atlas atlas48;
	font_atlas atlas24;

	Timer my_timer; //< Timer for machine independent motion

	glm::mat4 projection_matrix; //< OpenGL projection matrix
	glm::mat4 model_matrix; //< OpenGL model transformation matrix
	glm::mat4 view_matrix; //< OpenGL camera/view matrix
	glm::mat3 normal_matrix; //< OpenGL matrix to transfor normals

	SDL_Window* main_window; //< Our window handle
	SDL_GLContext main_context; //< Our opengl context handle 
};

#endif // _GAMEMANAGER_H_
