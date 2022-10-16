#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>

#include "OvRendering/Resources/ModelHierarchy.h"
#include <iostream>
#include <cassert>
int OvRendering::Resources::ModelNode::GetIndex(bool cached)
{
	if (cached)
		return index;
	for (int i = 0; i < hierarchy->nodes.size(); i++)
	{
		if (&hierarchy->nodes[i] == this)
			return i;
	}
	return -1;
}

OvRendering::Resources::ModelNode::ModelNode(const aiScene* scene, aiNode* node)
{
	this->name = node->mName.C_Str();
	this->transform = *(OvMaths::FMatrix4*)&node->mTransformation;

	for (int i = 0; i < node->mNumMeshes; i++)
	{
		this->meshNames.push_back(scene->mMeshes[node->mMeshes[i]]->mName.C_Str());
	}
}

OvRendering::Resources::ModelNodeAnimation::ModelNodeAnimation(ModelHierarchy* hierarchy, aiNodeAnim* anim)
{
	this->hierarchy = hierarchy;
	this->name = anim->mNodeName.C_Str();
	for (int i = 0; i < anim->mNumPositionKeys; i++)
		this->positions.emplace_back(anim->mPositionKeys[i]);

	for (int i = 0; i < anim->mNumRotationKeys; i++)
		this->rotations.emplace_back(anim->mRotationKeys[i]);

	for (int i = 0; i < anim->mNumScalingKeys; i++)
		this->scales.emplace_back(anim->mScalingKeys[i]);

}

struct FindResult
{
	int index1, index2;
	double weight1, weight2;
};
FindResult find(double time, const std::vector<OvRendering::Resources::ModelNodeAnimationKey>& keys)
{
	FindResult result;
	int len = keys.size();
	if (len == 0)
	{
		result.index1 = -1;
		result.index2 = -1;
		return result;
	}

	if (time <= keys[0].time)
	{
		result.index1 = 0;
		result.index2 = 0;
		result.weight1 = 1;
		result.weight2 = 0;
		return result;
	}


	for (int i = 0; i < len - 1; i++)
	{
		if (keys[i].time > time)
		{
			result.index1 = i;
			result.index2 = i + 1;
			double space = keys[i + 1].time - keys[i].time;
			result.weight1 = (time - keys[i].time) / space;
			result.weight2 = (keys[i + 1].time - time) / space;
			return result;
		}

	}

	{
		result.index1 = len - 1;
		result.index2 = len - 1;
		result.weight1 = 0;
		result.weight2 = 1;
	}
	return result;
}

OvMaths::FVector3 getvec(FindResult&& fr, const std::vector<OvRendering::Resources::ModelNodeAnimationKey>& keys)
{
	if (fr.index1 == -1 || fr.index2 == -1)
		return OvMaths::FVector3::Zero;
	return keys[fr.index1].value_vec * fr.weight1 + keys[fr.index2].value_vec * fr.weight2;
}

OvMaths::FQuaternion getqua(FindResult&& fr, const std::vector<OvRendering::Resources::ModelNodeAnimationKey>& keys)
{
	if (fr.index1 == -1 || fr.index2 == -1)
		return OvMaths::FQuaternion::Identity;
	return keys[fr.index1].value_qua * fr.weight1 + keys[fr.index2].value_qua * fr.weight2;
}

OvMaths::FMatrix4 OvRendering::Resources::ModelNodeAnimation::GetAnimationTransform(double time) const
{
	auto position = getvec(find(time, positions), positions);
	auto rotation = getqua(find(time, rotations), rotations);
	auto scale = getvec(find(time, scales), scales);

	return OvMaths::FMatrix4::Translation(position) * OvMaths::FQuaternion::ToMatrix4(OvMaths::FQuaternion::Normalize(rotation)) * OvMaths::FMatrix4::Scaling(scale);

}

OvRendering::Resources::ModelNode* OvRendering::Resources::ModelHierarchy::GetNode(std::string name)
{
	for (auto& it : this->nodes)
		if (it.name == name)
			return &it;
	return nullptr;
}

int OvRendering::Resources::ModelHierarchy::GetNodeIndex(std::string name) const
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].name == name)
			return i;
	}
	return -1;
}

void OvRendering::Resources::ModelHierarchy::Init(const aiScene* scene)
{
	CreateChildNode(scene, scene->mRootNode, -1);

	for (int i = 0; i < scene->mNumAnimations; i++)
		this->animations.emplace_back(this, scene->mAnimations[i]);
}

void OvRendering::Resources::ModelHierarchy::CreateChildNode(const aiScene* scene, aiNode* node, int parentIndex)
{
	assert(parentIndex < (int)nodes.size());
	assert(GetNodeIndex(node->mName.C_Str()) == -1);
	//if (parentIndex >= nodes.size())
	//	return false;
	//if (GetNode(node->mName.C_Str()))
	//	return false;

	auto& mnode = nodes.emplace_back(scene, node);
	mnode.hierarchy = this;
	int index = mnode.GetIndex();
	mnode.index = index;
	mnode.parent = parentIndex;
	if (parentIndex >= 0)
	{
		nodes[parentIndex].children.insert(index);
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		CreateChildNode(scene, node->mChildren[i], index);
	}
}

void printIndent(int depth)
{
	for (int i = 0; i < depth * 2; i++)
		putchar(' ');
}


