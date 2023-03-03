#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Prop.hpp"

extern Renderer* g_theRenderer;

Prop::Prop(Game* game, Vec3 spawnPosition)
	:Entity(game, spawnPosition)
{
}

Prop::Prop(Game* game, std::vector<Vertex_PCU>& vertices, const char* imageFilepath)
	:Entity(game), m_localVertices(vertices)
{
	if (imageFilepath)
	{
		m_texture = g_theRenderer->CreateOrGetTextureFromFile(imageFilepath);
	}
}

void Prop::Update(float deltaSeonds)
{
	m_orientation3D += (m_angularVelocity3D * deltaSeonds);
}

void Prop::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetModelMatrix(GetModelMatrix());
	g_theRenderer->SetModelColor(m_tintColor);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray((int)m_localVertices.size(), m_localVertices.data());
}

void Prop::SetTintColor(const Rgba8& color)
{
	m_tintColor = color;
}

