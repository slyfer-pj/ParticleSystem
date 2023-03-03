#include "Game/ParticleEditorBaseModule.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/ImGUI/imgui.h"

constexpr int SIM_SPACE_TYPE_COUNT = 2;
const char* SIM_SPACE_TYPES[SIM_SPACE_TYPE_COUNT] = { "Local", "World" };

void ParticleEditorBaseModule::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_maxParticles = emitterData.m_maxParticles;
	m_lifetimeMin = emitterData.m_particleLifetime.m_min;
	m_lifetimeMax = emitterData.m_particleLifetime.m_max;
	m_speedMin = emitterData.m_startSpeed.m_min;
	m_speedMax = emitterData.m_startSpeed.m_max;
	m_sizeMin = emitterData.m_startSize.m_min;
	m_sizeMax = emitterData.m_startSize.m_max;
	m_rotationMin = emitterData.m_startRotationDegrees.m_min;
	m_rotationMax = emitterData.m_startRotationDegrees.m_max;
	emitterData.m_startColor.GetAsFloats(m_colorStart);
	m_gravityScale = emitterData.m_gravityScale;
	m_drawOrder = emitterData.m_drawOrder;
	m_offsetFromBase = emitterData.m_offsetFromWorldPos;
	m_simSpace = emitterData.m_simulationSpace;
}

XmlElement* ParticleEditorBaseModule::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* baseModuleElement = doc.NewElement("Base");
	baseModuleElement->SetAttribute("order", m_drawOrder);
	baseModuleElement->SetAttribute("offset", m_offsetFromBase.ToXMLString().c_str());
	baseModuleElement->SetAttribute("maxParticles", m_maxParticles);
	baseModuleElement->SetAttribute("lifetime", FloatRange(m_lifetimeMin, m_lifetimeMax).ToXMLString().c_str());
	baseModuleElement->SetAttribute("speed", FloatRange(m_speedMin, m_speedMax).ToXMLString().c_str());
	baseModuleElement->SetAttribute("size", FloatRange(m_sizeMin, m_sizeMax).ToXMLString().c_str());
	baseModuleElement->SetAttribute("rotation", FloatRange(m_rotationMin, m_rotationMax).ToXMLString().c_str());
	Rgba8 startColor;
	startColor.SetFromFloats(m_colorStart);
	baseModuleElement->SetAttribute("startColor", startColor.ToXMLString().c_str());
	baseModuleElement->SetAttribute("gravity", m_gravityScale);
	baseModuleElement->SetAttribute("simspace", SIM_SPACE_TYPES[static_cast<int>(m_simSpace)]);
	return baseModuleElement;
}

void ParticleEditorBaseModule::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Base Module"))
	{
		ImGui::InputInt("Max Particles", &m_maxParticles);
		ImGui::DragFloatRange2("Lifetime", &m_lifetimeMin, &m_lifetimeMax, 1.f, 0.f, 0.f, "%.2f");
		ImGui::DragFloatRange2("Speed", &m_speedMin, &m_speedMax, 1.f, 0.f, 0.f, "%.2f");
		ImGui::DragFloatRange2("Size", &m_sizeMin, &m_sizeMax, 1.f, 0.f, 0.f, "%.2f");
		ImGui::DragFloatRange2("Rotation", &m_rotationMin, &m_rotationMax, 1.f, 0.f, 0.f, "%.2f");
		ImGui::ColorEdit4("Start Color", m_colorStart);
		ImGui::InputInt("Gravity Scale", &m_gravityScale);
		ImGui::InputFloat3("Offset From Base Position", &(m_offsetFromBase.x), "%.2f");
		ImGui::InputInt("Draw Order", &m_drawOrder);
		ImGui::Combo("SimSpace", (int*)&m_simSpace, SIM_SPACE_TYPES, SIM_SPACE_TYPE_COUNT);
	}
}
