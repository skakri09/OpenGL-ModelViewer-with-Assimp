/********************************************************************
	created:	15:2:2013   14:12
	filename: 	ModelTexture.h
	author:		Kristian Skarseth
	
	purpose:	Load and handle textures. Loading a texture uploads it directly
				to the gpu, and stores the textureID in a map, where it can be
				accessed with the original texturePath as key.

				Using a singleton for this class as we only ever want one copy of it, 
				and it can be convenient to publically access it from several classes.

	Note:		Initializing DevIL in the constructor of this class. Since it's a singleton
				that causes no issues with trying to initialize multiple times, and this is the
				only class that should use devil, so letting it initialize it seems OK.
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


class TextureFactory
{
public:
	//Singleton access function
	static TextureFactory* Inst();

	/*
	* Returns the textureID for the textureName. Can be directly
	* bound to openGL for use.
	* If the texture has not already been loaded, the class will
	* attempt to load it.
	*/
	GLint GetTexture(std::string textureName);

	/*
	* Attempts to load and store the texture on the texturepath.
	* If it's successfull, the openGL ID is returned.
	*/
	GLint LoadTexture(std::string texturePath);

protected:

private:
	TextureFactory();
	~TextureFactory();
	
	// Our map of textureIDs. Key should be the path of the texture,
	// which was used to load it
	std::map<std::string, GLint> textureMap;
};

#endif // ModelTexture_h__
