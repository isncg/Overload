/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>

#include "OvRendering/Resources/Parsers/AssimpParser.h"
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <cassert>

struct OvRendering::Geometry::AssimpVertex
{
public:
	union { aiVector3D position;  OvMaths::FVector3 _position;	};
	union { aiVector3D normal;    OvMaths::FVector3 _normal;	};
	union { aiVector3D tangent;   OvMaths::FVector3 _tangent;	};
	union { aiVector3D bitangent; OvMaths::FVector3 _bitangent;	};
	int boneIDs[4];
	float boneWeights[4];

	union
	{
		aiVector3D texCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
		struct
		{
			OvMaths::FVector2 _texCoords;
			float _;
		} _texCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
	};


	AssimpVertex() 
	{
		boneIDs[0] = 0;
		boneIDs[1] = 0;
		boneIDs[2] = 0;
		boneIDs[3] = 0;
	}
	AssimpVertex(const AssimpVertex& other)
	{
		position = other.position;
		normal = other.normal;
		tangent = other.tangent;
		for (int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
		{
			texCoords[i] = other.texCoords[i];
		}
		bitangent = other.bitangent;

		memcpy_s(boneIDs, sizeof(boneIDs), other.boneIDs, sizeof(other.boneIDs));
		memcpy_s(boneWeights, sizeof(boneWeights), other.boneWeights, sizeof(other.boneWeights));
	}
};

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPosition(AssimpVertex& v)      { return v._position;                    }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormal(AssimpVertex& v)        { return v._normal;                      }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangent(AssimpVertex& v)       { return v._tangent;                     }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUV(AssimpVertex& v)            { return v._texCoords[0]._texCoords;     }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUV(AssimpVertex& v, int index) { return v._texCoords[index]._texCoords; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangent(AssimpVertex& v)     { return v._bitangent;                   }
const int* OvRendering::Geometry::VertexHelper::GetBoneIds(AssimpVertex& v) { return v.boneIDs; }
const float* OvRendering::Geometry::VertexHelper::GetBoneWeights(AssimpVertex& v) { return v.boneWeights; }

bool OvRendering::Resources::Parsers::AssimpParser::LoadModel(const std::string & p_fileName, std::vector<Mesh*>& p_meshes, std::vector<std::string>& p_materials, EModelParserFlags p_parserFlags, OvRendering::Resources::ModelHierarchy& p_modelHierarchy)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(p_fileName, static_cast<unsigned int>(p_parserFlags));

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	ProcessMaterials(scene, p_materials);

	aiMatrix4x4 identity;

	p_modelHierarchy.Init(scene);
	std::cout << "======== " << p_fileName<< " ========" << std::endl;
	p_modelHierarchy.DumpNodeTree();
	std::cout << "=========== end ===========" << std::endl;
	ProcessNode(&identity, scene->mRootNode, scene, p_meshes);
	return true;
}

void OvRendering::Resources::Parsers::AssimpParser::ProcessMaterials(const aiScene * p_scene, std::vector<std::string>& p_materials)
{
	for (uint32_t i = 0; i < p_scene->mNumMaterials; ++i)
	{
		aiMaterial* material = p_scene->mMaterials[i];
		if (material)
		{
			aiString name;
			aiGetMaterialString(material, AI_MATKEY_NAME, &name);
			p_materials.push_back(name.C_Str());
		}
	}
}

void OvRendering::Resources::Parsers::AssimpParser::ProcessNode(void* p_transform, aiNode * p_node, const aiScene * p_scene, std::vector<Mesh*>& p_meshes)
{
	aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform) * p_node->mTransformation;

	// Process all the node's meshes (if any)
	for (uint32_t i = 0; i < p_node->mNumMeshes; ++i)
	{
		std::vector<Geometry::AssimpVertex> vertices;
		std::vector<uint32_t> indices;
		aiMesh* mesh = p_scene->mMeshes[p_node->mMeshes[i]];
		auto ovmesh = new OvRendering::Resources::Mesh();
		ovmesh->m_rigInfo.nodeName = p_node->mName.C_Str();
		ovmesh->m_rigInfo.meshName = mesh->mName.C_Str();
		ProcessMesh(&nodeTransformation, mesh, p_scene, vertices, indices, ovmesh->m_rigInfo);
		std::cout << "Init mesh " << ovmesh->m_rigInfo.meshName << " on node " << ovmesh->m_rigInfo.nodeName << std::endl;
		ovmesh->Init(vertices, indices, mesh->mMaterialIndex);
		p_meshes.push_back(ovmesh); // The model will handle mesh destruction
	}

