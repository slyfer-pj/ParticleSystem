#include "Game/ParticleEditorOrbitalVelocityOverLifetime.hpp"

void ParticleEditorOrbitalVelocityOverLifetime::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_velocityModifier = emitterData.m_orbitalVelocityModifier;
	m_velKeys = emitterData.m_orbitalVelOverLifetime;
	m_radiusModifier = emitterData.m_orbitalRadiusModifier;
	m_radiusKeys = emitterData.m_orbitalRadiusOverLifetime;
	m_orbitAxis = emitterData.m_orbitalVelocityAxis;
}

XmlElement* ParticleEditorOrbitalVelocityOverLifetime::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* orbitalVelElement = doc.NewElement("OrbitalVelocityOverLifetime");
	orbitalVelElement->SetAttribute("forward", m_orbitAxis.ToXMLString().c_str());
	XmlElement* vel = doc.NewElement("Velocity");
	vel->SetAttribute("modifier", m_velocityModifier);
	orbitalVelElement->InsertEndChild(vel);
	WriteKeysToXMLElement(doc, vel, m_velKeys);

	XmlElement* radius = doc.NewElement("Radius");
	radius->SetAttribute("modifier", m_radiusModifier);
	orbitalVelElement->InsertEndChild(radius);
	WriteKeysToXMLElement(doc, radius, m_radiusKeys);

	return orbitalVelElement;
}

void ParticleEditorOrbitalVelocityOverLifetime::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Orbital Velocity Over Lifetime"))
	{
		ImGui::InputFloat("Orbital Velocity", &m_velocityModifier, 0.5f, 2.f, "%.2f");
		if (ImGui::Button("Open Curve Editor##X"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_velKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_velKeys);
			}
		}

		ImGui::InputFloat("Orbital Radius", &m_radiusModifier, 0.5f, 2.f, "%.2f");
		if (ImGui::Button("Open Curve Editor##Y"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_radiusKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_radiusKeys);
			}
		}

		ImGui::InputFloat3("Orbit Axis", &m_orbitAxis.x, "%.2f");
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
