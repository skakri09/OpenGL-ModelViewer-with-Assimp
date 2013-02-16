#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include <memory>

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>

#include "Timer.h"
#include "GLUtils/GLUtils.hpp"
#include "Model.h"
#include "VirtualTrackball.h"

#include "FileHandler.h"

/**
 * This class handles the game logic and display.
 * Uses SDL as the display manager, and glm for 
 * vector and matrix computations
 */

class FileHandler;//<forward declaration for FileHandler class

class GameManager 
{
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

	/*
	* Function to set the path of the model the program should load
	*/
	void SetModelToLoad(std::string modelPath);

	/*
	* Loads a new model
	*/
	void LoadModel(std::string fullFilePath);

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

	/**
	 * Creates vertex array objects
	 */
	void createVAO();

	static const unsigned int window_width = 1280;
	static const unsigned int window_height = 720;

	void ZoomIn();

	void ZoomOut();

private:
	enum RenderMode
	{
		RENDERMODE_PHONG,
		RENDERMODE_FLAT,
		RENDERMODE_WIREFRAME,
		RENDERMODE_HIDDEN_LINE,
		RENDERMODE_TEXTURED,
		NONE
	};

	RenderMode renderMode, oldRenderMode;

	static void renderMeshRecursive(MeshPart& mesh, const std::shared_ptr<GLUtils::Program>& program, 
									const glm::mat4& modelview, const glm::mat4& transform, RenderMode mode);

	void DetermineRenderMode(SDL_Keycode keyCode);

	void UpdateAttripPtrs();

	GLuint vao; //< Vertex array object
	
	//GLuint program; //< OpenGL shader program
	std::shared_ptr<GLUtils::Program> current_program;
	std::shared_ptr<GLUtils::Program> prog_phong;
	std::shared_ptr<GLUtils::Program> prog_flat;
	std::shared_ptr<GLUtils::Program> prog_wireframe;
	std::shared_ptr<GLUtils::Program> prog_hiddenLine;
	std::shared_ptr<GLUtils::Program> prog_textured;
	std::shared_ptr<GLUtils::Program> createProgram(std::string vs_path, std::string fs_Path);

	std::shared_ptr<Model> model;

	Timer my_timer; //< Timer for machine independent motion

	glm::mat4 projection_matrix; //< OpenGL projection matrix
	glm::mat4 model_matrix; //< OpenGL model transformation matrix
	glm::mat4 view_matrix; //< OpenGL camera/view matrix
	glm::mat3 normal_matrix; //< OpenGL matrix to transfor normals
	glm::mat4 trackball_view_matrix; //< OpenGL camera matrix for the trackball
	
	VirtualTrackball trackball;
	SDL_Window* main_window; //< Our window handle
	SDL_GLContext main_context; //< Our opengl context handle 

	float FoV;

	std::string modelToLoad;

	//the FileHandler object handles input for file loading from the console window
	std::shared_ptr<FileHandler> fileHandler;

	glm::vec4 backgroundColor;//<the color openGL clears the background with each update
};

#endif // _GAMEMANAGER_H_
