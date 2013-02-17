/********************************************************************
    created:    16:2:2013   14:27
    filename:   TextRenderer.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef TextRenderer_h__
#define TextRenderer_h__

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <iostream>

#include "GLUtils/Program.hpp"
#include "GLUtils/VBO.hpp"
#include "GLUtils/GlUtils.hpp"
#include "GameException.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
using std::cerr;
using std::endl;
using GLUtils::VBO;
using GLUtils::Program;
using GLUtils::readFile;

struct font_atlas
{
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

struct Text
{
	Text()
	{
		x = y = w = h = 0.0f;
	}
	float x;
	float y;
	float w;
	float h;
	std::string text;
	bool Contains(glm::vec2 normalizedMouseCoords)
	{
		if(normalizedMouseCoords.x > x
		&& normalizedMouseCoords.x < x+w
		&& normalizedMouseCoords.y > y 
		&& normalizedMouseCoords.y < y+h)
		return true;

		return false;
	}
	
	glm::vec4 color;

	std::shared_ptr<GLUtils::VBO> vbo;
	std::shared_ptr<font_atlas> atlas;
	int count;
};


class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();

	void InitTextRenderer(std::shared_ptr<Program> textProgram);

	Text RenderText(std::string text, std::string font, float xPos, float yPos, 
					float scaleX, float scaleY, glm::vec4 color);

	void RenderText(Text text);

	Text GenerateText(std::string text, std::string font, float xPos, float yPos, 
		float scaleX, float scaleY, glm::vec4 color);

protected:

private:
	std::map<std::string, std::shared_ptr<font_atlas>> fontMap;
	std::shared_ptr<font_atlas> GetFontAtlas(std::string key);

	std::shared_ptr<Program> program;

	void CreateAndStoreFontAtlas(std::string fontFilePath, std::string fontKey, FT_UInt fontSize);

	std::shared_ptr<TextRenderer> textRenderer;

	std::shared_ptr<GLUtils::VBO> vbo;

	GLuint vao;
};

#endif // TextRenderer_h__