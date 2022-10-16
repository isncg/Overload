#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>

#include "OvRendering/Resources/ModelHierarchy.h"
#include <iostream>
#include <cassert>
int OvRendering::Resources::ModelHierarchyNode::GetIndex(bool cached)
{
	if (cached)
		return index;
	for (int i = 0; i < m_hierarchy->nodes.size(); i++)
	{
		if (&m_hierarchy->nodes[i] == this)
			return i;
	}
	return -1;
}

OvRendering::Resources::ModelHierarchyNode::ModelHierarchyNode(const aiScene* scene, aiNode* node)
{
	this->m_name = node->mName.C_Str();
	this->m_localTransform = *(OvMaths::FMatrix4*)&node->mTransformation;

	for (int i = 0; i < node->mNumMeshes; i++)
	{
		this->m_meshNames.push_back(scene->mMeshes[node->mMeshes[i]]->mName.C_Str());
	}
}

OvRendering::Resources::ModelNodeAnimation::ModelNodeAnimation(ModelHierarchy* hierarchy, aiNodeAnim* anim)
{
	this->m_hierarchy = hierarchy;
	this->m_hierarchyNodeName = anim->mNodeName.C_Str();
	for (int i = 0; i < anim->mNumPositionKeys; i++)
		this->positions.emplace_back(anim->mPositionKeys[i].mTime, *(OvMaths::FVector3*)&anim->mPositionKeys[i].mValue);

	for (int i = 0; i < anim->mNumRotationKeys; i++)
		this->rotations.emplace_back(anim->mRotationKeys[i].mTime, *(OvMaths::FQuaternion*)&anim->mRotationKeys[i].mValue);

	for (int i = 0; i < anim->mNumScalingKeys; i++)
		this->scales.emplace_back(anim->mScalingKeys[i].mTime, *(OvMaths::FVector3*)&anim->mScalingKeys[i].mValue);

}

template<class T>
T Interpolate(std::vector<std::tuple<double, T>> keys, double time)
{
	int len = keys.size();
	assert(len > 0);
	if (time < std::get<0>(keys.front()))
		return std::get<1>(keys.front());
	int left = -1;
	int right = -1;
	for (int i = 1; i < len; i++)
	{
		if (std::get<0>(keys[i]) > time)
		{
			left = i - 1;
			right = i;
			break;
		}
	}
	if (left < 0 || right < 0)
		return std::get<1>(keys[len -1]);

	T& vl = std::get<1>(keys[left]);
	T& vr = std::get<1>(keys[right]);

	double d = std::get<0>(keys[right]) - std::get<0>(keys[left]);
	double dl = time - std::get<0>(keys[left]);
	double dr = std::get<0>(keys[right]) - time;

	return (vl * dl + vr * dr) / d;
}

OvMaths::FMatrix4 OvRendering::Resources::ModelNodeAnimation::GetAnimationTransform(double time) const
{
	auto position = Interpolate(positions, time);// getvec(find(time, positions), positions);
	auto rotation = Interpolate(rotations, time);//getqua(find(time, rotations), rotations);
	auto scale = Interpolate(scales, time);

	return OvMaths::FMatrix4::Translation(position) * OvMaths::FQuaternion::ToMatrix4(OvMaths::FQuaternion::Normalize(rotation)) * OvMaths::FMatrix4::Scaling(scale);

}

OvRendering::Resources::ModelHierarchyNode* OvRendering::Resources::ModelHierarchy::GetNode(std::string name)
{
	for (auto& it : this->nodes)
		if (it.m_name == name)
			return &it;
	return nullptr;
}

int OvRendering::Resources::ModelHierarchy::GetNodeIndex(std::string name) const
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].m_name == name)
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

	auto& mnode = nodes.emplace_back(scene, node);
	mnode.m_hierarchy = this;
	int index = mnode.GetIndex();
	mnode.index = index;
	mnode.parent = parentIndex;
	if (parentIndex >= 0)
	{
		nodes[parentIndex].m_childrenIndices.insert(index);
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
	std::cout << node.m_name << std::endl;
	for (int i = 0; i < 4; i++)
	{
		printIndent(depth + 4);
		for (int j = 0; j < 4; j++)
			std::cout << "\t" << node.m_localTransform.data[i * 4 + j] << "  ";
		std::cout << std::endl;
	}
	printIndent(depth+4);
	std::cout << "Meshes(" << node.m_meshNames.size() << "):  ";
	for (auto& n : node.m_meshNames)
		std::cout << n <<"  ";
	std::cout << std::endl;
	for (auto childIndex : node.m_childrenIndices)
		DumpNodeTree(childIndex, depth + 1);
}

