#include "OvRendering/Resources/Animation.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>

template<class T>
bool SearchTime(const std::vector<T>& keys, double time, T& result)
{
	int len = keys.size();
	if (len < 1)
		return false;
	if (len == 1)
	{
		result.value = keys[0].value;
		return true;
	}
	int i = 0;
	for (; i < keys.size(); i++)
	{
		if (keys[i].time > time)
			break;
	}

	if (i >= len)
	{
		result.value = keys[len - 1].value;
		return true;
	}
	double t1 = keys[i - 1].time;
	double t2 = keys[i].time;
	float factor = (time - t1) / (t2 - t1);
	result.value = keys[i - 1].value * t2 + keys[i].value * (1.0 - t2);
	return true;
	


	/*if (keys.size() == 1)
	{
		result.value = keys[0].value;
		return true;
	}
	int left = 0;
	int right = keys.size() - 1;
	auto vl = keys[left].time;
	auto vr = keys[right].time;
	while (left + 1 < right)
	{
		int middle = (left + right) / 2;
		auto vm = keys[middle].time;
		if (vm > time)
		{
			right = middle;
			vr = vm;
		}
		else
		{
			left = middle;
			vl = vm;
		}
	}
	if (left < right && right < keys.size())
	{
		auto& l = keys[left];
		auto& r = keys[right];
		result.value = (l.value * (r.time - time) + r.value * (time - l.time)) / (r.time - l.time);
		return true;
	}
	return false;*/
}


void OvRendering::Resources::MeshBoneAnimation::GetTransform(double time, MeshBoneTransform& result) const
{
	MeshBoneAnimationPositionKey positions_result;
	MeshBoneAnimationRotationKey rotations_result;
	MeshBoneAnimationScaleKey scales_result;
	if (SearchTime(positions, time, positions_result))
		result.position = positions_result.value;
	else
		result.position = OvMaths::FVector3::Zero;
	if (SearchTime(rotations, time, rotations_result))
		result.rotation = rotations_result.value;
	else
		result.rotation = OvMaths::FQuaternion::Identity;
	if (SearchTime(scales, time, scales_result))
		result.scale = scales_result.value;
	else
		result.scale = OvMaths::FVector3::One;

}

void OvRendering::Resources::MeshBoneAnimation::GetTransform(double time, OvMaths::FMatrix4& result) const
{
	MeshBoneTransform cache;
	GetTransform(time, cache);
	//result = OvMaths::FMatrix4::Translation(cache.position)* OvMaths::FQuaternion::ToMatrix4(OvMaths::FQuaternion::Normalize(cache.rotation))* OvMaths::FMatrix4::Scaling(cache.scale);
	result = OvMaths::FMatrix4::Identity;
}

void OvRendering::Resources::Animation::UpdateTransform(double time, int boneId, OvMaths::FMatrix4& parentTransform, OvMaths::FMatrix4* allTransformMatrix)
{
	int index = boneId - 1;
	auto& anim = meshBoneAnimations[index];
	anim.GetTransform(time, allTransformMatrix[index]);
	allTransformMatrix[index] = parentTransform * allTransformMatrix[index];
	for (auto childId : anim.children)
	{
		UpdateTransform(time, childId, allTransformMatrix[index], allTransformMatrix);
	}
}

void OvRendering::Resources::Animation::UpdateTransforms(double time, OvMaths::FMatrix4* allTransformMatrix)
{
	//allTransformMatrix.resize(meshBoneAnimations.size());
	UpdateTransform(time, meshBoneAnimations.front().boneId, meshBoneAnimations.front().offset, allTransformMatrix);
}

void OvRendering::Resources::Animation::InitTree(aiNode* node, int parentBoneId)
{
	int newBoneId = (int)meshBoneAnimations.size() + 1;
	auto& anim = meshBoneAnimations.emplace_back();
	anim.boneName = node->mName.C_Str();
	anim.boneId = newBoneId;
	anim.offset = *(OvMaths::FMatrix4*)&node->mTransformation;

	if (parentBoneId > 0)
	{
		MeshBoneAnimation& parentAnim = meshBoneAnimations[parentBoneId - 1];
		parentAnim.children.insert(newBoneId);
	}
	for (int i = 0; i < node->mNumChildren; i++)
	{
		InitTree(node->mChildren[i], newBoneId);
	}
}

void OvRendering::Resources::Animation::InitTree(aiNode* node)
{
	InitTree(node, 0);
}