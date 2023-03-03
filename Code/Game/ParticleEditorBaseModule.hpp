#pragma once
#include <string>
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Game/ParticleEditorModule.hpp"

class ParticleEditorBaseModule : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	int m_maxParticles = 0;
	float m_lifetimeMin = 0.f;
	float m_lifetimeMax = 0.f;
	float m_speedMin = 0.f;
	float m_speedMax = 0.f;
	float m_sizeMin = 0.f;
	float m_sizeMax = 0.f;
	float m_rotationMin = 0.f;
	float m_rotationMax = 0.f;
	float m_colorStart[4] = { 0.f };
	int m_gravityScale = 0;
	int m_drawOrder = 0;
	Vec3 m_offsetFromBase;
	SimulationSpace m_simSpace = SimulationSpace::LOCAL;

private:
	std::string m_filePath;
};