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
#include "OvRendering/Resources/Animation.h"
#include <string>
#include <map>
#include <algorithm>

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
		boneIDs[0] = -1;
		boneIDs[1] = -1;
		boneIDs[2] = -1;
		boneIDs[3] = -1;
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

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPositionImpl(AssimpVertex& v)      { return v._position;                    }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormalImpl(AssimpVertex& v)        { return v._normal;                      }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangentImpl(AssimpVertex& v)       { return v._tangent;                     }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUVImpl(AssimpVertex& v)            { return v._texCoords[0]._texCoords;     }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUVImpl(AssimpVertex& v, int index) { return v._texCoords[index]._texCoords; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangentImpl(AssimpVertex& v)     { return v._bitangent;                   }


bool OvRendering::Resources::Parsers::AssimpParser::LoadModel(const std::string & p_fileName, std::vector<Mesh*>& p_meshes, std::vector<std::string>& p_materials, EModelParserFlags p_parserFlags, OvRendering::Resources::Animation* p_outAnimation)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(p_fileName, static_cast<unsigned int>(p_parserFlags));

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	ProcessMaterials(scene, p_materials);

	aiMatrix4x4 identity;

	ProcessNode(&identity, scene->mRootNode, scene, p_meshes, p_outAnimation);
	ProcessAnimations(scene, p_outAnimation);
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

void OvRendering::Resources::Parsers::AssimpParser::ProcessNode(void* p_transform, aiNode * p_node, const aiScene * p_scene, std::vector<Mesh*>& p_meshes, OvRendering::Resources::Animation* p_outAnimation)
{
	aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform) * p_node->mTransformation;

	// Process all the node's meshes (if any)
	for (uint32_t i = 0; i < p_node->mNumMeshes; ++i)
	{
		std::vector<Geometry::AssimpVertex> vertices;
		std::vector<uint32_t> indices;
		aiMesh* mesh = p_scene->mMeshes[p_node->mMeshes[i]];
		ProcessMesh(&nodeTransformation, mesh, p_scene, vertices, indices, p_outAnimation);
		auto ovmesh = new OvRendering::Resources::Mesh();
		ovmesh->Init(vertices, indices, mesh->mMaterialIndex);
		p_meshes.push_back(ovmesh); // The model will handle mesh destruction
	}

	// Then do the same for each of its children
	for (uint32_t i = 0; i < p_node->mNumChildren; ++i)
	{
		ProcessNode(&nodeTransformation, p_node->mChildren[i], p_scene, p_meshes, p_outAnimation);
	}
}

void OvRendering::Resources::Parsers::AssimpParser::ProcessMesh(void* p_transform, aiMesh* p_mesh, const aiScene* p_scene, std::vector<OvRendering::Geometry::AssimpVertex>& p_outVertices, std::vector<uint32_t>& p_outIndices, OvRendering::Resources::Animation* p_outAnimation)
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

	if (p_mesh->HasBones() && p_outAnimation)
	{
		for (int i = 0; i < p_mesh->mNumBones; ++i)
		{
			auto p_bone = p_mesh->mBones[i];
			std::string boneName = p_bone->mName.C_Str();
			auto findResultIter = std::find_if(p_outAnimation->meshBoneAnimations.begin(), p_outAnimation->meshBoneAnimations.end(), [&boneName](const MeshBoneAnimation& x) {return x.boneName == boneName; });
			auto anim = findResultIter == p_outAnimation->meshBoneAnimations.end() ?
				&p_outAnimation->meshBoneAnimations.emplace_back(MeshBoneAnimation{ boneName, (int)p_outAnimation->meshBoneAnimations.size(), 0 }) : findResultIter._Ptr;
			//MeshBoneAnimation* anim = p_outAnimation->GetOrAddBoneAnimation(p_bone->mName.C_Str());
			for (int w = 0; w < p_bone->mNumWeights; w++)
			{
				auto & weight = p_bone->mWeights[w];
				auto& vert = p_outVertices[weight.mVertexId];
				for (int k = 0; k < 4; k++)
				{
					if (vert.boneIDs[k] < 0)
					{
						vert.boneIDs[k] = anim->boneId;
						vert.boneWeights[k] = weight.mWeight;
						break;
					}
				}
			}
		}
	}
}

void OvRendering::Resources::Parsers::AssimpParser::ProcessAnimations(const aiScene* p_scene, OvRendering::Resources::Animation* p_outAnimation)
{
	for (int i = 0; i < p_scene->mNumAnimations; ++i)
	{
		auto& anim = p_scene->mAnimations[i];
		for (int channelIndex = 0; channelIndex < anim->mNumChannels; channelIndex++)
		{
			aiNodeAnim* nodeAnim = anim->mChannels[channelIndex];
			std::string nodeName(nodeAnim->mNodeName.C_Str());
			auto findResult = std::find_if(p_outAnimation->meshBoneAnimations.begin(), p_outAnimation->meshBoneAnimations.end(), [&nodeName](MeshBoneAnimation a) {return a.boneName == nodeName; });
			if (findResult != p_outAnimation->meshBoneAnimations.end())
			{
				for (int keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; keyIndex++)
				{
					findResult->positions.emplace_back(
						MeshBoneAnimationPositionKey{
							nodeAnim->mPositionKeys[keyIndex].mTime,
							OvMaths::FVector3(
								nodeAnim->mPositionKeys[keyIndex].mValue.x,
								nodeAnim->mPositionKeys[keyIndex].mValue.y,
								nodeAnim->mPositionKeys[keyIndex].mValue.z
							)
						});
				}

				for (int keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; keyIndex++)
				{
					findResult->rotations.emplace_back(
						MeshBoneAnimationRotationKey{
							nodeAnim->mRotationKeys[keyIndex].mTime,
							OvMaths::FQuaternion(
								nodeAnim->mRotationKeys[keyIndex].mValue.x,
								nodeAnim->mRotationKeys[keyIndex].mValue.y,
								nodeAnim->mRotationKeys[keyIndex].mValue.z,
								nodeAnim->mRotationKeys[keyIndex].mValue.w
							)
						});
				}

				for (int keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; keyIndex++)
				{
					findResult->scales.emplace_back(
						MeshBoneAnimationScaleKey{
							nodeAnim->mScalingKeys[keyIndex].mTime,
							OvMaths::FVector3(
								nodeAnim->mScalingKeys[keyIndex].mValue.x,
								nodeAnim->mScalingKeys[keyIndex].mValue.y,
								nodeAnim->mScalingKeys[keyIndex].mValue.z
							)
						});
				}
			}
		}
	}
}
