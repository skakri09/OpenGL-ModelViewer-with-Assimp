#include "GameManager.h"
#include "GeometryManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLUtils/GlUtils.hpp"
#include "GameException.h"

#include <ft2build.h>
#include FT_FREETYPE_H

using std::cerr;
using std::endl;
using GLUtils::VBO;
using GLUtils::Program;
using GLUtils::readFile;

GameManager::GameManager() {
	my_timer.restart();
}

GameManager::~GameManager() {
}

void GameManager::createOpenGLContext() {
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
	main_window = SDL_CreateWindow("NITH - PG612 Example OpenGL Program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!main_window) {
		THROW_EXCEPTION("SDL_CreateWindow failed");
	}

	//Create OpenGL context
	main_context = SDL_GL_CreateContext(main_window);

	// Init glew
	// glewExperimental is required in openGL 3.1 
	// to create forward compatible contexts 
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK) {
		std::stringstream err;
		err << "Error initializing GLEW: " << glewGetErrorString(glewErr);
		THROW_EXCEPTION(err.str());
	}

	// Unfortunately glewInit generates an OpenGL error, but does what it's
	// supposed to (setting function pointers for core functionality).
	// Lets do the ugly thing of swallowing the error....
	glGetError();
}

void GameManager::setOpenGLStates() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0, 0.0, 0.5, 1.0);
}

void GameManager::createMatrices() {
	projection_matrix = glm::perspective(45.0f,
			window_width / (float) window_height, 1.0f, 10.0f);
	model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(3));
	view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

void GameManager::createSimpleProgram() {
	std::string fs_src = readFile("shaders/test.frag");
	std::string vs_src = readFile("shaders/test.vert");

	//Compile shaders, attach to program object, and link
	program.reset(new Program(vs_src, fs_src));

	//Set uniforms for the program.
//	program->use();
//	glUniformMatrix4fv(program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
//	program->disuse();
}

void GameManager::renderText(std::string text, font_atlas* atlas, float x, float y, float scalex, float scaley) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas->texID);
	program->use();
	glUniform1i(program->getUniform("tex"), 0);


	std::vector<float> coords;
	coords.resize(text.length() * 6 * 4);

	int count = 0;

	for(int i = 0; i < text.length(); i++){
		unsigned char c = text.at(i);

		float x2 = x + atlas->characters[c].bl * scalex;
		float y2 = -y - atlas->characters[c].bt * scaley;
		float w = atlas->characters[c].bw * scalex;
		float h = atlas->characters[c].bh * scaley;

		x += atlas->characters[c].ax * scalex;
		y += atlas->characters[c].ay * scaley;

		// skip glyphs with no pixlez
		if(!w || !h){
			continue;
		}

		coords[count++] = x2;
		coords[count++] = -y2;
		coords[count++] = atlas->characters[c].tx;
		coords[count++] = atlas->characters[c].ty;

		coords[count++] = x2;
		coords[count++] = -y2 - h;
		coords[count++] = atlas->characters[c].tx;
		coords[count++] = atlas->characters[c].ty + atlas->characters[c].bh / atlas->height;

		coords[count++] = x2 + w;
		coords[count++] = -y2;
		coords[count++] = atlas->characters[c].tx + atlas->characters[c].bw / atlas->width;
		coords[count++] = atlas->characters[c].ty;

		coords[count++] = x2 + w;
		coords[count++] = -y2;
		coords[count++] = atlas->characters[c].tx + atlas->characters[c].bw / atlas->width;
		coords[count++] = atlas->characters[c].ty;

		coords[count++] = x2;
		coords[count++] = -y2 - h;
		coords[count++] = atlas->characters[c].tx;
		coords[count++] = atlas->characters[c].ty + atlas->characters[c].bh / atlas->height;

		coords[count++] = x2 + w;
		coords[count++] = -y2 - h;
		coords[count++] = atlas->characters[c].tx + atlas->characters[c].bw / atlas->width;
		coords[count++] = atlas->characters[c].ty + atlas->characters[c].bh / atlas->height;
	}
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, &coords[0], GL_DYNAMIC_DRAW);

	program->setAttributePointer("in_Position", 4);

	glDrawArrays(GL_TRIANGLES, 0, count / 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	program->disuse();
	CHECK_GL_ERROR();
}

