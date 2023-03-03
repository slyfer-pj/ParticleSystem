#pragma once
#include <vector>
#include <string>
#include "Engine/Math/Vec3.hpp"

class Game;
class EmitterWindow;
struct ParticleEmitterData;

class ParticleEditor
{
public:
	ParticleEditor(Game* game, const std::vector<ParticleEmitterData>& emitterData, const char* filepath);
	~ParticleEditor();
	void UpdateWindow();
	bool IsEditorDataDirty() const;
	void SetEditorDataDirty(bool dirty);
	std::vector<ParticleEmitterData> GetLatestEmitterData();
	bool RespawnSystem() const;

public:
	Vec3 m_particleSystemPos;

private:
	std::string m_filepath;
	std::vector<EmitterWindow*> m_emitterWindows;
	Game* m_game = nullptr;
	bool m_respawn = false;

private:
	void SaveDataToXML();
	void UpdateAndSaveData();
};