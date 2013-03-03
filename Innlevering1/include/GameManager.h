#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_
#pragma warning( disable : 4345 )
#include <memory>

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>

#include "Timer.h"
#include "GLUtils/GLUtils.hpp"
#include "Model.h"
#include "VirtualTrackball.h"

#include "FileHandler.h"
#include "DirectoryBrowser.h"
/**
 * This class handles the game logic and display.
 * Uses SDL as the display manager, and glm for 
 * vector and matrix computations
 */

struct Light{
	glm::vec3 position;
	glm::mat4 projection;
	glm::mat4 view;
};

class DirectoryBrowser;
enum LeftMouseState;
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

	unsigned int WindowWidth(){return window_width;}
	unsigned int WindowHeight(){return window_height;}
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
	Light sceneLight;


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

	void renderMeshRecursive(MeshPart& mesh, const std::shared_ptr<GLUtils::Program>& program, 
									const glm::mat4& view_matrix, const glm::mat4& model_matrix, RenderMode mode,
									const glm::mat4& light_view_matrix);

	void DetermineRenderMode(SDL_Keycode keyCode);

	void UpdateAttripPtrs();

	void RenderHiddenLine(glm::mat4 view_matrix_new, const glm::mat4& light_model_view_matrix);

	GLuint vao; //< Vertex array object
	
	//The various shader programs we can use
	std::shared_ptr<GLUtils::Program> current_program; //<A pointer to the program currently in use
	std::shared_ptr<GLUtils::Program> prog_phong;
	std::shared_ptr<GLUtils::Program> prog_flat;
	std::shared_ptr<GLUtils::Program> prog_wireframe;
	std::shared_ptr<GLUtils::Program> prog_hiddenLine;
	std::shared_ptr<GLUtils::Program> prog_textured;
	std::shared_ptr<GLUtils::Program> prog_text;
	std::shared_ptr<GLUtils::Program> fbo_program;
	std::shared_ptr<GLUtils::Program> light_prog;
	/*
	* Helper function to create a shader program. The two parameters are paths to the shader txt files
	*/
	std::shared_ptr<GLUtils::Program> createProgram(std::string vs_path, std::string fs_Path, bool setProjM = true);

	std::shared_ptr<Model> model; //< The currently loaded model
	
	std::shared_ptr<Model> scene; //< The Scene
	glm::mat4 scene_model_matrix;

	Timer my_timer; //< Timer for machine independent motion
	float deltaTime;//< Game deltatime variable
	glm::mat4 projection_matrix; //< OpenGL projection matrix
	glm::mat4 model_matrix; //< OpenGL model transformation matrix
	glm::mat4 view_matrix; //< OpenGL camera/view matrix
	glm::mat3 normal_matrix; //< OpenGL matrix to transfor normals
	glm::mat4 trackball_view_matrix; //< OpenGL camera matrix for the trackball

	//Variables for the FBO 
	GLuint fbo; //< Frame buffer object id
	GLuint fbo_vao; //< Vertex array object for our FBO geometry (full-screen quad)
	GLuint fbo_vertex_bo; //< Vetex buffer object for fullscreen quad
	GLuint fbo_texture; //< framebuffer object color attachment (texture)
	GLuint fbo_depth; //< framebuffer object depth attachment (renerbuffer)
	
	glm::mat4 fbo_modelMatrix;
	glm::mat4 fbo_projectionMatrix;
	glm::mat4 fbo_viewMatrix;

	void RenderLightPoV();
	void RenderCamPoV();
	void RenderDepthDump();
	
	static void renderMeshRecursiveLight(MeshPart& mesh, const std::shared_ptr<GLUtils::Program>& program, 
										const glm::mat4& view_matrix, const glm::mat4& model_matrix);

	glm::vec3 lightPosition;
	glm::mat4 lightProjection;
	glm::mat4 lightView;

	/**
	 * Compiles, attaches, links, and sets uniforms for
	 * the OpenGL program that renders the texture to the
	 * fullscreen quad
	 */
	void createFBOProgram();
	
	/**
	 * Creates vertex array objects for the fullscreen
	 * quad used to render the FBO texture to screen
	 */
	void createFBOVAO();

	/**
	  * Creates the framebuffer object
	  */
	void createFBO();

	VirtualTrackball trackball;
	SDL_Window* main_window; //< Our window handle
	SDL_GLContext main_context; //< Our opengl context handle 

	float FoV;

	std::string modelToLoad;

	//the FileHandler object handles input for file loading from the console window
	std::shared_ptr<DirectoryBrowser> dirBrowser;

	glm::vec4 backgroundColor;//<the color openGL clears the background with each update

	float mouseX, mouseY;

	LeftMouseState* mouseState;
};

#endif // _GAMEMANAGER_H_
