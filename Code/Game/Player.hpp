#pragma once
#include "Game/Entity.hpp"

class Player : public Entity
{
public:
	Player() = default;
	Player(Game* game, Vec3 spawnPosition);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

private:
	void HandleMouseInput();
	void HandleKeyboardInput();
	void HandleControllerInput();

private:
	Vec3 moveDirection;
	bool m_isSprinting = false;
};