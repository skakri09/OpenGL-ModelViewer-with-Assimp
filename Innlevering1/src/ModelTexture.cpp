#include "ModelTexture.h"
#include "GameException.h"

ModelTexture::ModelTexture()
{
	ilInit();
	iluInit();
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilEnable(IL_ORIGIN_SET);
}

ModelTexture::~ModelTexture()
{
}

ModelTexture* ModelTexture::Inst()
{
	static ModelTexture* instance = new ModelTexture();
	return instance;
}

GLint ModelTexture::GetTexture( std::string textureName )
{
	
	if(textureMap.find(textureName) != textureMap.end())
		return textureMap.find(textureName)->second;
	else
		return LoadTexture(textureName);
}

GLint ModelTexture::LoadTexture( std::string texturePath )
{
	if(textureMap.find(texturePath) != textureMap.end())
		return textureMap.find(texturePath)->second;
	else
	{
		std::vector<unsigned char> data;
		ILuint ImageName;
		unsigned int width, height;
		GLuint texture;

		ilGenImages(1, &ImageName); // Grab a new image name.
		ilBindImage(ImageName);

		if (!ilLoadImage(texturePath.c_str())) {
			ILenum e;
			std::stringstream error;
			while ((e = ilGetError()) != IL_NO_ERROR) {
				error << e << ": " << iluErrorString(e) << std::endl;
			}
			ilDeleteImages(1, &ImageName); // Delete the image name. 
		}

		width = ilGetInteger(IL_IMAGE_WIDTH); // getting image width
		height = ilGetInteger(IL_IMAGE_HEIGHT); // and height
		data.resize(width*height*3);

		//ilInit compon = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

		ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data.data());
		ilDeleteImages(1, &ImageName); // Delete the image name. 

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		CHECK_GL_ERROR();
		textureMap[texturePath] = texture;
		return texture;
	}
}
