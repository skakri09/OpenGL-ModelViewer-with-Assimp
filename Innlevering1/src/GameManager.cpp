#include "GameManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL_video.h>
using std::cerr;
using std::endl;
using GLUtils::VBO;
using GLUtils::Program;
using GLUtils::readFile;

GameManager::GameManager() 
{
	my_timer.restart();
}

GameManager::~GameManager() 
{
}

void GameManager::createOpenGLContext() 
{
	//Set OpenGL major an minor versions
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Use double buffering
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); // Use framebuffer with 16 bit depth buffer
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8); // Use framebuffer with 8 bit for red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); // Use framebuffer with 8 bit for green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8); // Use framebuffer with 8 bit for blue
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // Use framebuffer with 8 bit for alpha

	// Initalize video
	main_window = SDL_CreateWindow("NITH - PG612 Example OpenGL Program", 
									SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
									window_width, window_height, 
									SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	
	if (!main_window) 
	{
		THROW_EXCEPTION("SDL_CreateWindow failed");
	}

	//Create OpenGL context
	main_context = SDL_GL_CreateContext(main_window);
	trackball.setWindowSize(window_width, window_height);

	// Init glew
	// glewExperimental is required in openGL 3.3
	// to create forward compatible contexts 
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK) 
	{
		std::stringstream err;
		err << "Error initializing GLEW: " << glewGetErrorString(glewErr);
		THROW_EXCEPTION(err.str());
	}

	// Unfortunately glewInit generates an OpenGL error, but does what it's
	// supposed to (setting function pointers for core functionality).
	// Lets do the ugly thing of swallowing the error....
	glGetError();
}

void GameManager::setOpenGLStates() 
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	backgroundColor = glm::vec4(0.0, 0.0, 0.5, 1.0);
	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
}

void GameManager::createMatrices() 
{
	FoV = 45.0f;
	projection_matrix = glm::perspective(FoV,	window_width / (float) window_height, 1.0f, 10.f);
	model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(3));
	view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));

	fbo_projectionMatrix = glm::mat4(1);
	fbo_viewMatrix = glm::mat4(1);

	fbo_modelMatrix = glm::mat4(1);

	fbo_modelMatrix = glm::translate(fbo_modelMatrix, glm::vec3(0.7f, 0.7f, 0));
	fbo_modelMatrix = glm::scale(fbo_modelMatrix, glm::vec3(0.3));

	lightPosition = glm::vec3(0, 1, 5);
	lightProjection = glm::perspective(FoV, window_width/(float)window_height, 1.0f, 10.0f);
	lightView = glm::lookAt(lightPosition, glm::vec3(0), glm::vec3(0, 1, 0));
	totalTime = 0.0f;
}

void GameManager::createSimpleProgram() 
{
	//Loading and compiling all the shader programs we can use
	prog_phong = createProgram("shaders/phong.vert", "shaders/phong.frag");
	prog_flat = createProgram("shaders/flat.vert", "shaders/flat.frag");
	prog_wireframe = createProgram("shaders/wireframe.vert", "shaders/wireframe.frag");
	prog_hiddenLine = createProgram("shaders/hidden_line.vert", "shaders/hidden_line.frag");
	CHECK_GL_ERROR();
	prog_textured = createProgram("shaders/textured.vert", "shaders/textured.frag");

	CHECK_GL_ERROR();
	prog_text = createProgram("shaders/text.vert", "shaders/text.frag", false);
	renderMode = RENDERMODE_TEXTURED;
	oldRenderMode = NONE;
	current_program = prog_textured;	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GameManager::createVAO() 
{
	//Loading the model set by commandline argument as initial model, unless
	//there is nothing in the modelToLoad string. We then just load the bunny
	if(modelToLoad.size() > 0)
		LoadModel(modelToLoad);
	else
		LoadModel("models/bunny.obj");
}	 

void GameManager::LoadModel( std::string fullFilePath )
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if(modelToLoad.size() > 0)
		model.reset(new Model(fullFilePath, false));

	model->getInterleavedVBO()->bind();
	model->getIndexesVBO()->bindIndexes();
	oldRenderMode = NONE;

	//Unbind VBOs and VAO
	glBindVertexArray(0);
	CHECK_GL_ERROR();
}

