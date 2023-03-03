#pragma once
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "Game/ParticleEditorModule.hpp"

class ParticleEditorShapeModule : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	EmitterShape m_type = EmitterShape::CONE;
	float m_coneHalfAngle = 30.f;
	Vec3 m_coneForward = Vec3(0.f, 0.f, 1.f);
	float m_sphereRadius = 5.f;
	bool m_fromSphereSurface = false;
	Vec3 m_boxDimensions = Vec3(5.f, 5.f, 2.f);
	Vec3 m_boxForward = Vec3(0.f, 0.f, 1.f);


private:
	std::string GetStringForEmitterType(EmitterShape type);
};