#ifndef _MODEL_H__
#define _MODEL_H__

#include <memory>
#include <string>
#include <vector>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLUtils/VBO.hpp"
#include "TextureFactory.h"

struct MeshPart 
{
	MeshPart() 
	{
		transform = glm::mat4(1.0f);
		first = 0;
		count = 0;
		texCoords0 = false;
	}

	glm::mat4 transform;

	unsigned int first;	//<this meshParts initial index offset in the VBO
	unsigned int count;	//<the index length of this meshPart
	
	std::vector<std::string> diffuseTextures;
	std::vector<std::string> normalTextures;
	std::vector<std::string> specularTextures;

	bool texCoords0;//<true if the meshPart has one texture coord collection

	std::vector<MeshPart> children;
};

struct Vertex
{
	glm::vec3 vertex;
	glm::vec3 normal;
	glm::vec2 texCoord0;
};

class Model 
{
public:
	Model(std::string filename, bool invert=0);
	~Model();

	inline MeshPart getMesh() {return root;}

	inline std::shared_ptr<GLUtils::VBO> getInterleavedVBO(){return interleavedVBO;}
	inline std::shared_ptr<GLUtils::VBO> getIndexesVBO(){return indexes;}

	//Returns the stride, use for interleaved VBOs
	inline GLint getStride(){return stride;}

	//the offsets returns the offset used by interleaved VBOs as a GLVoid*
	inline GLvoid* getVerticeOffset() {return verticeOffset;}
	inline GLvoid* getNormalOffset() {return normalOffset;}
	inline GLvoid* getTexCoordOffset() {return texCoordOffset;}

	/*
	* Prints info about the model to the console
	*/
	void PrintModelInfoToConsole();

private:
	//This loadRecursive function places all data in one array,
	//used for interleaved VBOs, as well as indexes in a separate array
	static void loadRecursive(MeshPart& part, bool invert,
		std::vector<Vertex>& vertexData,
		std::vector<unsigned int>& indexes,
		glm::vec3& max_dim, glm::vec3& min_dim,
		const aiScene* scene, const aiNode* node, std::string relativeDirPath);

	/*
	* Helper function to see if x, y or z are bigger or smaller
	* than the x, y and z stored in max_dim and min_dim. If any value
	* matches the criteria (smaller than current value in min_dim, or
	* bigger than current value in max_dim), the value in the reference
	* vector is replaced
	*/
	static void checkDimensions(glm::vec3 newVertex, glm::vec3& max_dim, glm::vec3& min_dim);
	
	/*
	* Loads material names for the meshPart into the appropriate vector of names
	*/
	static void FindMaterials(const aiScene* scene, const aiMesh* mesh, 
							  MeshPart& meshpart, std::string relativeDirPath);

	const aiScene* scene;
	MeshPart root;

	//Our two VBO objects. the interleavedVBO stores vertexes, normals and potentially
	//texture data. The indexes VBO stores indexes for the model
	std::shared_ptr<GLUtils::VBO> interleavedVBO, indexes;
	
	glm::vec3 min_dim;//< max x, y and z for the model loaded.
	glm::vec3 max_dim;//< min x, y and z for the model loaded

	GLint stride;			//< Stride value for the interleavedVBO
	GLvoid* verticeOffset;	//< Offset value for the vertices (should be NULL)
	GLvoid* normalOffset;	//< Offset value for the normals (should be 3*sizeof(float))
	GLvoid* texCoordOffset;	//< Offset value for the textureCoords, if there are any(should be 6*sizeof(float))

	std::string modelFilePath;		  //< Name of the file loaded
	std::string projectRelativeDir;	  //< Directory relative to project
	float onLoadDiameter;			  //< The diameter of the file on load
	float downScale;				  //< Model scaling value used to fit the model
	glm::vec3 centeringTransformation;//< Transformation applied to scenter the model

	

};

#endif