void GameManager::init() 
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
	{
		std::stringstream err;
		err << "Could not initialize SDL: " << SDL_GetError();
		THROW_EXCEPTION(err.str());
	}

	atexit( SDL_Quit);

	createOpenGLContext();
	setOpenGLStates();
	createMatrices();
	CHECK_GL_ERROR();
	createSimpleProgram();
	CHECK_GL_ERROR();
	createVAO();
	CHECK_GL_ERROR();
	createFBOProgram();
	CHECK_GL_ERROR();
	createFBOVAO();
	CHECK_GL_ERROR();
	createFBO();
	CHECK_GL_ERROR();
	dirBrowser.reset(new DirectoryBrowser());
	dirBrowser->Init("models", this, prog_text);
	mouseState = new LeftMouseState();
	*mouseState = UP;
	mouseX = mouseY = 0.0f;
	SaveImagesToDisc = false;
	frameCounter = 0;
}

void GameManager::renderMeshRecursive(MeshPart& mesh, const std::shared_ptr<Program>& program, 
		const glm::mat4& view_matrix, const glm::mat4& model_matrix, RenderMode mode,
		const glm::mat4& light_view_matrix) 
{
	//Create modelview matrix
	glm::mat4 meshpart_model_matrix = model_matrix*mesh.transform;
	glm::mat4 modelview_matrix = view_matrix*meshpart_model_matrix;

	glm::mat4 light_modelview_matrix = light_view_matrix*meshpart_model_matrix;

	glUniformMatrix4fv(program->getUniform("modelview_matrix"), 1, 0, glm::value_ptr(modelview_matrix));
	glUniform3fv(program->getUniform("light_position"), 1, glm::value_ptr(lightPosition));

	glm::mat4 shadowMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	/*shadowMatrix += glm::mat4(0.5f, 0, 0, 0.5f,
							  0, 0.5f, 0, 0.5f,
							  0, 0, 0.5f*0.99f, 0.5f+0.01f,
							  0, 0, 0, 1);*/
	shadowMatrix = glm::scale(shadowMatrix, glm::vec3(0.5f)) * lightProjection * light_modelview_matrix;

	glUniformMatrix4fv(current_program->getUniform("shadow_matrix"), 1, 0, glm::value_ptr(shadowMatrix));

	///glUniformMatrix4fv(program->getUniform("model_matrix"), 1, 0, glm::value_ptr(light_modelview_matrix));

	if(mode == RENDERMODE_TEXTURED)
	{
		//If the rendermode is RENDERMODE_TEXTURED, we attempt to bind the texture which name is stored in the meshpart.
		//Currently this code only support one diffuse texture
		bool boundTexture = false;
		if(mesh.diffuseTextures.size() > 0 && mesh.texCoords0)
		{
			boundTexture = true;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TextureFactory::Inst()->GetTexture(mesh.diffuseTextures.at(0)));
		}

		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(modelview_matrix)));
		glUniformMatrix3fv(program->getUniform("normal_matrix"), 1, 0, glm::value_ptr(normal_matrix));
		
		glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));
		if(boundTexture)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}	
	else
	{
		//Create normal matrix, the transpose of the inverse
		//3x3 leading submatrix of the modelview matrix
		if(mode == RENDERMODE_PHONG || mode == RENDERMODE_FLAT)
		{
			glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(modelview_matrix)));
			glUniformMatrix3fv(program->getUniform("normal_matrix"), 1, 0, glm::value_ptr(normal_matrix));
		}

		//Drawing with indices, thus using drawElements instead of drawArrays
		glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));
	}
	

	for (unsigned int i=0; i<mesh.children.size(); ++i)
		renderMeshRecursive(mesh.children.at(i), program, view_matrix, meshpart_model_matrix, mode, light_view_matrix);
}

