#include "ThirdParty/ImGUI/imgui.h"
#include "Game/ParticleEditor.hpp"
#include "Game/EmitterWindow.hpp"

ParticleEditor::ParticleEditor(Game* game, const std::vector<ParticleEmitterData>& emitterData, const char* filepath)
	:m_filepath(filepath), m_game(game)
{
	for (int i = 0; i < emitterData.size(); i++)
	{
		EmitterWindow* newEmitter = new EmitterWindow(game, emitterData[i], i);
		m_emitterWindows.push_back(newEmitter);
	}
}

ParticleEditor::~ParticleEditor()
{
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		delete m_emitterWindows[i];
	}
	m_emitterWindows.clear();
}

void ParticleEditor::UpdateWindow()
{
	ImGui::Begin("Particle System Editor");
	{
		ImGui::InputFloat3("Position", &m_particleSystemPos.x, "%.2f");
		for (int i = 0; i < m_emitterWindows.size(); i++)
		{
			m_emitterWindows[i]->UpdateWindow();
		}

		if (ImGui::Button("Save"))
		{
			UpdateAndSaveData();
		}
		if (ImGui::Button("Add Emitter"))
		{
			EmitterWindow* newEmitter = new EmitterWindow(m_game, ParticleEmitterData(), int(m_emitterWindows.size()));
			m_emitterWindows.push_back(newEmitter);
			UpdateAndSaveData();
			m_respawn = true;
		}

	}
	ImGui::End();	

	for (auto iter = m_emitterWindows.begin(); iter != m_emitterWindows.end(); ++iter)
	{
		if ((*iter)->IsMarkedForDeletion())
		{
			m_emitterWindows.erase(iter);
			UpdateAndSaveData();
			m_respawn = true;
			break;
		}
	}
}

bool ParticleEditor::IsEditorDataDirty() const
{
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		if (m_emitterWindows[i]->IsAnyModuleDataDirty())
			return true;
	}

	return false;
}

void ParticleEditor::SetEditorDataDirty(bool dirty)
{
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		m_emitterWindows[i]->SetAllModuleDataDirty(dirty);
	}
}

std::vector<ParticleEmitterData> ParticleEditor::GetLatestEmitterData()
{
	std::vector<ParticleEmitterData> latestEmitterData;
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		latestEmitterData.push_back(m_emitterWindows[i]->GetLatestParticleData());
	}

	return latestEmitterData;
}

bool ParticleEditor::RespawnSystem() const
{
	return m_respawn;
}

void ParticleEditor::SaveDataToXML()
{
	tinyxml2::XMLDocument particleSystemData;
	XmlElement* rootNode = particleSystemData.NewElement("ParticleSystem");
	particleSystemData.InsertFirstChild(rootNode);
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		XmlElement* emitterData = m_emitterWindows[i]->SaveDataToXMLElement(particleSystemData);
		if (emitterData)
		{
			rootNode->InsertEndChild(emitterData);
		}
	}
	tinyxml2::XMLError result = particleSystemData.SaveFile(m_filepath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Error in writing emitter data to file");
}

void ParticleEditor::UpdateAndSaveData()
{
	for (int i = 0; i < m_emitterWindows.size(); i++)
	{
		m_emitterWindows[i]->UpdateParticleEmitterDataFromModules();
	}
	SaveDataToXML();
}
