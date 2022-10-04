#pragma once
#include <string>
#include <vector>
#include "OvMaths/FQuaternion.h"
#include "OvMaths/FMatrix4.h"
#include "OvMaths/FVector3.h"


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
		int parentBoneId;

		std::vector<MeshBoneAnimationPositionKey> positions;
		std::vector<MeshBoneAnimationRotationKey> rotations;
		std::vector<MeshBoneAnimationScaleKey> scales;
		void GetTransform(double time, MeshBoneTransform& result);
	};

	


	class Animation
	{
	public:
		std::vector<MeshBoneAnimation> meshBoneAnimations;
	};
}