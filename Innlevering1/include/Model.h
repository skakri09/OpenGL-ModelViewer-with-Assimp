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

struct MeshPart 
{
	MeshPart() 
	{
		transform = glm::mat4(1.0f);
		first = 0;
		count = 0;
	}

	glm::mat4 transform;
	unsigned int first;
	unsigned int count;
	std::vector<MeshPart> children;
};

class Model 
{
public:
	Model(std::string filename, bool invert=0);
	~Model();

	inline MeshPart getMesh() {return root;}

	inline std::shared_ptr<GLUtils::VBO> getInterleavedVBO(){return interleavedVBO;}

	//Returns the stride, use for interleaved VBOs
	inline GLint getStride(){return stride;}

	//the offsets returns the offset used by interleaved VBOs as a GLVoid*
	inline GLvoid* getVerticeOffset() {return verticeOffset;}
	inline GLvoid* getNormalOffset() {return normalOffset;}
	inline GLvoid* getTexCoordOffset() {return texCoordOffset;}

private:
	//This loadRecursive function places all data in one array,
	//used for interleaved VBOs
	static void loadRecursive(MeshPart& part, bool invert,
		std::vector<float>& data,
		glm::vec3& max_dim, glm::vec3& min_dim,
		const aiScene* scene, const aiNode* node);


	static void checkDimensions(float x, float y, float z, 
								glm::vec3& max_dim, glm::vec3& min_dim);

	const aiScene* scene;
	MeshPart root;

	std::shared_ptr<GLUtils::VBO> interleavedVBO;

	glm::vec3 min_dim;
	glm::vec3 max_dim;

	unsigned int dataSize;
	
	GLint stride;
	GLvoid* verticeOffset;
	GLvoid* normalOffset;
	GLvoid* texCoordOffset;
};

#endif