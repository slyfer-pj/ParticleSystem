#include "ThirdParty/ImGUI/imgui.h"
#include "Game/ParticleEditorSizeOverLifetime.hpp"
#include "Game/CurveEditor.hpp"

void ParticleEditorSizeOverLifetime::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_sizeOverLifeXModifier = emitterData.m_sizeOverLifeXModifier;
	m_animatedKeysX = emitterData.m_sizeOverLifetimeX;
	m_sizeOverLifeYModifier = emitterData.m_sizeOverLifeYModifier;
	m_animatedKeysY = emitterData.m_sizeOverLifetimeY;
}

XmlElement* ParticleEditorSizeOverLifetime::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* solElement = doc.NewElement("SizeOverLifetime");

	XmlElement* sizeX = doc.NewElement("X");
	sizeX->SetAttribute("modifier", m_sizeOverLifeXModifier);
	solElement->InsertEndChild(sizeX);
	WriteKeysToXMLElement(doc, sizeX, m_animatedKeysX);

	XmlElement* sizeY = doc.NewElement("Y");
	sizeY->SetAttribute("modifier", m_sizeOverLifeYModifier);
	solElement->InsertEndChild(sizeY);
	WriteKeysToXMLElement(doc, sizeY, m_animatedKeysY);

	return solElement;
}

void ParticleEditorSizeOverLifetime::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Size Over Lifetime"))
	{
		ImGui::InputFloat("X Modifier", &m_sizeOverLifeXModifier, 0.f, 0.f, "%.2f");
		ImGui::SameLine();
		if (ImGui::Button("Open Curve Editor###X"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_animatedKeysX, false);
		}
		ImGui::InputFloat("Y Modifier", &m_sizeOverLifeYModifier, 0.f, 0.f, "%.2f");
		ImGui::SameLine();
		if (ImGui::Button("Open Curve Editor###Y"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_animatedKeysY, false);
		}
	}

	if (m_curveEditor)
	{
		if (!m_curveEditor->IsWindowOpen())
		{
			delete m_curveEditor;
			m_curveEditor = nullptr;
		}
		else
		{
			m_curveEditor->UpdateWindow();
		}
	}

}