void OvRendering::Resources::ModelHierarchy::DumpNodeTree(int index /*= 0*/, int depth /*= 0*/)
{
	if (index >= nodes.size())
		return;
	auto& node = nodes[index];
	printIndent(depth);
	std::cout << node.name << std::endl;
	for (int i = 0; i < 4; i++)
	{
		printIndent(depth + 4);
		for (int j = 0; j < 4; j++)
			std::cout << "\t" << node.transform.data[i * 4 + j] << "  ";
		std::cout << std::endl;
	}
	printIndent(depth+4);
	std::cout << "Meshes(" << node.meshNames.size() << "):  ";
	for (auto& n : node.meshNames)
		std::cout << n <<"  ";
	std::cout << std::endl;
	for (auto childIndex : node.children)
		DumpNodeTree(childIndex, depth + 1);
}

void OvRendering::Resources::ModelHierarchy::DumpMeshList()
{
	//for(auto& m:)
}

OvRendering::Resources::ModelNodeAnimationKey::ModelNodeAnimationKey(aiVectorKey& key)
{
	this->time = key.mTime;
	this->value_vec = *(OvMaths::FVector3*)&key.mValue;
}

OvRendering::Resources::ModelNodeAnimationKey::ModelNodeAnimationKey(aiQuatKey& key)
{
	this->time = key.mTime;
	this->value_qua = *(OvMaths::FQuaternion*)&key.mValue;
}

OvRendering::Resources::ModelNodeAnimationKey::ModelNodeAnimationKey(ModelNodeAnimationKey& other)
{
	this->time = other.time;
	this->value_qua = other.value_qua;
}

OvRendering::Resources::ModelAnimation::ModelAnimation(ModelHierarchy* hierarchy, aiAnimation* anim)
{
	this->hierarchy = hierarchy;
	this->name = anim->mName.C_Str();
	for (int i = 0; i < anim->mNumChannels; i++)
		this->nodeAnimations.emplace_back(hierarchy, anim->mChannels[i]);
}

//void OvRendering::Resources::ModelAnimation::CalculateTransforms(double time, MeshRigInfo& rigInfo, OvMaths::FMatrix4* transforms)
//{
//	for (int i = 0; i < rigInfo.orderedBoneNames.size(); i++)
//	{
//		auto anim = GetNodeAnimation(rigInfo.orderedBoneNames[i]);
//		if (nullptr == anim)
//			transforms[i] = OvMaths::FMatrix4::Identity;
//		else
//			transforms[i] = anim->GetAnimationTransform(time);
//	}
//}

const OvRendering::Resources::ModelNodeAnimation* OvRendering::Resources::ModelAnimation::GetNodeAnimation(std::string name) const
{
	for (auto& n : nodeAnimations)
		if (n.name == name)
			return &n;
	return nullptr;
}

//int OvRendering::Resources::MeshRigInfo::GetOrAddBoneName(std::string name)
//{
//	for (int i = 0; i < orderedBoneNames.size(); i++)
//		if (orderedBoneNames[i] == name)
//			return i;
//	orderedBoneNames.push_back(name);
//	return orderedBoneNames.size() - 1;
//}

OvRendering::Resources::MeshRigBoneInfo::MeshRigBoneInfo(aiBone& bone)
{
	this->name = bone.mName.C_Str();
	this->offset = *(OvMaths::FMatrix4*)&bone.mOffsetMatrix;
}

OvRendering::Resources::MeshRigBoneInfo::MeshRigBoneInfo(std::string name)
{
	this->name = name;
	this->offset = OvMaths::FMatrix4::Identity;
}

void OvRendering::Resources::ModelNodeTransformCalculator::CalculateNodeTransform(int nodeIndex, int parentIndex)
{
	auto& node = hierarchy.nodes[nodeIndex];
	auto& parentWorldTransfom = parentIndex < 0 ? OvMaths::FMatrix4::Identity : hierarchyWorldTransforms[parentIndex];
	hierarchyWorldTransforms[nodeIndex] = parentWorldTransfom * hierarchyLocalTransforms[nodeIndex];
	for (auto i : node.children)
		CalculateNodeTransform(i, nodeIndex);
}

OvRendering::Resources::ModelNodeTransformCalculator::ModelNodeTransformCalculator(const ModelHierarchy& hierarchy, const ModelAnimation& animation)
	:hierarchy(hierarchy), animation(animation)
{
	hierarchyWorldTransforms.resize(hierarchy.nodes.size());
	hierarchyLocalTransforms.resize(hierarchy.nodes.size());
}

void OvRendering::Resources::ModelNodeTransformCalculator::GetRigBoneTransforms(const MeshRigInfo& rig, OvMaths::FMatrix4* transforms)
{

	for (int i = 0; i < hierarchy.nodes.size(); i++)
	{
		hierarchyLocalTransforms[i] = hierarchy.nodes[i].transform;

		// bug to fix 
		/*auto pNodeAnimation = animation.GetNodeAnimation(hierarchy.nodes[i].name);
		if (nullptr != pNodeAnimation)
			hierarchyLocalTransforms[i] = pNodeAnimation->GetAnimationTransform(0);*/
	}


	CalculateNodeTransform(0, -1);


	int count = rig.boneInfos.size();
	int nodeIndex = hierarchy.GetNodeIndex(rig.nodeName);

	for (int i = 0; i < count; i++)
	{
		int matrixIndex = hierarchy.GetNodeIndex(rig.boneInfos[i].name);
		transforms[i] = OvMaths::FMatrix4::Inverse(hierarchyWorldTransforms[nodeIndex])* hierarchyWorldTransforms[matrixIndex] * rig.boneInfos[i].offset;
	}
}

int OvRendering::Resources::MeshRigInfo::GetBoneIndex(std::string name)
{
	for (int i = 0; i < boneInfos.size(); i++)
	{
		if (boneInfos[i].name == name)
			return i;
	}
	return -1;
}
