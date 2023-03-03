#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class ParticleEditorVelocityOverLifetime : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	float m_speedModifier = 1.f;
	AnimatedCurve<float> m_velXKeys;
	AnimatedCurve<float> m_velYKeys;
	AnimatedCurve<float> m_velZKeys;
	float m_dragModifier = 0.f;
	AnimatedCurve<float> m_dragKeys;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
};