void GameManager::renderMeshRecursiveLight( MeshPart& mesh, const std::shared_ptr<GLUtils::Program>& program, 
											const glm::mat4& view_matrix, const glm::mat4& model_matrix )
{
	//Create modelview matrix
	glm::mat4 meshpart_model_matrix = model_matrix*mesh.transform;

	glm::mat4 modelview_matrix = view_matrix*meshpart_model_matrix;

	glUniformMatrix4fv(program->getUniform("modelview_matrix"), 1, 0, glm::value_ptr(modelview_matrix));

	glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));



	for (unsigned int i=0; i<mesh.children.size(); ++i)
		renderMeshRecursiveLight(mesh.children.at(i), program, view_matrix, meshpart_model_matrix);
}

void GameManager::render() 
{
	/*Rendering the lightPoV to our FBO*/
	RenderLightPoV();

	CHECK_GL_ERROR();
	/*Rendering the actual camera PoV to screen*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderCamPoV();

	CHECK_GL_ERROR();

	/*Rendering a dump of the depthBuffer from the lightPoV*/
	RenderDepthDump();

	CHECK_GL_ERROR();
	/*Rendering the directoryBrowser*/
	dirBrowser->RenderDirectoryBrowser(mouseX, mouseY, mouseState);
	
	if(SaveImagesToDisc)
		SaveImageToDisc(window_width, window_height, &frameCounter);

	CHECK_GL_ERROR();
}

void GameManager::RenderLightPoV()
{

	glm::mat4 view_matrix_new = lightView*trackball_view_matrix;

#pragma region fbo1
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	light_prog->use();

	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 4.4f);

	model->getInterleavedVBO()->bind();
	model->getIndexesVBO()->bindIndexes();
	light_prog->setAttributePointer("in_Position", 3, GL_FLOAT, GL_FALSE, model->getStride(), model->getVerticeOffset());

	glUniformMatrix4fv(light_prog->getUniform("projection"), 1, 0, glm::value_ptr(lightProjection));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	renderMeshRecursiveLight(model->getMesh(), light_prog, view_matrix_new, model_matrix);
	glDisable(GL_POLYGON_OFFSET_FILL);
	light_prog->disuse();
	glBindVertexArray(0);
#pragma endregion

#pragma region fbo2
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao);
	light_prog->use();

	model->getInterleavedVBO()->bind();
	model->getIndexesVBO()->bindIndexes();
	light_prog->setAttributePointer("in_Position", 3, GL_FLOAT, GL_FALSE, model->getStride(), model->getVerticeOffset());

	glUniformMatrix4fv(light_prog->getUniform("projection"), 1, 0, glm::value_ptr(lightProjection));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	renderMeshRecursiveLight(model->getMesh(), light_prog, view_matrix_new, model_matrix);

	light_prog->disuse();
	glBindVertexArray(0);

#pragma endregion

}

