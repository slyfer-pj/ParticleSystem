#pragma once
#include <string>
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Game/ParticleEditorModule.hpp"

class ParticleEditorEmissionModule : public ParticleEditorModule
{
public:
	void LoadDataFromXML(const ParticleEmitterData& emitterData) override;
	XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) override;
	void UpdateWindow() override;

public:
	EmissionMode m_emissionMode = EmissionMode::CONSTANT;
	float m_emissionRate = 10.f;
	float m_numBurstParticles = 100.f;
	float m_burstInterval = 5.f;

private:
	std::string m_filePath;
};