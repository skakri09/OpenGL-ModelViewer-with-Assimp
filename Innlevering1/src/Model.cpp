#include "Model.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/filesystem.hpp>

Model::Model(std::string filename, bool invert) 
{
	boost::filesystem::path pa = filename;
	projectRelativeDir = pa.branch_path().string();

	this->modelFilePath = filename;
	std::vector<Vertex> vertexData;
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

	loadRecursive(root, invert, vertexData, indexs, max_dim, min_dim, scene, scene->mRootNode, projectRelativeDir);

	onLoadDiameter = glm::distance(min_dim, max_dim);
	centeringTransformation = min_dim + max_dim;
	centeringTransformation /= 2;
	downScale = 1/onLoadDiameter;

	//Set the transformation matrix for the root node
	root.transform = glm::scale(root.transform, glm::vec3(downScale));
	root.transform = glm::translate(root.transform, -centeringTransformation);


	//Create the VBOs from the data.
	if (fmod(static_cast<float>(indexs.size()), 3.0f) < 0.000001f) 
	{
		interleavedVBO.reset(new GLUtils::VBO(vertexData.data(), vertexData.size()*sizeof(Vertex)));
		indexes.reset(new GLUtils::VBO(indexs.data(), indexs.size() * sizeof(unsigned int)));

		stride = sizeof(Vertex);
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

void Model::loadRecursive( MeshPart& part, bool invert, std::vector<Vertex>& vertexData, 
							std::vector<unsigned int>& indexes, glm::vec3& max_dim, glm::vec3& min_dim, 
							const aiScene* scene, const aiNode* node, std::string relativeDirPath )
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
		
		FindMaterials(scene, mesh, part, relativeDirPath);

		part.first = indexes.size();
		part.count = mesh->mNumFaces*3;

		//Allocate data
		unsigned int indexOffset = vertexData.size();
		vertexData.reserve(vertexData.size() + part.count*3);
		indexes.reserve(indexes.size() + mesh->mNumVertices*3);
		 
		bool hasNormals = mesh->HasNormals();
		part.texCoords0 = mesh->HasTextureCoords(0);
		
		for(unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			Vertex newVertex;

			newVertex.vertex = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);

			checkDimensions(newVertex.vertex, max_dim, min_dim);

			if(hasNormals)
				newVertex.normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);

			if(part.texCoords0)
				newVertex.texCoord0 = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
			else
				newVertex.texCoord0 = glm::vec2(0.0f, 0.0f);

			vertexData.push_back(newVertex);
		}	

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
	}

	// load all children
	for (unsigned int n = 0; n < node->mNumChildren; ++n) 
	{
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, vertexData, indexes, max_dim, min_dim, scene, node->mChildren[n], relativeDirPath);
	}
}

void Model::checkDimensions( glm::vec3 newVertex, glm::vec3& max_dim, glm::vec3& min_dim )
{
	if(newVertex.x < min_dim.x)
		min_dim.x = newVertex.x;
	if(newVertex.y < min_dim.y)
		min_dim.y = newVertex.y;
	if(newVertex.z < min_dim.z)
		min_dim.z = newVertex.z;

	if(newVertex.x > max_dim.x)
		max_dim.x = newVertex.x;
	if(newVertex.y > max_dim.y)
		max_dim.y = newVertex.y;
	if(newVertex.z > max_dim.z)
		max_dim.z = newVertex.z;
}

void Model::PrintModelInfoToConsole()
{
	std::cout << "Model info: "<< std::endl;
	std::cout << "Filename:\t\t" << modelFilePath << std::endl;
	std::cout << "On load min_dim:\t" << "X: " << min_dim.x << "\tY: " << min_dim.y << "\tZ: " << min_dim.z << std::endl;
	std::cout << "On load max_dim:\t" << "X: " << max_dim.x << "\tY: " << max_dim.y << "\tZ: " << max_dim.z << std::endl;

	std::cout << "On load diameter:\t" << onLoadDiameter << std::endl;
	std::cout << "Scale value:\t\t" << downScale << std::endl;
	std::cout << "Center transformation:\t" << "X: " << centeringTransformation.x << "\tY: " << centeringTransformation.y << "\tZ: " << centeringTransformation.z << std::endl;
}

void Model::FindMaterials( const aiScene* scene, const aiMesh* mesh,  
						   MeshPart& meshpart, std::string relativeDirPath)
{
	aiMaterial* p =  scene->mMaterials[mesh->mMaterialIndex];
	for(unsigned int i = 0; i < p->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString s;
		p->GetTexture(aiTextureType_DIFFUSE, i, &s);
		meshpart.diffuseTextures.push_back(relativeDirPath + "/" + s.C_Str());
		std::cout << "Diffuse Texture: " << relativeDirPath + "/" +s.C_Str() << std::endl;
		ModelTexture::Inst()->LoadTexture(meshpart.diffuseTextures.back());
	}
	for(unsigned int i = 0; i < p->GetTextureCount(aiTextureType_NORMALS); i++)
	{
		aiString s;
		p->GetTexture(aiTextureType_NORMALS, i, &s);
		meshpart.normalTextures.push_back(relativeDirPath + "/" + s.C_Str());
		std::cout << "Normalmap Texture: " << relativeDirPath + "/" + s.C_Str() << std::endl;
		ModelTexture::Inst()->LoadTexture(meshpart.normalTextures.back());
	}
	for(unsigned int i = 0; i < p->GetTextureCount(aiTextureType_SPECULAR); i++)
	{
		aiString s;
		p->GetTexture(aiTextureType_SPECULAR, i, &s);
		meshpart.specularTextures.push_back(relativeDirPath + "/" + s.C_Str());
		std::cout << "Specular Texture: " << relativeDirPath + "/" + s.C_Str() << std::endl;
		ModelTexture::Inst()->LoadTexture(meshpart.specularTextures.back());
	}
}
