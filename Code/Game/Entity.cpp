#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity(Game* game, Vec3 spawnPosition)
	:m_game(game), m_position(spawnPosition)
{
}

Entity::Entity(Game* game)
	:m_game(game)
{
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation3D.GetAsMatrix_XFwd_YLeft_ZUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

Vec3 Entity::GetForwardVector() const
{
	//return Mat44::CreateZRotationDegrees(m_orientation3D.m_yawDegrees).GetIBasis3D().GetNormalized();
	return GetModelMatrix().GetIBasis3D().GetNormalized();
}

Vec3 Entity::GetLeftVector() const
{
	return GetModelMatrix().GetJBasis3D().GetNormalized();
}


