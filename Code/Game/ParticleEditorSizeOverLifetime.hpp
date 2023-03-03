#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class ParticleEditorSizeOverLifetime : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	float m_sizeOverLifeXModifier = 0.f;
	AnimatedCurve<float> m_animatedKeysX;
	float m_sizeOverLifeYModifier = 0.f;
	AnimatedCurve<float> m_animatedKeysY;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
};