	// Then do the same for each of its children
	for (uint32_t i = 0; i < p_node->mNumChildren; ++i)
	{
		ProcessNode(&nodeTransformation, p_node->mChildren[i], p_scene, p_meshes);
	}
}

void OvRendering::Resources::Parsers::AssimpParser::ProcessMesh(void* p_transform, aiMesh* p_mesh, const aiScene* p_scene, std::vector<OvRendering::Geometry::AssimpVertex>& p_outVertices, std::vector<uint32_t>& p_outIndices, OvRendering::Resources::MeshRigInfo& rigInfo)
{
	aiMatrix4x4 meshTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform);

	for (uint32_t i = 0; i < p_mesh->mNumVertices; ++i)
	{
		OvRendering::Geometry::AssimpVertex v;
		v.position = meshTransformation * p_mesh->mVertices[i];
		v.normal = meshTransformation * (p_mesh->mNormals ? p_mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
		for (int texcoordindex = 0; texcoordindex < AI_MAX_NUMBER_OF_TEXTURECOORDS; texcoordindex++)
			v.texCoords[texcoordindex] = p_mesh->mTextureCoords[texcoordindex] ? p_mesh->mTextureCoords[texcoordindex][i] : aiVector3D(0.0f, 0.0f, 0.0f);
		v.tangent = p_mesh->mTangents ? meshTransformation * p_mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);
		v.bitangent = p_mesh->mBitangents ? meshTransformation * p_mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);

		p_outVertices.push_back(v);
	}

	for (uint32_t faceID = 0; faceID < p_mesh->mNumFaces; ++faceID)
	{
		auto& face = p_mesh->mFaces[faceID];

		for (size_t indexID = 0; indexID < 3; ++indexID)
			p_outIndices.push_back(face.mIndices[indexID]);
	}

	rigInfo.boneInfos.clear();
	auto& defaultBoneInfo = rigInfo.boneInfos.emplace_back(rigInfo.nodeName);
	for (int i = 0; i < p_outVertices.size(); i++)
	{
		p_outVertices[i].boneIDs[0] = 1; p_outVertices[i].boneWeights[0] = 1.0f;
		p_outVertices[i].boneIDs[1] = 1; p_outVertices[i].boneWeights[1] = 0.0f;
		p_outVertices[i].boneIDs[2] = 1; p_outVertices[i].boneWeights[2] = 0.0f;
		p_outVertices[i].boneIDs[3] = 1; p_outVertices[i].boneWeights[3] = 0.0f;
	}
	if (p_mesh->HasBones())
	{
		for (int i = 0; i < p_mesh->mNumBones; ++i)
		{
			auto p_bone = p_mesh->mBones[i];
			//auto& boneInfo = rigInfo.boneInfos.emplace_back(*p_bone);
			int boneIndex = rigInfo.GetBoneIndex(p_bone->mName.C_Str());
			if (boneIndex < 0)
			{
				rigInfo.boneInfos.emplace_back(*p_bone);
				boneIndex = rigInfo.GetBoneIndex(p_bone->mName.C_Str());
			}
			assert(boneIndex >= 0);
			int bondId = boneIndex + 1;
			for (int w = 0; w < p_bone->mNumWeights; w++)
			{
				auto & weight = p_bone->mWeights[w];
				auto& vert = p_outVertices[weight.mVertexId];

				int k = 3;
				for (; k > 1; k--)
					if(vert.boneWeights[k] <= 0.0f)
						break;
				vert.boneIDs[k] = bondId;
				vert.boneWeights[k] = weight.mWeight;
				if (k > 0)
				{
					float sum = 0.0f;
					sum += vert.boneWeights[1];
					sum += vert.boneWeights[2];
					sum += vert.boneWeights[3];
					vert.boneWeights[0] = 1.0 - sum;
				}
			}
		}
	}
}
