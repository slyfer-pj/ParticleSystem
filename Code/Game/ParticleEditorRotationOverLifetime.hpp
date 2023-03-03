#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class ParticleEditorRotationOverLifetime : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	float m_rotationModifier = 1.f;
	AnimatedCurve<float> m_rotKeys;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
};