void GameManager::createAtlas(font_atlas* atlas, int font_size,  std::string filename){
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) {
		std::stringstream err;
		err << "Error initializing FreeType";
		THROW_EXCEPTION(err.str());
	}

	FT_Face face;
	if(FT_New_Face(ft, filename.c_str(), 0, &face)){
		std::stringstream err;
		err << "Error opening fontfile";
		THROW_EXCEPTION(err.str());
	}
	
	FT_Set_Pixel_Sizes(face, 0, font_size);
	FT_GlyphSlot g = face->glyph;

	int row_w = 0;
	int row_h = 0;
	atlas->height = 0;
	atlas->width = 0;

	memset(atlas->characters, 0, sizeof(atlas->characters));

	// find minimum size for texture holding all visible ASCII characters
	for(int i = 0; i < 255; i++){
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)){
			std::cout << "Failed to load character: " << (unsigned char) i << std::endl;
			continue;
		}
		if(row_w + g->bitmap.width + 1 >= 1024) {
			atlas->width = std::max(atlas->width, row_w);
			atlas->height += row_h;
			row_w = 0;
			row_h = 0;
		}
		row_w += g->bitmap.width + 1;
		row_h = std::max(row_h, g->bitmap.rows);
	}

	atlas->width = std::max(atlas->width, row_w);
	atlas->height += row_h;

	// create GL texture to hold all glyphs
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &atlas->texID);
	glBindTexture(GL_TEXTURE_2D, atlas->texID);

	program->use();
	glUniform1i(program->getUniform("tex"), 0);
	program->disuse();
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// glyph offset
	int ox = 0; 
	int oy = 0;

	row_h = 0;

	for(int i = 0; i < 255; i++){
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)){
			std::cout << "Failed to load character: " << (unsigned char) i << std::endl;
			continue;
		}

		if(ox + g->bitmap.width + 1 >= 1024){
			oy += row_h;
			row_h = 0;
			ox = 0;
		}
		
		glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, (void*)g->bitmap.buffer);

		atlas->characters[i].ax = (float)(g->advance.x >> 6);
		atlas->characters[i].ay = (float)(g->advance.y >> 6);

		atlas->characters[i].bw = (float)g->bitmap.width;
		atlas->characters[i].bh = (float)g->bitmap.rows;

		atlas->characters[i].bl = (float)g->bitmap_left;
		atlas->characters[i].bt = (float)g->bitmap_top;

		atlas->characters[i].tx = ox / (float) atlas->width;
		atlas->characters[i].ty = oy / (float) atlas->height;

		row_h = std::max(row_h, g->bitmap.rows);
		ox += g->bitmap.width + 1;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::init() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::stringstream err;
		err << "Could not initialize SDL: " << SDL_GetError();
		THROW_EXCEPTION(err.str());
	}
	atexit(SDL_Quit);

	createOpenGLContext();
	setOpenGLStates();
	createMatrices();
	createSimpleProgram();

	glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	/*glBindBuffer(GL_VERTEX_ARRAY, vbo);
	program->use();
	program->setAttributePointer("in_Position", 4);
	program->disuse();
	glBindBuffer(GL_VERTEX_ARRAY, 0);

	glBindVertexArray(0);*/
	

	createAtlas(&atlas24, 24, "fonts/calibri.ttf");
	createAtlas(&atlas48, 48, "fonts/calibri.ttf");
}

void GameManager::render() {
	//Get elapsed time
	double elapsed = my_timer.elapsedAndRestart();
	//Clear screen, and set the correct program
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float sx = 2.0 / window_width;
	float sy = 2.0 / window_height;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	program->use();
	glUniform4f(program->getUniform("color"), 1.0f, 1.0f, 1.0f, 1.0f);
	program->disuse();

	renderText("Testuuuurrrrr", &atlas48, 0, 0, sx, sy);

	program->use();
	glUniform4f(program->getUniform("color"), 1.0f, 1.0f, 0.0f, 1.0f);
	program->disuse();

	renderText("Testuuuurrrrr22222", &atlas48, -0.5, 0.5, sx, sy);

	renderText("Testuuuåøæurrrrr33333333", &atlas24, -1.0, 0.9, sx, sy);

	program->use();
	glUniform4f(program->getUniform("color"), 1.0f, 0.0f, 1.0f, 1.0f);
	program->disuse();

	renderText("Testuuuurrrrr444444", &atlas24, 0.5, -1.0, sx, sy);
	
	
	CHECK_GL_ERROR();
}

void GameManager::play() {
	bool doExit = false;

	//SDL main loop
	while (!doExit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {// poll for pending events
			switch (event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
					doExit = true;
					break;
				case SDLK_q:
					if (event.key.keysym.mod & KMOD_CTRL) doExit = true; //Ctrl+q
					break;
				}
				break;
			case SDL_QUIT: //e.g., user clicks the upper right x
				doExit = true;
				break;
			}
		}

		//Render, and swap front and back buffers
		render();
		
		SDL_GL_SwapWindow(main_window);
	}
	quit();
}

void GameManager::quit() {
	std::cout << "Bye bye..." << std::endl;
}
