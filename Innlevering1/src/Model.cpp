#include "Model.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Model::Model(std::string filename, bool invert) 
{
	this->fileName = filename;
	std::vector<float> data;
	std::vector<unsigned int> indexs;
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
		
	scene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);// | aiProcess_FlipWindingOrder);
	if (!scene) 
	{
		std::string log = "Unable to load mesh from ";
		log.append(filename);
		THROW_EXCEPTION(log);
	}

 	//initializing the max and min dim using the numeric float limits, so that we are
	//sure we'll be finding a new bigger and smaller value for them.
	max_dim = -glm::vec3(std::numeric_limits<float>::max());
	min_dim = glm::vec3(std::numeric_limits<float>::max());

	loadRecursive(root, invert, data, indexs, max_dim, min_dim, scene, scene->mRootNode);

	onLoadDiameter = glm::distance(min_dim, max_dim);
	centeringTransformation = min_dim + max_dim;
	centeringTransformation /= 2;
	downScale = 1/onLoadDiameter;

	//Set the transformation matrix for the root node
	root.transform = glm::scale(root.transform, glm::vec3(downScale));
	root.transform = glm::translate(root.transform, -centeringTransformation);

	unsigned int dataSize = data.size();

	//Create the VBOs from the data.
	if (fmod(static_cast<float>(dataSize), 3.0f) < 0.000001f) 
	{
		interleavedVBO.reset(new GLUtils::VBO(data.data(), dataSize*sizeof(float)));
		indexes.reset(new GLUtils::VBO(indexs.data(), indexs.size() * sizeof(unsigned int)));
		
		stride = 6*sizeof(float);//3 for vertices, 3 for normals.
		verticeOffset = NULL;
		normalOffset = (GLvoid*)(3*sizeof(float));
		texCoordOffset = (GLvoid*)(6*sizeof(float));
	}
	else
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");

	PrintModelInfoToConsole();
}

Model::~Model() 
{

}

void Model::loadRecursive( MeshPart& part, bool invert, std::vector<float>& data,
							std::vector<unsigned int>& indexes,glm::vec3& max_dim, 
							glm::vec3& min_dim, const aiScene* scene, const aiNode* node )
{
	//update transform matrix. notice that we also transpose it
	aiMatrix4x4 m = node->mTransformation;
	for (int j=0; j<4; ++j)
		for (int i=0; i<4; ++i)
			part.transform[j][i] = m[i][j];

	// draw all meshes assigned to this node
	for (unsigned int n=0; n < node->mNumMeshes; ++n) 
	{
		const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

		//apply_material(scene->mMaterials[mesh->mMaterialIndex]);

		part.first = indexes.size();
		part.count = mesh->mNumFaces*3;

		//Allocate data
		unsigned int indexOffset = data.size()/6;
		data.reserve(data.size() + part.count*3);
		
		//Storing the indices of this meshPart. Applying index offset
		//to each index, matching the number of vertices already stored
		//in the vector from other meshparts
		for(unsigned int t = 0; t < mesh->mNumFaces; ++t)
		{
			const struct aiFace* face = &mesh->mFaces[t];
			if(face->mNumIndices != 3)
				THROW_EXCEPTION("Only triangle meshes are supported");
			for(unsigned int i = 0; i < face->mNumIndices; i++) {
				indexes.push_back(face->mIndices[i]+indexOffset);
			}
		}

		//Storing the vertices, normals and potentially texture coordinates
		//in the data vector. 
		bool hasNormals = mesh->HasNormals();
		part.texCoords0 = mesh->HasTextureCoords(0);
		for(unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			//adding vertices
			float x = mesh->mVertices[v].x;
			float y = mesh->mVertices[v].y;
			float z = mesh->mVertices[v].z;
			data.push_back(x);
			data.push_back(y);
			data.push_back(z);
			checkDimensions(x, y, z, max_dim, min_dim);
			if(hasNormals)
			{
				data.push_back(mesh->mNormals[v].x);
				data.push_back(mesh->mNormals[v].y);
				data.push_back(mesh->mNormals[v].z);
			}
			if(mesh->HasTextureCoords(0))
			{
				/*data.push_back(mesh->mTextureCoords[0][v].x);
				data.push_back(mesh->mTextureCoords[0][v].y);
				data.push_back(mesh->mTextureCoords[0][v].z);*/
			}
		}	
	}


	// load all children
	for (unsigned int n = 0; n < node->mNumChildren; ++n) 
	{
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, data, indexes, max_dim, min_dim, scene, node->mChildren[n]);
	}
}


void Model::checkDimensions( float x, float y, float z, glm::vec3& max_dim, glm::vec3& min_dim)
{
	if(x < min_dim.x)
		min_dim.x = x;
	if(y < min_dim.y)
		min_dim.y = y;
	if(z < min_dim.z)
		min_dim.z = z;

	if(x > max_dim.x)
		max_dim.x = x;
	if(y > max_dim.y)
		max_dim.y = y;
	if(z > max_dim.z)
		max_dim.z = z;
}

void Model::PrintModelInfoToConsole()
{
	std::cout << "Model info: "<< std::endl;
	std::cout << "Filename:\t\t" << fileName << std::endl;
	std::cout << "On load min_dim:\t" << "X: " << min_dim.x << "\tY: " << min_dim.y << "\tZ: " << min_dim.z << std::endl;
	std::cout << "On load max_dim:\t" << "X: " << max_dim.x << "\tY: " << max_dim.y << "\tZ: " << max_dim.z << std::endl;

	std::cout << "On load diameter:\t" << onLoadDiameter << std::endl;
	std::cout << "Scale value:\t\t" << downScale << std::endl;
	std::cout << "Center transformation:\t" << "X: " << centeringTransformation.x << "\tY: " << centeringTransformation.y << "\tZ: " << centeringTransformation.z << std::endl;
}
