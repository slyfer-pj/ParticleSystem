#pragma once
#include <vector>
#include "Game/Game.hpp"

class Game;
class ParticleSystem;
class Prop;
class Mesh;

struct AtomizerStreakData
{
	ParticleSystem* m_system;
	Vec3 m_moveDirection;
	float m_speed = 0.f;
};

class Zoo
{
public:
	Zoo(Game* game, GameMode zooMode);
	~Zoo();
	void Update(float deltaSeconds);
	void Render() const;
	void ChangeParticleSystemType();

private:
	Game* m_game = nullptr;
	GameMode m_zooMode = GameMode::ZOO;
	std::vector<ParticleSystem*> m_systems;
	ParticleSystem* m_materializeParticle_Hearts = nullptr;
	ParticleSystem* m_materializeParticle_Stars = nullptr;
	float m_materializeParticle_CurrAngle = 0.f;
	float m_materializeParticle_CurrHeight = 0.f;
	Mesh* m_miku = nullptr;
	std::vector<AtomizerStreakData> m_atomizerStreaks;
	float m_domeRadius = 0.f;
	float m_domeYaw  = 0.f;
	unsigned char m_domeAlpha = 255;
	float m_atomizerTimer = 0.f;

private:
	void UpdateParticlePos(float deltaSeconds);
	void UpdateAtomizer(float deltaSeconds);
	void SpawnZooParticles();
	void RenderPinky() const;
	void RenderMiku() const;
	void LoadMikuModel();
	void RenderSphereDome() const;
	void SpawnAtomizerStreaks();
};