void GameManager::RenderCamPoV()
{
	glm::mat4 view_matrix_new = view_matrix*trackball_view_matrix;
	glm::mat4 _light_view_matrix_new = lightView*trackball_view_matrix;
	//Render geometry
	glBindVertexArray(vao);
	current_program->use();

	UpdateAttripPtrs();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);

	glUniformMatrix4fv(current_program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
	CHECK_GL_ERROR();
	glUniform1i(prog_textured->getUniform("diffuseMap_texture"), 0); //< 0 means GL_TEXTURE0

	CHECK_GL_ERROR();
	glUniform1i(current_program->getUniform("shadowMap_texture"), 1); //< 1 means GL_TEXTURE1

	if(renderMode == RENDERMODE_HIDDEN_LINE || renderMode == RENDERMODE_WIREFRAME)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(renderMode == RENDERMODE_HIDDEN_LINE)
		RenderHiddenLine(view_matrix_new, lightView);
	else
		renderMeshRecursive(model->getMesh(), current_program, view_matrix_new, model_matrix, renderMode, _light_view_matrix_new);

	current_program->disuse();
	glBindVertexArray(0);

	if(renderMode == RENDERMODE_HIDDEN_LINE || renderMode == RENDERMODE_WIREFRAME)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::RenderDepthDump()
{
	fbo_program->use();

	//Bind the textures before rendering
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_texture2);

	glBindVertexArray(fbo_vao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();

	//Unbind the textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::play() 
{
	bool doExit = false;
	float fps = 0.0f;
	float fpsTimer = 0.0f;
	bool hasBeenDirBrowsingMode = false;
	//SDL main loop
	while (!doExit) 
	{

		SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{// poll for pending events
			switch (event.type) 
			{
			case SDL_MOUSEWHEEL:
				if (event.wheel.y > 0 )
					ZoomIn();
				else if (event.wheel.y < 0 )
					ZoomOut();
				break;
			case SDL_MOUSEBUTTONDOWN:
				trackball.rotateBegin(event.motion.x, event.motion.y);
				if(event.button.button == SDL_BUTTON_LEFT)
					if(*mouseState != DOWN)
						*mouseState = FIRST_DOWN;
				break;
			case SDL_MOUSEBUTTONUP:
				trackball.rotateEnd(event.motion.x, event.motion.y);
				if(event.button.button == SDL_BUTTON_LEFT)
					if(*mouseState != UP)
						*mouseState = FIRST_UP;
				break;
			case SDL_MOUSEMOTION:
				mouseX = static_cast<float>(event.motion.x);
				mouseY = static_cast<float>(event.motion.y);
				trackball_view_matrix = trackball.rotate(event.motion.x, event.motion.y, 1.0f);
				break;
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_TAB && hasBeenDirBrowsingMode == false)
				{
					hasBeenDirBrowsingMode = true;
					dirBrowser->ToggleDirectoryBrowser();
				}
				if (event.key.keysym.sym == SDLK_ESCAPE) //Esc
					doExit = true;
				if (event.key.keysym.sym == SDLK_q
						&& event.key.keysym.mod & KMOD_CTRL) //Ctrl+q
					doExit = true;
				if(event.key.keysym.sym == SDLK_PAGEUP)
					ZoomIn();
				if(event.key.keysym.sym == SDLK_PAGEDOWN)
					ZoomOut();
				if(event.key.keysym.sym == SDLK_p)
				{
					if(!SaveImagesToDisc)
					{
						SaveImagesToDisc = true;
						videoRecorder.PrepVideoRecording(window_width, window_height, 3, 30, 5);
					}
				}

				DetermineRenderMode(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				if(event.key.keysym.sym == SDLK_TAB)
					hasBeenDirBrowsingMode = false;
				break;
			case SDL_QUIT: //e.g., user clicks the upper right x	
				doExit = true;
				break;
			}
		}
		//Render, and swap front and back buffers
		render();
		SDL_GL_SwapWindow(main_window);

		RotateLight();
		deltaTime = static_cast<float>(my_timer.elapsedAndRestart());
		fpsTimer += deltaTime;
		if(fpsTimer >= 0.3f)//updating the fps counter once every .3sec
		{
			fps = 1/deltaTime;
			std::ostringstream captionStream;		
			captionStream << "FPS: " << fps;
			SDL_SetWindowTitle(main_window, captionStream.str().c_str());
			fps = 0;
			fpsTimer = 0;
		}
	}
	quit();
}

void GameManager::quit() 
{
	std::cout << "Bye bye..." << std::endl;
}

void GameManager::ZoomIn()
{
	if(FoV > 10.0f)
		FoV -= 5.0f;
	else if(FoV > 3.0f)
		FoV -= 1.0f;
	else FoV -= 0.1f;
	
	if(FoV < 0.0001f)
		FoV = 0.0001f;

	projection_matrix = glm::perspective(FoV,	window_width / (float) window_height, 1.0f, 10.f);
}

void GameManager::ZoomOut()
{
	if(FoV > 10.0f)
		FoV += 5.0f;
	else if(FoV > 3.0f)
		FoV += 1.0f;
	else FoV += 0.1f;

	if(FoV > 179.999f)
		FoV = 179.999f;

	projection_matrix = glm::perspective(FoV,	window_width / (float) window_height, 1.0f, 10.f);
}

void GameManager::DetermineRenderMode(SDL_Keycode keyCode)
{
	RenderMode currentRendermode = renderMode;
	//Phong shading
	switch(keyCode)
	{
	case SDLK_1:
		renderMode = RENDERMODE_PHONG;
		current_program = prog_phong;
		break;
	case SDLK_2:
		renderMode = RENDERMODE_FLAT;
		current_program = prog_flat;
		break;
	case SDLK_3:
		renderMode = RENDERMODE_WIREFRAME;
		current_program = prog_wireframe;
		break;
	case SDLK_4:
		renderMode = RENDERMODE_HIDDEN_LINE;
		current_program = prog_hiddenLine;
		break;
	case SDLK_5:
		renderMode = RENDERMODE_TEXTURED;
		current_program = prog_textured;
		break;
	}
}

std::shared_ptr<GLUtils::Program> GameManager::createProgram( std::string vs_path, std::string fs_Path, bool setProjM)
{
	std::shared_ptr<GLUtils::Program> program;
	std::string fs_src = readFile(fs_Path);
	std::string vs_src = readFile(vs_path);

	//Compile shaders, attach to program object, and link
	program.reset(new Program(vs_src, fs_src));

	//Set uniforms for the program.
	program->use();
	if(setProjM)
		glUniformMatrix4fv(program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
	program->disuse();
	return program;
}

void GameManager::SetModelToLoad( std::string modelPath )
{
	modelToLoad = modelPath;
}

void GameManager::UpdateAttripPtrs()
{
	//if(oldRenderMode != renderMode)
	{
		oldRenderMode = renderMode;	
		model->getInterleavedVBO()->bind();
		model->getIndexesVBO()->bindIndexes();
		current_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, model->getStride(), model->getVerticeOffset());

		if(renderMode == RENDERMODE_PHONG || renderMode == RENDERMODE_FLAT || renderMode == RENDERMODE_TEXTURED)
			current_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, model->getStride(), model->getNormalOffset());

		if(renderMode == RENDERMODE_TEXTURED)
		{
			current_program->setAttributePointer("textureCoord0", 2, GL_FLOAT, GL_FALSE, model->getStride(), model->getTexCoordOffset());
		}
	}
}

void GameManager::RenderHiddenLine(glm::mat4 view_matrix_new, const glm::mat4& light_model_view_matrix)
{
	//IF its hidden_line rendermode, we do some polygon offset stuff here and in the next if check below
	if(renderMode == RENDERMODE_HIDDEN_LINE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 4.4f);
		glUniform4fv(current_program->getUniform("rendering_color"), 1, glm::value_ptr(backgroundColor));
	}

	renderMeshRecursive(model->getMesh(), current_program, view_matrix_new, model_matrix, renderMode, light_model_view_matrix);

	//Rendering the model a second time with some polygon offset magic if its hidden_line rendering
	if(renderMode == RENDERMODE_HIDDEN_LINE)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);

		glUniform4fv(current_program->getUniform("rendering_color"), 1, glm::value_ptr(glm::vec4(1.0f, 0.6f, 0.1f, 1.0f)));
		glEnable (GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonOffset(0.0f, 0.0f);
		renderMeshRecursive(model->getMesh(), current_program, view_matrix_new, model_matrix, renderMode, light_model_view_matrix);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}
}

