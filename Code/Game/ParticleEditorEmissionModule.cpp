#include "ThirdParty/ImGUI/imgui.h"
#include "Game/ParticleEditorEmissionModule.hpp"

const char* EMISSION_MODE_TYPES[2] = { "Constant", "Burst" };

void ParticleEditorEmissionModule::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_emissionMode = emitterData.m_emissionMode;
	m_emissionRate = emitterData.m_particlesEmittedPerSecond;
	m_numBurstParticles = emitterData.m_numBurstParticles;
	m_burstInterval = emitterData.m_burstInterval;
}

XmlElement* ParticleEditorEmissionModule::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* emissionModuleElement = doc.NewElement("Emission");
	emissionModuleElement->SetAttribute("mode", EMISSION_MODE_TYPES[static_cast<unsigned int>(m_emissionMode)]);
	emissionModuleElement->SetAttribute("emissionRate", m_emissionRate);
	emissionModuleElement->SetAttribute("numBurstParticles", m_numBurstParticles);
	emissionModuleElement->SetAttribute("burstInterval", m_burstInterval);

	return emissionModuleElement;
}

void ParticleEditorEmissionModule::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Emission Module"))
	{
		ImGui::Combo("Emission Mode", (int*)&m_emissionMode, EMISSION_MODE_TYPES, 2);
		if (m_emissionMode == EmissionMode::CONSTANT)
		{
			ImGui::InputFloat("Emission Rate", &m_emissionRate, 0.f, 0.f, "%.2f");
		}
		else if (m_emissionMode == EmissionMode::BURST)
		{
			ImGui::InputFloat("Num Particles", &m_numBurstParticles, 0.f, 0.f, "%.1f");
			ImGui::InputFloat("Burst Interval", &m_burstInterval, 0.f, 0.f, "%.2f");
		}
	}
}

