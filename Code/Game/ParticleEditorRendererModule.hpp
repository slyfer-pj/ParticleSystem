#pragma once
#include <string>
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class Game;

class ParticleEditorRendererModule : public ParticleEditorModule
{
public:
	ParticleEditorRendererModule(Game* game);
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	RenderMode m_renderMode = RenderMode::BILLBOARD;
	std::string m_textureFilePath;
	bool m_isSpriteSheet = false;
	int m_spriteDimensions[2] = { 1, 1 };
	BlendMode m_blendMode = BlendMode::ALPHA;
	bool m_sortParticles = false;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
	Game* m_game = nullptr;
};
