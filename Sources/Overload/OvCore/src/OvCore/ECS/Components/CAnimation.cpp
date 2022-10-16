#include <OvUI/Widgets/Drags/DragFloat.h>
#include <OvUI/Widgets/Selection/ComboBox.h>
#include <OvUI/Plugins/DataDispatcher.h>

#include "OvCore/ECS/Components/CAnimation.h"
#include "OvCore/ECS/Components/CModelRenderer.h"
#include "OvCore/ECS/Actor.h"

OvRendering::Resources::ModelHierarchy* OvCore::ECS::Components::CAnimation::GetModelHierarchy()
{
	auto pModelRenderer = owner.GetComponent<CModelRenderer>();
	return pModelRenderer->GetModel()->GetHierarchy();
}

OvCore::ECS::Components::CAnimation::CAnimation(ECS::Actor& p_owner) : AComponent(p_owner)
{

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

