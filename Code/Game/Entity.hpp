#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Game;
class Entity
{
public:
	Entity() {};
	Entity(Game* game);
	Entity(Game* game, Vec3 spawnPosition);
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	Mat44 GetModelMatrix() const;
	Vec3 GetForwardVector() const;
	Vec3 GetLeftVector() const;
	virtual ~Entity() {};

public:
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation3D;
	EulerAngles m_angularVelocity3D;
	//float m_orientationDegrees = 0.f;
	//float m_angularVelocity = 0.f;
	/*float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;*/
	Rgba8 m_color;
	Game* m_game = nullptr;

};
