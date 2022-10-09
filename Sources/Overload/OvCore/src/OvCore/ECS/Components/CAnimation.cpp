#include <OvUI/Widgets/Drags/DragFloat.h>
#include <OvUI/Widgets/Selection/ComboBox.h>
#include <OvUI/Plugins/DataDispatcher.h>

#include "OvCore/ECS/Components/CAnimation.h"
#include "OvCore/ECS/Components/CModelRenderer.h"
#include "OvCore/ECS/Actor.h"
#include "OvRendering/Resources/Animation.h"

const OvRendering::Resources::Animation* OvCore::ECS::Components::CAnimation::GetAnimationFromModel()
{
	auto pModelRenderer = owner.GetComponent<CModelRenderer>();
	if (!pModelRenderer)
		return nullptr;
	return pModelRenderer->GetModel()->GetAnimation();
}

OvCore::ECS::Components::CAnimation::CAnimation(ECS::Actor& p_owner) : AComponent(p_owner)
{

}

void OvCore::ECS::Components::CAnimation::SetSamplePos(float time)
{
	m_samplePos = time;
	auto anim = GetAnimationFromModel();
	if (anim)
	{
		const size_type size = anim->meshBoneAnimations.size();
		m_meshBones.boneCount = size;
		m_meshBones.bones.resize(size);
		for (int i = 0; i < m_meshBones.boneCount; ++i)
		{
			int id = anim->meshBoneAnimations[i].boneId;
			anim->meshBoneAnimations[i].GetTransform(time, m_meshBones.bones[id]);
		}
	}
	else
	{
		m_meshBones.boneCount = 0;		
	}
}

const OvRendering::Resources::MeshBones* OvCore::ECS::Components::CAnimation::GetBones()
{
	return &m_meshBones;
}

std::string OvCore::ECS::Components::CAnimation::GetName()
{
	return "Animation";
}

void OvCore::ECS::Components::CAnimation::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

void OvCore::ECS::Components::CAnimation::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

void OvCore::ECS::Components::CAnimation::OnInspector(OvUI::Internal::WidgetContainer& p_root)
{
}

