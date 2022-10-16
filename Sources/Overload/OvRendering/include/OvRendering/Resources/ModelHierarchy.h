#pragma once
#include <string>
#include <set>
#include "OvMaths/FTransform.h"

struct aiScene;
struct aiAnimation;
struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;
struct aiNode;
struct aiBone;

namespace OvRendering::Resources
{
	class ModelHierarchy;
	class ModelNode;
	class ModelNodeAnimation;
	class MeshRigInfo;

	class ModelNode
	{
	public:
		ModelHierarchy* hierarchy;
		std::string name;
		OvMaths::FMatrix4 transform;
		int index;
		int parent;
		std::set<int> children;
		std::vector<std::string> meshNames;
		int GetIndex(bool cached = false);
		ModelNode(const aiScene* scene, aiNode* node);
	};

	struct ModelNodeAnimationKey
	{
		double time;
		union
		{
			OvMaths::FVector3 value_vec;
			OvMaths::FQuaternion value_qua;
		};
		ModelNodeAnimationKey(aiVectorKey& key);
		ModelNodeAnimationKey(aiQuatKey& key);
		ModelNodeAnimationKey(ModelNodeAnimationKey& other);
	};


	class ModelNodeAnimation
	{
		friend class ModelHierarchy;
		bool isEmpty;
	public:
		std::string name;
		ModelHierarchy* hierarchy;
		std::vector<ModelNodeAnimationKey> positions;
		std::vector<ModelNodeAnimationKey> rotations;
		std::vector<ModelNodeAnimationKey> scales;

		OvMaths::FMatrix4 GetAnimationTransform(double time) const;
		ModelNodeAnimation(ModelHierarchy* hierarchy, aiNodeAnim* anim);
	};


	class ModelAnimation
	{
		friend class ModelHierarchy;
		ModelHierarchy* hierarchy;
	public:
		std::string name;
		std::vector<ModelNodeAnimation> nodeAnimations;
		ModelAnimation(ModelHierarchy* hierarchy, aiAnimation* anim);
		const ModelNodeAnimation* GetNodeAnimation(std::string name) const;

		//void CalculateTransforms(double time, MeshRigInfo& rigInfo, OvMaths::FMatrix4* transforms);
	};

	class ModelNodeTransformCalculator
	{
		const ModelHierarchy& hierarchy;
		const ModelAnimation& animation;
		std::vector<OvMaths::FMatrix4> hierarchyLocalTransforms;
		std::vector<OvMaths::FMatrix4> hierarchyWorldTransforms;
		void CalculateNodeTransform(int nodeIndex, int parentIndex);
	public:

		ModelNodeTransformCalculator(const ModelHierarchy& hierarchy, const ModelAnimation& animation);
		void GetRigBoneTransforms(const MeshRigInfo& rig, OvMaths::FMatrix4* transforms);
	};

	class ModelHierarchy
	{
		void CreateChildNode(const aiScene* scene, aiNode* node, int rootIndex);
	public:
		std::vector<ModelNode> nodes;
		std::vector<ModelAnimation> animations;
		ModelNode* GetNode(std::string name);
		int GetNodeIndex(std::string name) const;
		void Init(const aiScene* scene);

		void DumpNodeTree(int index = 0, int depth = 0);
		void DumpMeshList();
	};

	class MeshRigBoneInfo
	{
	public:
		std::string name;
		OvMaths::FMatrix4 offset;
		MeshRigBoneInfo(aiBone& bone);
		MeshRigBoneInfo(std::string name);
	};


	class MeshRigInfo
	{
	public:
		std::string meshName;
		std::string nodeName;
		std::vector<MeshRigBoneInfo> boneInfos;
		int GetBoneIndex(std::string name);
	};

	class AnimationPlayCtrl
	{
	public:
		OvRendering::Resources::ModelAnimation* animation;
		OvRendering::Resources::ModelNodeTransformCalculator* calculator;
		double time;
	};
}

