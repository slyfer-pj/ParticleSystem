#include "Game/ParticleEditorShapeModule.hpp"
#include "ThirdParty/ImGUI/imgui.h"

constexpr int ITEM_COUNT = 3;
const char* emitterType[ITEM_COUNT] = { "Cone", "Sphere", "Box" };
const char* emitFrom[2] = { "Volume", "Surface" };

void ParticleEditorShapeModule::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	if (emitterData.m_shape)
	{
		m_type = static_cast<EmitterShape>(emitterData.m_shape->GetShapeTypeNum());
		if (dynamic_cast<ConeEmitter*>(emitterData.m_shape))
		{
			ConeEmitter* coneEmitter = dynamic_cast<ConeEmitter*>(emitterData.m_shape);
			m_coneHalfAngle = coneEmitter->m_coneHalfAngle;
			m_coneForward = coneEmitter->m_coneForward;
		}
		else if (dynamic_cast<SphereEmitter*>(emitterData.m_shape))
		{
			SphereEmitter* sphereEmitter = dynamic_cast<SphereEmitter*>(emitterData.m_shape);
			m_sphereRadius = sphereEmitter->m_sphereRadius;
			m_fromSphereSurface = sphereEmitter->m_fromSurface;
		}
		else if (dynamic_cast<BoxEmitter*>(emitterData.m_shape))
		{
			BoxEmitter* boxEmitter = dynamic_cast<BoxEmitter*>(emitterData.m_shape);
			m_boxForward = boxEmitter->m_forward;
			m_boxDimensions = boxEmitter->m_dimensions;
		}
	}
	else
	{
		m_type = EmitterShape::CONE;
	}
}

XmlElement* ParticleEditorShapeModule::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* shapeModuleElement = doc.NewElement("Shape");
	shapeModuleElement->SetAttribute("shape", GetStringForEmitterType(m_type).c_str());
	shapeModuleElement->SetAttribute("coneHalfAngle", m_coneHalfAngle);
	shapeModuleElement->SetAttribute("coneForward", m_coneForward.ToXMLString().c_str());
	shapeModuleElement->SetAttribute("sphereRadius", m_sphereRadius);
	shapeModuleElement->SetAttribute("fromSurface", m_fromSphereSurface);
	shapeModuleElement->SetAttribute("boxDimensions", m_boxDimensions.ToXMLString().c_str());
	shapeModuleElement->SetAttribute("boxForward", m_boxForward.ToXMLString().c_str());
	return shapeModuleElement;
}

void ParticleEditorShapeModule::UpdateWindow()
{
	
	if (ImGui::CollapsingHeader("Shape Module"))
	{
		ImGui::Combo("Shape", (int*)&m_type, emitterType, ITEM_COUNT);
		if (m_type == EmitterShape::CONE)
		{
			ImGui::InputFloat("Half Angle", &m_coneHalfAngle, 1.f, 1.f, "%.2f");
			ImGui::InputFloat3("Forward", &m_coneForward.x, "%.2f");
		}
		else if(m_type == EmitterShape::SPHERE)
		{
			static int currentSelection = 0;
			ImGui::InputFloat("Radius", &m_sphereRadius, 1.f, 1.f, "%.2f");
			ImGui::Combo("Emit From", &currentSelection, emitFrom, 2);
			m_fromSphereSurface = (currentSelection == 1);
		}
		else if (m_type == EmitterShape::BOX)
		{
			ImGui::InputFloat3("Dimensions", &m_boxDimensions.x, "%.2f");
			ImGui::InputFloat3("Forward", &m_boxForward.x, "%.2f");
		}
	}
} 

std::string ParticleEditorShapeModule::GetStringForEmitterType(EmitterShape type)
{
	switch (type)
	{
	case EmitterShape::CONE: return "Cone";
	case EmitterShape::SPHERE: return "Sphere";
	case EmitterShape::BOX: return "Box";
	default: return "Cone";
	}
}
