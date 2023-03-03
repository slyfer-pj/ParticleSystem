#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class ParticleEditorColorOverLifetime : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	std::vector<AnimatedValueKey<Rgba8>> m_colorKeys;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
};