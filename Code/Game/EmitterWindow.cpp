#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "ThirdParty/ImGUI/imgui.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/EmitterWindow.hpp"
#include "Game/ParticleEditorBaseModule.hpp"
#include "Game/ParticleEditorShapeModule.hpp"
#include "Game/ParticleEditorSizeOverLifetime.hpp"
#include "Game/ParticleEditorVelocityOverLifetime.hpp"
#include "Game/ParticleEditorRendererModule.hpp"
#include "Game/ParticleEditorColorOverLifetime.hpp"
#include "Game/ParticleEditorPhysicsModule.hpp"
#include "Game/ParticleEditorEmissionModule.hpp"
#include "Game/ParticleEditorRotationOverLifetime.hpp"
#include "Game/ParticleEditorOrbitalVelocityOverLifetime.hpp"
#include "Game/Game.hpp"

EmitterWindow::EmitterWindow(Game* game, const ParticleEmitterData& emitterData, int id)
	:m_game(game), m_id(id)
{
	LoadFromXML(emitterData);
	UpdateParticleEmitterDataFromModules();
}

EmitterWindow::~EmitterWindow()
{
	//clean up shape emitter memory
	if (m_emitterData.m_shape)
		delete m_emitterData.m_shape;

	for (int i = 0; i < m_modules.size(); i++)
	{
		delete m_modules[i];
	}
	m_modules.clear();
}

void EmitterWindow::UpdateWindow()
{
	std::string name = Stringf("Emitter Properties - %s###%d", m_emitterName, m_id);
	ImGui::PushID(m_id);
	{
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImGui::Indent();
			ImGui::InputText("Emitter Name", m_emitterName, sizeof(m_emitterName));
			if (ImGui::IsItemActive())
			{
				m_game->SetActiveInputField();
			}
			ImGui::Checkbox("Stop Render", &m_stopRender);

			for (int i = 0; i < m_modules.size(); i++)
			{
				m_modules[i]->UpdateWindow();
			}

			if (ImGui::Button("Delete Emitter"))
			{
				m_markedForDeletion = true;
			}
			ImGui::Unindent();
		}
	}
	ImGui::PopID();
}

ParticleEmitterData EmitterWindow::GetLatestParticleData()
{
	return m_emitterData;
}

void EmitterWindow::LoadFromXML(const ParticleEmitterData& emitterData)
{
	strcpy_s(m_emitterName, emitterData.m_name.c_str());

	ParticleEditorModule* baseModule = new ParticleEditorBaseModule();
	baseModule->LoadDataFromXML(emitterData);
	m_modules.push_back(baseModule);

	ParticleEditorModule* emissionModule = new ParticleEditorEmissionModule();
	emissionModule->LoadDataFromXML(emitterData);
	m_modules.push_back(emissionModule);

	ParticleEditorModule* shapeModule = new ParticleEditorShapeModule();
	shapeModule->LoadDataFromXML(emitterData);
	m_modules.push_back(shapeModule);

	ParticleEditorModule* sizeModule = new ParticleEditorSizeOverLifetime();
	sizeModule->LoadDataFromXML(emitterData);
	m_modules.push_back(sizeModule);

	ParticleEditorModule* velocityModule = new ParticleEditorVelocityOverLifetime();
	velocityModule->LoadDataFromXML(emitterData);
	m_modules.push_back(velocityModule);

	ParticleEditorModule* orbitalVelModule = new ParticleEditorOrbitalVelocityOverLifetime();
	orbitalVelModule->LoadDataFromXML(emitterData);
	m_modules.push_back(orbitalVelModule);

	ParticleEditorModule* rotationModule = new ParticleEditorRotationOverLifetime();
	rotationModule->LoadDataFromXML(emitterData);
	m_modules.push_back(rotationModule);

	ParticleEditorModule* colorModule = new ParticleEditorColorOverLifetime();
	colorModule->LoadDataFromXML(emitterData);
	m_modules.push_back(colorModule);

	ParticleEditorModule* physicsModule = new ParticleEditorPhysicsModule();
	physicsModule->LoadDataFromXML(emitterData);
	m_modules.push_back(physicsModule);

	ParticleEditorModule* rendererModule = new ParticleEditorRendererModule(m_game);
	rendererModule->LoadDataFromXML(emitterData);
	m_modules.push_back(rendererModule);
}

