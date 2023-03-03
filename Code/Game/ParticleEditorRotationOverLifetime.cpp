#include "Game/ParticleEditorRotationOverLifetime.hpp"

void ParticleEditorRotationOverLifetime::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_rotKeys = emitterData.m_rotationOverLifetime;
	m_rotationModifier = emitterData.m_rotationModifier;
}

XmlElement* ParticleEditorRotationOverLifetime::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* rolElement = doc.NewElement("RotationOverLifetime");
	rolElement->SetAttribute("modifier", m_rotationModifier);
	WriteKeysToXMLElement(doc, rolElement, m_rotKeys);

	return rolElement;
}

void ParticleEditorRotationOverLifetime::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Rotation Over Lifetime"))
	{
		ImGui::InputFloat("Rotation Modifier", &m_rotationModifier, 0.f, 0.f, "%.2f");
		if (ImGui::Button("Open Curve Editor###rot"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_rotKeys);
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
