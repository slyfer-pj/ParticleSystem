#include "Game/ParticleEditorVelocityOverLifetime.hpp"

void ParticleEditorVelocityOverLifetime::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_speedModifier = emitterData.m_volSpeedModifier;
	m_velXKeys = emitterData.m_velocityOverLifetime_X;
	m_velYKeys = emitterData.m_velocityOverLifetime_Y;
	m_velZKeys = emitterData.m_velocityOverLifetime_Z;
	m_dragModifier = emitterData.m_dragModifier;
	m_dragKeys = emitterData.m_dragOverLifetime;
}

XmlElement* ParticleEditorVelocityOverLifetime::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* volElement = doc.NewElement("VelocityOverLifetime");
	volElement->SetAttribute("modifier", m_speedModifier);

	XmlElement* velX = doc.NewElement("VelocityX");
	volElement->InsertEndChild(velX);
	WriteKeysToXMLElement(doc, velX, m_velXKeys);

	XmlElement* velY = doc.NewElement("VelocityY");
	volElement->InsertEndChild(velY);
	WriteKeysToXMLElement(doc, velY, m_velYKeys);

	XmlElement* velZ = doc.NewElement("VelocityZ");
	volElement->InsertEndChild(velZ);
	WriteKeysToXMLElement(doc, velZ, m_velZKeys);

	XmlElement* drag = doc.NewElement("Drag");
	drag->SetAttribute("modifier", m_dragModifier);
	volElement->InsertEndChild(drag);
	WriteKeysToXMLElement(doc, drag, m_dragKeys);

	return volElement;
}

void ParticleEditorVelocityOverLifetime::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Velocity Over Lifetime"))
	{
		ImGui::InputFloat("Speed Modifier", &m_speedModifier, 0.5f, 2.f, "%.2f");

		ImGui::Text("X");
		ImGui::SameLine();
		if (ImGui::Button("Open Curve Editor##X"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_velXKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_velXKeys);
			}
		}

		ImGui::Text("Y");
		ImGui::SameLine();
		if (ImGui::Button("Open Curve Editor##Y"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_velYKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_velYKeys);
			}
		}

		ImGui::Text("Z");
		ImGui::SameLine();
		if (ImGui::Button("Open Curve Editor##Z"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_velZKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_velZKeys);
			}
		}

		ImGui::Text("Drag");
		ImGui::InputFloat("Drag Modifier", &m_dragModifier, 0.f, 0.f, "%.2f");
		if (ImGui::Button("Open Curve Editor##drag"))
		{
			if (!m_curveEditor)
				m_curveEditor = new CurveEditor<float>(&m_dragKeys);
			else
			{
				delete m_curveEditor;
				m_curveEditor = new CurveEditor<float>(&m_dragKeys);
			}
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