XmlElement* EmitterWindow::SaveDataToXMLElement(tinyxml2::XMLDocument& xmlDoc)
{
	tinyxml2::XMLElement* rootNode = xmlDoc.NewElement("EmitterData");
	rootNode->SetAttribute("name", m_emitterData.m_name.c_str());
	for (int i = 0; i < m_modules.size(); i++)
	{
		XmlElement* moduleData = m_modules[i]->SaveDataToXMLElement(xmlDoc);
		if (moduleData)
			rootNode->InsertEndChild(moduleData);
	}

	return rootNode;
}

void EmitterWindow::UpdateParticleEmitterDataFromModules()
{
	m_emitterData.m_name = m_emitterName;
	m_emitterData.m_stopRender = m_stopRender;

	for (int i = 0; i < m_modules.size(); i++)
	{
		ParticleEditorModule* module = m_modules[i];

		ParticleEditorBaseModule* baseModule = dynamic_cast<ParticleEditorBaseModule*>(module);
		if (baseModule)
		{
			m_emitterData.m_maxParticles = baseModule->m_maxParticles;
			m_emitterData.m_particleLifetime = FloatRange(baseModule->m_lifetimeMin, baseModule->m_lifetimeMax);
			m_emitterData.m_startSpeed = FloatRange(baseModule->m_speedMin, baseModule->m_speedMax);
			m_emitterData.m_startSize = FloatRange(baseModule->m_sizeMin, baseModule->m_sizeMax);
			m_emitterData.m_startRotationDegrees = FloatRange(baseModule->m_rotationMin, baseModule->m_rotationMax);
			m_emitterData.m_startColor.SetFromFloats(baseModule->m_colorStart);
			//m_emitterData.m_endColor.SetFromFloats(baseModule->m_colorEnd);
			m_emitterData.m_gravityScale = baseModule->m_gravityScale;
			m_emitterData.m_drawOrder = baseModule->m_drawOrder;
			m_emitterData.m_offsetFromWorldPos = baseModule->m_offsetFromBase;
			m_emitterData.m_simulationSpace = baseModule->m_simSpace;

			baseModule->SetDataDirty(true);
			continue;
		}

		ParticleEditorEmissionModule* emissionModule = dynamic_cast<ParticleEditorEmissionModule*>(module);
		if (emissionModule)
		{
			m_emitterData.m_emissionMode = emissionModule->m_emissionMode;
			m_emitterData.m_particlesEmittedPerSecond = emissionModule->m_emissionRate;
			m_emitterData.m_numBurstParticles = emissionModule->m_numBurstParticles;
			m_emitterData.m_burstInterval = emissionModule->m_burstInterval;
			continue;
		}

		ParticleEditorShapeModule* shapeModule = dynamic_cast<ParticleEditorShapeModule*>(module);
		if (shapeModule)
		{
			//clean up memory of the old shape emitter
			if (m_emitterData.m_shape)
			{
				delete m_emitterData.m_shape;
			}

			switch (shapeModule->m_type)
			{
			case EmitterShape::CONE:
			default:
			{
				ConeEmitter* coneEmitter = new ConeEmitter();
				coneEmitter->m_coneHalfAngle = shapeModule->m_coneHalfAngle;
				coneEmitter->m_coneForward = shapeModule->m_coneForward;
				m_emitterData.m_shape = coneEmitter;
				break;
			}
			case EmitterShape::SPHERE:
			{
				SphereEmitter* sphereEmitter = new SphereEmitter();
				sphereEmitter->m_sphereRadius = shapeModule->m_sphereRadius;
				sphereEmitter->m_fromSurface = shapeModule->m_fromSphereSurface;
				m_emitterData.m_shape = sphereEmitter;
				break;
			}
			case EmitterShape::BOX:
			{
				BoxEmitter* boxEmitter = new BoxEmitter();
				boxEmitter->m_dimensions = shapeModule->m_boxDimensions;
				boxEmitter->m_forward = shapeModule->m_boxForward;
				m_emitterData.m_shape = boxEmitter;
				break;
			}
			}

			continue;
		}

		ParticleEditorSizeOverLifetime* sizeOverLifetimeModule = dynamic_cast<ParticleEditorSizeOverLifetime*>(module);
		if (sizeOverLifetimeModule)
		{
			m_emitterData.m_sizeOverLifeXModifier = sizeOverLifetimeModule->m_sizeOverLifeXModifier;
			m_emitterData.m_sizeOverLifetimeX = sizeOverLifetimeModule->m_animatedKeysX;
			m_emitterData.m_sizeOverLifeYModifier = sizeOverLifetimeModule->m_sizeOverLifeYModifier;
			m_emitterData.m_sizeOverLifetimeY = sizeOverLifetimeModule->m_animatedKeysY;
			continue;
		}

		ParticleEditorVelocityOverLifetime* velocityOverLifetimeModule = dynamic_cast<ParticleEditorVelocityOverLifetime*>(module);
		if (velocityOverLifetimeModule)
		{
			m_emitterData.m_volSpeedModifier = velocityOverLifetimeModule->m_speedModifier;
			m_emitterData.m_velocityOverLifetime_X = velocityOverLifetimeModule->m_velXKeys;
			m_emitterData.m_velocityOverLifetime_Y = velocityOverLifetimeModule->m_velYKeys;
			m_emitterData.m_velocityOverLifetime_Z = velocityOverLifetimeModule->m_velZKeys;
			m_emitterData.m_dragModifier = velocityOverLifetimeModule->m_dragModifier;
			m_emitterData.m_dragOverLifetime = velocityOverLifetimeModule->m_dragKeys;
			continue;
		}

		ParticleEditorOrbitalVelocityOverLifetime* orbitalVelOverLifetimeModule = dynamic_cast<ParticleEditorOrbitalVelocityOverLifetime*>(module);
		if (orbitalVelOverLifetimeModule)
		{
			m_emitterData.m_orbitalVelocityModifier = orbitalVelOverLifetimeModule->m_velocityModifier;
			m_emitterData.m_orbitalVelOverLifetime = orbitalVelOverLifetimeModule->m_velKeys;
			m_emitterData.m_orbitalRadiusModifier = orbitalVelOverLifetimeModule->m_radiusModifier;
			m_emitterData.m_orbitalRadiusOverLifetime = orbitalVelOverLifetimeModule->m_radiusKeys;
			m_emitterData.m_orbitalVelocityAxis = orbitalVelOverLifetimeModule->m_orbitAxis;
			continue;
		}

		ParticleEditorRotationOverLifetime* rotOverLifetimeModule = dynamic_cast<ParticleEditorRotationOverLifetime*>(module);
		if (rotOverLifetimeModule)
		{
			m_emitterData.m_rotationModifier = rotOverLifetimeModule->m_rotationModifier;
			m_emitterData.m_rotationOverLifetime = rotOverLifetimeModule->m_rotKeys;
			continue;
		}

		ParticleEditorRendererModule* rendererModule = dynamic_cast<ParticleEditorRendererModule*>(module);
		if (rendererModule)
		{
			m_emitterData.m_renderMode = rendererModule->m_renderMode;
			m_emitterData.m_textureFilepath = rendererModule->m_textureFilePath;
			m_emitterData.m_isSpriteSheetTexture = rendererModule->m_isSpriteSheet;
			m_emitterData.m_spriteSheetGridLayout = IntVec2(rendererModule->m_spriteDimensions[0], rendererModule->m_spriteDimensions[1]);
			m_emitterData.m_blendMode = rendererModule->m_blendMode;
			m_emitterData.m_sortParticles = rendererModule->m_sortParticles;
			continue;
		}

		ParticleEditorColorOverLifetime* colorOverLifetimeModule = dynamic_cast<ParticleEditorColorOverLifetime*>(module);
		if (colorOverLifetimeModule)
		{
			m_emitterData.m_colorOverLifetime = colorOverLifetimeModule->m_colorKeys;
			continue;
		}

		ParticleEditorPhysicsModule* physicsModule = dynamic_cast<ParticleEditorPhysicsModule*>(module);
		if (physicsModule)
		{
			m_emitterData.m_pointAttractors = physicsModule->m_pointAttractors;
			continue;
		}
	}
}

bool EmitterWindow::IsMarkedForDeletion() const
{
	return m_markedForDeletion;
}

bool EmitterWindow::IsAnyModuleDataDirty() const
{
	for (int i = 0; i < m_modules.size(); i++)
	{
		if (m_modules[i]->IsDataDirty())
			return true;
	}

	return false;
}

void EmitterWindow::SetAllModuleDataDirty(bool dirty)
{
	for (int i = 0; i < m_modules.size(); i++)
	{
		m_modules[i]->SetDataDirty(dirty);
	}
}

