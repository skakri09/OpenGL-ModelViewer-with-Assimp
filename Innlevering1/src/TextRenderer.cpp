#include "TextRenderer.h"


TextRenderer::TextRenderer()
{
	
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::CreateAndStoreFontAtlas( std::string fontFilePath, std::string fontKey, FT_UInt fontSize )
{
	std::shared_ptr<font_atlas> atlas = std::make_shared<font_atlas>();
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) {
		std::stringstream err;
		err << "Error initializing FreeType";
		THROW_EXCEPTION(err.str());
	}

	FT_Face face;
	if(FT_New_Face(ft, fontFilePath.c_str(), 0, &face)){
		std::stringstream err;
		err << "Error opening fontfile";
		THROW_EXCEPTION(err.str());
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
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

	fontMap[fontKey] = atlas;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::InitTextRenderer( std::shared_ptr<Program> textProgram )
{
	this->program = textProgram;
//	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	CreateAndStoreFontAtlas("fonts/calibri.ttf", "calibri", 60);
}


Text TextRenderer::RenderText( std::string text, std::string font, float xPos, float yPos, 
								float scaleX, float scaleY, glm::vec4 color )
{
	std::shared_ptr<font_atlas> atlas = GetFontAtlas(font);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas->texID);
	program->use();
	glUniform1i(program->getUniform("tex"), 0);


	std::vector<float> coords;
	coords.resize(text.length() * 6 * 4);

	Text retText;
	retText.x = xPos + atlas->characters[text.at(0)].bl * scaleX;
	retText.y = yPos;
	int count = 0;
	for(unsigned int i = 0; i < text.length(); i++){
		unsigned char c = text.at(i);

		float x2 = xPos + atlas->characters[c].bl * scaleX;
		float y2 = -yPos - atlas->characters[c].bt * scaleY;
		float w = atlas->characters[c].bw * scaleX;
		float h = atlas->characters[c].bh * scaleY;
		
		if(retText.h < h)
			retText.h = h;

		xPos += atlas->characters[c].ax * scaleX;
		yPos += atlas->characters[c].ay * scaleY;

		// skip glyphs with no pixels
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
	retText.w = xPos - retText.x;
	retText.vbo.reset(new GLUtils::VBO(coords.data(), coords.size()*sizeof(float)));
	retText.color = color;
	retText.atlas = atlas;
	retText.count = count;
	retText.text = text;

	glBindVertexArray(vao);

	vbo.reset(new GLUtils::VBO(coords.data(), coords.size()*sizeof(float)));
	vbo->bind();
	program->setAttributePointer("in_Position", 4);
	
	glUniform4f(program->getUniform("color"), color.r, color.g, color.b, color.a);

	//glDrawArrays(GL_TRIANGLES, 0, count / 4);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	program->disuse();

	CHECK_GL_ERROR();
	return retText;
}

void TextRenderer::RenderText( Text text )
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, text.atlas->texID);

	program->use();
	glUniform1i(program->getUniform("tex"), 0);

	glBindVertexArray(vao);

	text.vbo->bind();
	
	program->setAttributePointer("in_Position", 4);

	glUniform4fv(program->getUniform("color"), 1,  glm::value_ptr(text.color));
	glDrawArrays(GL_TRIANGLES, 0, text.count / 4);


	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	text.vbo->unbind();
}

Text TextRenderer::GenerateText( std::string text, std::string font, float xPos, float yPos, float scaleX, float scaleY, glm::vec4 color )
{
	Text newText = RenderText(text, font, xPos, yPos, scaleX, scaleY, color);

	return newText;
}

std::shared_ptr<font_atlas> TextRenderer::GetFontAtlas( std::string key )
{
	if(fontMap.find(key) != fontMap.end())
		return fontMap.find(key)->second;
	else
		THROW_EXCEPTION("Font not initialized: " + key);
}

