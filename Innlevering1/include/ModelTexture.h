/********************************************************************
	created:	15:2:2013   14:12
	filename: 	ModelTexture.h
	author:		Kristian Skarseth
	
	purpose:	
*********************************************************************/
#ifndef ModelTexture_h__
#define ModelTexture_h__
#include <string>
#include <map>

#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <vector>
#include <GL/glew.h>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include "GLUtils/GLUtils.hpp"
struct Image 
{
	std::vector<char> data;
	unsigned int components;
	unsigned long widht;
	unsigned long height;
};

class ModelTexture
{
public:
	static ModelTexture* Inst();

	GLint GetTexture(std::string textureName);

	GLint LoadTexture(std::string texturePath);

protected:

private:
	ModelTexture();
	~ModelTexture();
	
	

	std::map<std::string, GLint> textureMap;
};

#endif // ModelTexture_h__
