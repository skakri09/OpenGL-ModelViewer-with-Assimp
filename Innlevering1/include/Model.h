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
	
	inline std::shared_ptr<GLUtils::VBO> getVertices() {return vertices;}
	inline std::shared_ptr<GLUtils::VBO> getNormals() {return normals;}
	inline std::shared_ptr<GLUtils::VBO> getTexCoords(){return texCoords;}

	inline GLint getStride(){return stride;}
	inline GLvoid* getVerticeOffset() {return verticeOffset;}
	inline GLvoid* getNormalOffset() {return normalOffset;}
	inline GLvoid* getTexCoordOffset() {return texCoordOffset;}

private:
	static void loadRecursive(MeshPart& part, bool invert,
			std::vector<float>& vertex_data, 
			std::vector<float>& normal_data,
			std::vector<float>& texture_data,
			glm::vec3& max_dim, glm::vec3& min_dim,
			const aiScene* scene, const aiNode* node);
	
	static void loadRecursive(MeshPart& part, bool invert,
		std::vector<float>& data,
		glm::vec3& max_dim, glm::vec3& min_dim,
		const aiScene* scene, const aiNode* node);


	static void checkDimensions(float x, float y, float z, 
								glm::vec3& max_dim, glm::vec3& min_dim);

	const aiScene* scene;
	MeshPart root;

	std::shared_ptr<GLUtils::VBO> interleavedVBO;
	std::shared_ptr<GLUtils::VBO> normals;
	std::shared_ptr<GLUtils::VBO> vertices;
	std::shared_ptr<GLUtils::VBO> texCoords;
	
	glm::vec3 min_dim;
	glm::vec3 max_dim;

	unsigned int n_vertices;
	unsigned int dataSize;
	
	GLint stride;
	GLvoid* verticeOffset;
	GLvoid* normalOffset;
	GLvoid* texCoordOffset;
};

#endif