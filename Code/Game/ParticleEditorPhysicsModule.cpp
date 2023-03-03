#include "Game/ParticleEditorPhysicsModule.hpp"

void ParticleEditorPhysicsModule::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_pointAttractors = emitterData.m_pointAttractors;
}

XmlElement* ParticleEditorPhysicsModule::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* physicsElement = doc.NewElement("Physics");
	for (int i = 0; i < m_pointAttractors.size(); i++)
	{
		XmlElement* keyElement = doc.NewElement("Point");
		keyElement->SetAttribute("offset", m_pointAttractors[i].m_offsetFromEmitter.ToXMLString().c_str());
		keyElement->SetAttribute("strength", m_pointAttractors[i].m_strength);
		physicsElement->InsertEndChild(keyElement);
	}
	return physicsElement;
}

void ParticleEditorPhysicsModule::UpdateWindow()
{
	int indexToDeleteAt = -1;
	if(ImGui::CollapsingHeader("Physics"))
	{
		ImGui::BeginDisabled(m_pointAttractors.size() >= 8);
		{
			if (ImGui::Button("Add Attractors"))
			{
				PointAttractor attractor;
				m_pointAttractors.push_back(attractor);
			}
		}
		ImGui::EndDisabled();

		for (int i = 0; i < m_pointAttractors.size(); i++)
		{
			PointAttractor& attractor = m_pointAttractors[i];
			if(ImGui::TreeNode(Stringf("Attractor %d", i).c_str()))
			{
				ImGui::InputFloat3("Offset From Emitter", &(attractor.m_offsetFromEmitter.x), "%.2f");
				ImGui::InputFloat("Strength", &attractor.m_strength, 1.f, 2.f, "%.2f");
				if (ImGui::Button("Delete key"))
				{
					indexToDeleteAt = i;
				}
				ImGui::TreePop();
			}
		}
	}

	if (indexToDeleteAt > -1)
	{
		m_pointAttractors.erase(m_pointAttractors.begin() + indexToDeleteAt);
	}
}
