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
	class ModelHierarchyNode;
	class ModelNodeAnimation;
	class MeshRigInfo;

	class ModelHierarchyNode
	{
	public:
		ModelHierarchy* m_hierarchy;
		std::string m_name;
		OvMaths::FMatrix4 m_localTransform;
		int index;
		int parent;
		std::set<int> m_childrenIndices;
		std::vector<std::string> m_meshNames;
		int GetIndex(bool cached = false);
		ModelHierarchyNode(const aiScene* scene, aiNode* node);
	};


	class ModelNodeAnimation
	{
		friend class ModelHierarchy;
		bool m_isEmpty;
	public:
		std::string m_hierarchyNodeName;
		ModelHierarchy* m_hierarchy;
		std::vector<std::tuple<double, OvMaths::FVector3>> positions;
		std::vector<std::tuple<double, OvMaths::FQuaternion>> rotations;
		std::vector<std::tuple<double, OvMaths::FVector3>> scales;

		OvMaths::FMatrix4 GetAnimationTransform(double time) const;
		ModelNodeAnimation(ModelHierarchy* hierarchy, aiNodeAnim* anim);
	};


	class ModelHierarchyAnimation
	{
		friend class ModelHierarchy;
		ModelHierarchy* hierarchy;
	public:
		std::string m_animName;
		std::vector<ModelNodeAnimation> m_modelNodeAnimations;
		ModelHierarchyAnimation(ModelHierarchy* hierarchy, aiAnimation* anim);
		const ModelNodeAnimation* GetNodeAnimation(std::string name) const;

		//void CalculateTransforms(double time, MeshRigInfo& rigInfo, OvMaths::FMatrix4* transforms);
	};

	class ModelNodeTransformCalculator
	{
		const ModelHierarchy& m_hierarchy;
		const ModelHierarchyAnimation& animation;
		std::vector<OvMaths::FMatrix4> hierarchyLocalTransforms;
		std::vector<OvMaths::FMatrix4> hierarchyWorldTransforms;
		void CalculateNodeTransform(int nodeIndex, int parentIndex);
	public:

		ModelNodeTransformCalculator(const ModelHierarchy& hierarchy, const ModelHierarchyAnimation& animation);
		void GetRigBoneTransforms(const MeshRigInfo& rig, OvMaths::FMatrix4* transforms);
	};

	class ModelHierarchy
	{
		void CreateChildNode(const aiScene* scene, aiNode* node, int rootIndex);
	public:
		std::vector<ModelHierarchyNode> nodes;
		std::vector<ModelHierarchyAnimation> animations;
		ModelHierarchyNode* GetNode(std::string name);
		int GetNodeIndex(std::string name) const;
		void Init(const aiScene* scene);

		void DumpNodeTree(int index = 0, int depth = 0);
		void DumpMeshList();
	};

	class MeshRigBoneInfo
	{
	public:
		std::string m_boneName;
		OvMaths::FMatrix4 m_worldToRigSpaceMatrix;
		MeshRigBoneInfo(aiBone& bone);
		MeshRigBoneInfo(std::string name, OvMaths::FMatrix4& offset);
	};


	class MeshRigInfo
	{
	public:
		std::string m_meshName;
		std::string m_meshNodeName;
		std::vector<MeshRigBoneInfo> m_boneInfos;
		int GetBoneIndex(std::string name);
	};

	class AnimationPlayCtrl
	{
	public:
		OvRendering::Resources::ModelHierarchyAnimation* m_animation;
		OvRendering::Resources::ModelNodeTransformCalculator* m_calculator;
		double time;
	};
}

