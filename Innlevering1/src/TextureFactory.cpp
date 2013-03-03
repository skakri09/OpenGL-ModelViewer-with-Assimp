#include "TextureFactory.h"
#include "GameException.h"

TextureFactory::TextureFactory()
{
	ilInit();
	iluInit();
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilEnable(IL_ORIGIN_SET);
}

TextureFactory::~TextureFactory()
{
}

TextureFactory* TextureFactory::Inst()
{
	static TextureFactory* instance = new TextureFactory();
	return instance;
}

GLint TextureFactory::GetTexture( std::string textureName )
{
	if(textureMap.find(textureName) != textureMap.end())
		return textureMap.find(textureName)->second;
	else
		return LoadTexture(textureName);
}

GLint TextureFactory::LoadTexture( std::string texturePath )
{
	if(textureMap.find(texturePath) != textureMap.end())
		return textureMap.find(texturePath)->second;
	else
	{
		std::vector<unsigned char> data;
		ILuint ImageName;
		unsigned int width, height;
		GLuint texture;

		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);

		if (!ilLoadImage(texturePath.c_str())) {
			ILenum e;
			std::stringstream error;
			while ((e = ilGetError()) != IL_NO_ERROR) {
				error << e << ": " << iluErrorString(e) << std::endl;
			}
			ilDeleteImages(1, &ImageName);
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		data.resize(width*height*3);

		unsigned int components = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
		if(components == 3)
			ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data.data());
		else if(components == 4)
			ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data.data());
		//The line above SHOULD be using IL_RGBA, but something does not seem to be right with the 
		//ilGetComponents, as some images return 4, but still crash when using RGBA.

		ilDeleteImages(1, &ImageName);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if(components == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glBindTexture(GL_TEXTURE_2D, textureMap.size());
		CHECK_GL_ERROR();
		textureMap[texturePath] = texture;
		return texture;
	}
}

void TextureFactory::SaveTexture( std::string key, GLint val )
{

}
