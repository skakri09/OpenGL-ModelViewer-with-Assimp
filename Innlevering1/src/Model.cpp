#include "Model.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Model::Model(std::string filename, bool invert) 
{
	std::vector<float> vertex_data, normal_data;
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
		
	scene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);// | aiProcess_FlipWindingOrder);
	if (!scene) 
	{
		std::string log = "Unable to load mesh from ";
		log.append(filename);
		THROW_EXCEPTION(log);
	}

	/**
	  * FIXME: Alter loadRecursive, so that it also loads normal data
	  */
	//Load the model recursively into data
	max_dim = -glm::vec3(std::numeric_limits<float>::max());
	min_dim = glm::vec3(std::numeric_limits<float>::max());

	loadRecursive(root, invert, vertex_data, normal_data, max_dim, min_dim, scene, scene->mRootNode);
	
	std::cout << "min x: " << min_dim.x << " min y: " << min_dim.y << " min z: " << min_dim.z << std::endl;
	std::cout << "max x: " << max_dim.x << " max y: " << max_dim.y << " max z: " << max_dim.z << std::endl;

	float diameter = glm::distance(min_dim, max_dim);
	std::cout << "diameter = " << diameter << std::endl;

	//Set the transformation matrix for the root node
	//These are hard-coded constants for the stanford bunny model.
	root.transform = glm::scale(root.transform, glm::vec3(1.0f/diameter));
	root.transform = glm::translate(root.transform, glm::vec3(0.016800813, -0.11015295, 0.0014822669));

	n_vertices = vertex_data.size();

	//Create the VBOs from the data.
	if (fmod(static_cast<float>(n_vertices), 3.0f) < 0.000001f) 
	{
		vertices.reset(new GLUtils::VBO(vertex_data.data(), n_vertices*sizeof(float)));
		normals.reset(new GLUtils::VBO(normal_data.data(), n_vertices*sizeof(float)));
	}
	else
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");

}

Model::~Model() 
{

}

void Model::loadRecursive(MeshPart& part, bool invert,
			std::vector<float>& vertex_data,
			std::vector<float>& normal_data,
			glm::vec3& max_dim, glm::vec3& min_dim,
			const aiScene* scene, const aiNode* node) 
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

		part.first = vertex_data.size()/3;
		part.count = mesh->mNumFaces*3;

		//Allocate data
		vertex_data.reserve(vertex_data.size() + part.count*3);

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

				vertex_data.push_back(x);
				vertex_data.push_back(y);
				vertex_data.push_back(z);

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

				if(hasNormals)
				{
					normal_data.push_back(mesh->mNormals[index].x);
					normal_data.push_back(mesh->mNormals[index].y);
					normal_data.push_back(mesh->mNormals[index].z);
				}
			}
		}
	}


	// load all children
	for (unsigned int n = 0; n < node->mNumChildren; ++n) 
	{
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, vertex_data, normal_data, max_dim, min_dim, scene, node->mChildren[n]);
	}
}