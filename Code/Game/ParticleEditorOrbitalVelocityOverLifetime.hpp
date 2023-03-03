#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"
#include "Game/CurveEditor.hpp"

class ParticleEditorOrbitalVelocityOverLifetime : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	float m_velocityModifier = 0.f;
	AnimatedCurve<float> m_velKeys;
	float m_radiusModifier = 0.f;
	AnimatedCurve<float> m_radiusKeys;
	Vec3 m_orbitAxis;

private:
	CurveEditor<float>* m_curveEditor = nullptr;
};