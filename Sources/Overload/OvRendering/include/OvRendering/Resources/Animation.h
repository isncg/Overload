#pragma once
#include <string>
#include <vector>
#include "OvMaths/FQuaternion.h"
#include "OvMaths/FMatrix4.h"
#include "OvMaths/FVector3.h"
#include <set>


struct aiNode;

namespace OvRendering::Resources
{
	struct MeshBoneAnimationPositionKey
	{
		double time;
		OvMaths::FVector3 value;
	};

	struct MeshBoneAnimationRotationKey
	{
		double time;
		OvMaths::FQuaternion value;
	};

	struct MeshBoneAnimationScaleKey
	{
		double time;
		OvMaths::FVector3 value;
	};

	struct MeshBoneTransform
	{
		OvMaths::FVector3 position;
		OvMaths::FQuaternion rotation;
		OvMaths::FVector3 scale;
	};

	class MeshBoneAnimation
	{
	public:
		std::string boneName;
		int boneId;
		std::set<int> children;
		std::vector<MeshBoneAnimationPositionKey> positions;
		std::vector<MeshBoneAnimationRotationKey> rotations;
		std::vector<MeshBoneAnimationScaleKey> scales;
		OvMaths::FMatrix4 offset = OvMaths::FMatrix4::Identity;
		void GetTransform(double time, MeshBoneTransform& result) const;
		void GetTransform(double time, OvMaths::FMatrix4& result) const;
	};

	


	class Animation
	{
		
		void UpdateTransform(double time, int boneId, OvMaths::FMatrix4& parentTransform, OvMaths::FMatrix4* allTransformMatrix);
		void InitTree(aiNode* node, int parentBoneId);
	public:
		std::vector<MeshBoneAnimation> meshBoneAnimations;
		void UpdateTransforms(double time, OvMaths::FMatrix4* allTransformMatrix);
		void InitTree(aiNode* node);
	};
}