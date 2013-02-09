#include "Model.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Model::Model(std::string filename, bool invert) 
{
	std::vector<float> data;
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
	//sure we'll be finding a new propper bigger and smaller value for them.
	max_dim = -glm::vec3(std::numeric_limits<float>::max());
	min_dim = glm::vec3(std::numeric_limits<float>::max());

	loadRecursive(root, invert, data, max_dim, min_dim, scene, scene->mRootNode);
	
	std::cout << "min x: " << min_dim.x << " min y: " << min_dim.y << " min z: " << min_dim.z << std::endl;
	std::cout << "max x: " << max_dim.x << " max y: " << max_dim.y << " max z: " << max_dim.z << std::endl;

	float diameter = glm::distance(min_dim, max_dim);
	std::cout << "diameter = " << diameter << std::endl;

	//Set the transformation matrix for the root node
	//the scale given by 1.0f/diameter scales the model to fit within the unit sphere
	root.transform = glm::scale(root.transform, glm::vec3(1.0f/diameter));
	root.transform = glm::translate(root.transform, glm::vec3(0.016800813, -0.11015295, 0.0014822669));

	dataSize = data.size();

	//Create the VBOs from the data.
	if (fmod(static_cast<float>(dataSize), 3.0f) < 0.000001f) 
	{
		interleavedVBO.reset(new GLUtils::VBO(data.data(), dataSize*sizeof(float)));
		stride = 6*sizeof(float);//3 for vertices, 3 for normals.
		verticeOffset = NULL;
		normalOffset = (GLvoid*)(3*sizeof(float));
		
		//texCoordOffset = (GLvoid*)(6*sizeof(float));
	}
	else
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");

}

Model::~Model() 
{

}

void Model::loadRecursive( MeshPart& part, bool invert, std::vector<float>& data, glm::vec3& max_dim, 
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

		part.first = data.size()/1;
		part.count = mesh->mNumFaces*3;

		//Allocate data
		data.reserve(data.size() + part.count*6);

		bool hasNormals = mesh->HasNormals();

		//Add the vertices from file
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
		{
			const struct aiFace* face = &mesh->mFaces[t];

			if(face->mNumIndices != 3)
				THROW_EXCEPTION("Only triangle meshes are supported");

			for(unsigned int i = 0; i < face->mNumIndices; i++) 
			{
				int index = face->mIndices[i];
				float x = mesh->mVertices[index].x;
				float y = mesh->mVertices[index].y;
				float z = mesh->mVertices[index].z;

				//checks if the x, y or z should be part of the bounding box
				checkDimensions(x, y, z, max_dim, min_dim);

				data.push_back(x);
				data.push_back(y);
				data.push_back(z);
				
				if(hasNormals)
				{
					data.push_back(mesh->mNormals[index].x);
					data.push_back(mesh->mNormals[index].y);
					data.push_back(mesh->mNormals[index].z);
				}
				
				if(mesh->HasTextureCoords(index))
				{
					data.push_back(mesh->mTextureCoords[index]->x);
					data.push_back(mesh->mTextureCoords[index]->y);
					data.push_back(mesh->mTextureCoords[index]->z);
				}
			}
		}
	}


	// load all children
	for (unsigned int n = 0; n < node->mNumChildren; ++n) 
	{
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, data, max_dim, min_dim, scene, node->mChildren[n]);
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
