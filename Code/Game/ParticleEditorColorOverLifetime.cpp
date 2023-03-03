#include <algorithm>
#include "Game/ParticleEditorColorOverLifetime.hpp"

void ParticleEditorColorOverLifetime::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_colorKeys = emitterData.m_colorOverLifetime;
}

XmlElement* ParticleEditorColorOverLifetime::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	std::sort(m_colorKeys.begin(), m_colorKeys.end());
	XmlElement* colElement = doc.NewElement("ColorOverLifetime");
	for (int i = 0; i < m_colorKeys.size(); i++)
	{
		XmlElement* keyElement = doc.NewElement("key");
		keyElement->SetAttribute("value", m_colorKeys[i].GetValue().ToXMLString().c_str());
		keyElement->SetAttribute("time", m_colorKeys[i].GetTime());
		colElement->InsertEndChild(keyElement);
	}
	return colElement;
}

void ParticleEditorColorOverLifetime::UpdateWindow()
{
	int keyToDelete = -1;
	if (ImGui::CollapsingHeader("Color Over Lifetime"))
	{
		ImGui::BeginDisabled(m_colorKeys.size() >= 8);
		if (ImGui::Button("Add Keys"))
		{
			std::sort(m_colorKeys.begin(), m_colorKeys.end());
			AnimatedValueKey<Rgba8> color = m_colorKeys[m_colorKeys.size() - 1];
			color.SetTime(color.GetTime() + 0.01f);
			m_colorKeys.push_back(color);
		}
		ImGui::EndDisabled();

		for (int i = 0; i < m_colorKeys.size(); i++)
		{
			if (ImGui::TreeNode(Stringf("Key %d", i).c_str()))
			{
				float keyColor[4];
				float time = m_colorKeys[i].GetTime();
				m_colorKeys[i].GetValue().GetAsFloats(keyColor);
				ImGui::ColorEdit4("Color", keyColor);
				ImGui::SliderFloat("Time", &time, 0.f, 1.f, "%.2f");
				ImGui::BeginDisabled(m_colorKeys.size() <= 2);
				{
					if (ImGui::Button("Delete Key"))
					{
						keyToDelete = i;
					}
				}
				ImGui::EndDisabled();
				m_colorKeys[i].SetTime(time);
				Rgba8 newValue;
				newValue.SetFromFloats(keyColor);
				m_colorKeys[i].SetValue(newValue);

				ImGui::TreePop();
			}
		}
	}

	if (keyToDelete > -1)
	{
		m_colorKeys.erase(m_colorKeys.begin() + keyToDelete);
	}
}
