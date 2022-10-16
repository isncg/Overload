#pragma once

#include "AComponent.h"
#include "OvRendering/Resources/Mesh.h"
#include <OvRendering/Resources/ModelHierarchy.h>

namespace OvRendering::Resources 
{
	class Animation; 
	class MeshBoneAnimation;
}


namespace OvCore::ECS { class Actor; }

namespace OvCore::ECS::Components
{
	class CAnimation : public AComponent
	{
	private:
		//const OvRendering::Resources::Animation* m_pAnim;
		float m_samplePos = 0;
		OvRendering::Resources::MeshBones m_meshBones;
	public:
		OvRendering::Resources::ModelHierarchy* GetModelHierarchy();
		std::vector<OvRendering::Resources::AnimationPlayCtrl> playCtrls;
		/**
		* Constructor
		* @param p_owner
		*/
		CAnimation(ECS::Actor& p_owner);

		/**
		* Destructor
		*/
		~CAnimation() = default;
		bool isPlaying = false;
		bool isPaused = false;

		const OvRendering::Resources::MeshBones* GetBones();
		virtual std::string GetName() override;
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnInspector(OvUI::Internal::WidgetContainer& p_root) override;
	};
}