void GameManager::createFBOProgram()
{
	std::string vs_src = readFile("shaders/fbo.vert");
	std::string fs_src = readFile("shaders/fbo.frag");

	fbo_program.reset(new Program(vs_src, fs_src));
	fbo_program->use();

	glUniformMatrix4fv(fbo_program->getUniform("projection"), 1, 0, glm::value_ptr(fbo_projectionMatrix));
	glUniformMatrix4fv(fbo_program->getUniform("view"), 1, 0, glm::value_ptr(fbo_viewMatrix));
	glUniformMatrix4fv(fbo_program->getUniform("model_matrix"), 1, 0, glm::value_ptr(fbo_modelMatrix));
	glUniform1i(fbo_program->getUniform("fbo_texture"), 0);
	glUseProgram(0);
	CHECK_GL_ERROR();

	std::string vs_lsrc = readFile("shaders/lightPoV.vert");
	std::string fs_lsrc = readFile("shaders/lightPoV.frag");
	light_prog.reset(new Program(vs_lsrc, fs_lsrc));
	light_prog->use();
	glUniformMatrix4fv(light_prog->getUniform("projection"), 1, 0, glm::value_ptr(lightProjection));
	light_prog->disuse();
	CHECK_GL_ERROR();
}

void GameManager::createFBOVAO()
{
	glGenVertexArrays(1, &fbo_vao);
	glBindVertexArray(fbo_vao);

	static float positions[8] = {
		-1.0, 1.0,
		-1.0, -1.0,
		1.0, 1.0,
		1.0, -1.0,

	};

	glGenBuffers(1, &fbo_vertex_bo);
	glBindBuffer(GL_ARRAY_BUFFER, fbo_vertex_bo);
	glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(float), &positions[0], GL_STATIC_DRAW);

	fbo_program->setAttributePointer("in_Position", 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GameManager::createFBO()
{
#pragma region fbo1
	glGenTextures(1, &fbo_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo_texture, 0);
	TextureFactory::Inst()->SaveTexture("fbo_texture", fbo_texture);
	////depth render buffer
	//glGenRenderbuffers(1, &fbo_depth);
	//glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	////create an fbo and attach texture and render buffer
	//glGenFramebuffers(1, &fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_FBO_COMPLETENESS();
#pragma endregion

#pragma region fbo2
	glGenTextures(1, &fbo_texture2);
	glBindTexture(GL_TEXTURE_2D, fbo_texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);

	glGenFramebuffers(1, &fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo_texture2, 0);
	TextureFactory::Inst()->SaveTexture("fbo_texture2", fbo_texture2);
	////depth render buffer
	//glGenRenderbuffers(1, &fbo_depth);
	//glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	////create an fbo and attach texture and render buffer
	//glGenFramebuffers(1, &fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_FBO_COMPLETENESS();

#pragma endregion
}

void GameManager::RotateLight()
{
	totalTime += (float)my_timer.elapsed();
	lightPosition = glm::vec3(sin(totalTime)*2, 1, 5+cos(totalTime));
	lightView = glm::lookAt(lightPosition, glm::vec3(0), glm::vec3(0, 1, 0));
}

void GameManager::SaveImageToDisc( unsigned int window_width, unsigned int window_height, 
									unsigned int* frameCounter, std::string format /*= ".bmp"*/ )
{

	SaveImagesToDisc = videoRecorder.StoreFrame(deltaTime);
	/*std::vector<unsigned char> pixelData;
	pixelData.resize(window_width*window_height*3);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, &pixelData[0]);
	ILuint ImageName;

	ilGenImages(1, &ImageName); 
	ilBindImage(ImageName);
	ilTexImage(window_width, window_height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &pixelData[0]);

	std::stringstream str;
	std::string path = "video/frame";
	str<<path<<++*frameCounter<<format;*/

	//ilSaveImage(str.str().c_str());
}












