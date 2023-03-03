#pragma once
#include <vector>
#include "Engine/Renderer/ParticleEmitterData.hpp"

class Game;
class ParticleEditorModule;

class EmitterWindow
{
public:
	EmitterWindow(Game* game, const ParticleEmitterData& emitterData, int id);
	~EmitterWindow();
	void UpdateWindow();
	ParticleEmitterData GetLatestParticleData();
	bool IsAnyModuleDataDirty() const;
	void SetAllModuleDataDirty(bool dirty);
	void LoadFromXML(const ParticleEmitterData& emitterData);
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& xmlDoc);
	void UpdateParticleEmitterDataFromModules();
	bool IsMarkedForDeletion() const;

private:
	std::string m_filePath;
	std::vector<ParticleEditorModule*> m_modules = {};
	ParticleEmitterData m_emitterData;
	Game* m_game = nullptr;
	int m_id;
	char m_emitterName[100] = { '\0' };
	bool m_stopRender = false;
	bool m_markedForDeletion = false;
};