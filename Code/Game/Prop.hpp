#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include <vector>

class Texture;

class Prop : public Entity
{
public:
	Prop() = default;
	Prop(Game* game, Vec3 spawnPosition);
	Prop(Game* game, std::vector<Vertex_PCU>& vertices, const char* imageFilepath);
	virtual void Update(float deltaSeonds) override;
	virtual void Render() const override;
	void SetTintColor(const Rgba8& color);

private:
	std::vector<Vertex_PCU> m_localVertices;
	Texture* m_texture = nullptr;
	Rgba8 m_tintColor;
};