void OvRendering::Resources::ModelHierarchy::DumpMeshList()
{
	//for(auto& m:)
}

OvRendering::Resources::ModelHierarchyAnimation::ModelHierarchyAnimation(ModelHierarchy* hierarchy, aiAnimation* anim)
{
	this->hierarchy = hierarchy;
	this->m_animName = anim->mName.C_Str();
	for (int i = 0; i < anim->mNumChannels; i++)
		this->m_modelNodeAnimations.emplace_back(hierarchy, anim->mChannels[i]);
}

const OvRendering::Resources::ModelNodeAnimation* OvRendering::Resources::ModelHierarchyAnimation::GetNodeAnimation(std::string name) const
{
	for (auto& n : m_modelNodeAnimations)
		if (n.m_hierarchyNodeName == name)
			return &n;
	return nullptr;
}

OvRendering::Resources::MeshRigBoneInfo::MeshRigBoneInfo(aiBone& bone)
	:m_boneName(bone.mName.C_Str()), m_worldToRigSpaceMatrix(*(OvMaths::FMatrix4*)&bone.mOffsetMatrix)
{}

OvRendering::Resources::MeshRigBoneInfo::MeshRigBoneInfo(std::string name, OvMaths::FMatrix4& offset)
	:m_boneName(name), m_worldToRigSpaceMatrix(offset)
{}

void OvRendering::Resources::ModelNodeTransformCalculator::CalculateNodeTransform(int nodeIndex, int parentIndex)
{
	auto& node = m_hierarchy.nodes[nodeIndex];
	auto& parentWorldTransfom = parentIndex < 0 ? OvMaths::FMatrix4::Identity : hierarchyWorldTransforms[parentIndex];
	hierarchyWorldTransforms[nodeIndex] = parentWorldTransfom * hierarchyLocalTransforms[nodeIndex];
	for (auto i : node.m_childrenIndices)
		CalculateNodeTransform(i, nodeIndex);
}

OvRendering::Resources::ModelNodeTransformCalculator::ModelNodeTransformCalculator(const ModelHierarchy& hierarchy, const ModelHierarchyAnimation& animation)
	:m_hierarchy(hierarchy), animation(animation)
{
	hierarchyWorldTransforms.resize(hierarchy.nodes.size());
	hierarchyLocalTransforms.resize(hierarchy.nodes.size());
}

void OvRendering::Resources::ModelNodeTransformCalculator::GetRigBoneTransforms(const MeshRigInfo& rig, OvMaths::FMatrix4* transforms)
{

	for (int i = 0; i < m_hierarchy.nodes.size(); i++)
	{
		hierarchyLocalTransforms[i] = m_hierarchy.nodes[i].m_localTransform;

		// bug to fix 
		/*auto pNodeAnimation = animation.GetNodeAnimation(hierarchy.nodes[i].name);
		if (nullptr != pNodeAnimation)
			hierarchyLocalTransforms[i] = pNodeAnimation->GetAnimationTransform(0);*/
	}


	CalculateNodeTransform(0, -1);


	int count = rig.m_boneInfos.size();
	int nodeIndex = m_hierarchy.GetNodeIndex(rig.m_meshNodeName);

	for (int i = 0; i < count; i++)
	{
		int matrixIndex = m_hierarchy.GetNodeIndex(rig.m_boneInfos[i].m_boneName);
		transforms[i] = hierarchyWorldTransforms[matrixIndex] * rig.m_boneInfos[i].m_worldToRigSpaceMatrix;
	}
}

int OvRendering::Resources::MeshRigInfo::GetBoneIndex(std::string name)
{
	for (int i = 0; i < m_boneInfos.size(); i++)
	{
		if (m_boneInfos[i].m_boneName == name)
			return i;
	}
	return -1;
}
