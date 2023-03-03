#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"

extern InputSystem* g_theInput;

constexpr float moveSpeed = 5.f;
constexpr float mouseDeltaScale = 0.1f;
constexpr float sprintMultiplier = 2.f;
constexpr float joystickSensitivity = 2.f;
constexpr float rollSpeed = 1.5f;

Player::Player(Game* game, Vec3 spawnPosition)
	:Entity(game, spawnPosition)
{
}

void Player::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	moveDirection = Vec3();
	HandleMouseInput();
	HandleKeyboardInput();
	HandleControllerInput();

	float currentMoveSpeed = moveSpeed;
	if (m_isSprinting)
		currentMoveSpeed *= sprintMultiplier;
	//DebuggerPrintf("forward = %f %f %f \n", forwardVector.x, forwardVector.y, forwardVector.z);
	m_position += moveDirection * currentMoveSpeed * deltaSeconds;
	m_orientation3D.m_pitchDegrees = Clamp(m_orientation3D.m_pitchDegrees , -85.f, 85.f);
	m_orientation3D.m_rollDegrees = Clamp(m_orientation3D.m_rollDegrees , -45.f, 45.f);
	Camera& worldCam = m_game->GetWorldCamera();
	worldCam.SetTransform(m_position, m_orientation3D);

	m_isSprinting = false;
}

void Player::Render() const
{

}

void Player::HandleMouseInput()
{
	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	//DebuggerPrintf("Mouse delta = %f %f \n", mouseDelta.x, mouseDelta.y);
	m_orientation3D.m_yawDegrees += (mouseDelta.x * mouseDeltaScale);
	m_orientation3D.m_pitchDegrees -= (mouseDelta.y * mouseDeltaScale);
}

void Player::HandleKeyboardInput()
{
	if (m_game->IsPlayerInputDisabled())
		return;

	Vec3 forwardNormal = GetForwardVector();
	Vec3 leftNormal = GetLeftVector();
	if (g_theInput->IsKeyDown('W'))
		moveDirection += forwardNormal;
	if (g_theInput->IsKeyDown('S'))
		moveDirection -= forwardNormal;
	if (g_theInput->IsKeyDown('A'))
		moveDirection += leftNormal;
	if (g_theInput->IsKeyDown('D'))
		moveDirection -= leftNormal;
	if (g_theInput->IsKeyDown('Q'))
		moveDirection += Vec3(0.f, 0.f, 1.f);
	if (g_theInput->IsKeyDown('E'))
		moveDirection += Vec3(0.f, 0.f, -1.f);
	if (g_theInput->IsKeyDown('H'))
	{
		m_position = Vec3();
		m_orientation3D = EulerAngles();
	}
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		m_isSprinting = true;
}

void Player::HandleControllerInput()
{
	Vec3 forwardNormal = GetForwardVector();
	Vec3 leftNormal = GetLeftVector();
	const XboxController& controller = g_theInput->GetController(0);
	if (controller.GetLeftJoystick().GetMagnitude() > 0.f)
	{
		float stickOrientation = controller.GetLeftJoystick().GetOrientationDegrees();
		Vec2 stickDirection = Vec2::MakeFromPolarDegrees(stickOrientation);
		if (stickDirection.y > 0.2f)
			moveDirection += forwardNormal;
		if (stickDirection.y < -0.2f)
			moveDirection -= forwardNormal;
		if (stickDirection.x > 0.2f)
			moveDirection -= leftNormal;
		if (stickDirection.x < -0.2f)
			moveDirection += leftNormal;
	}

	if (controller.GetRightJoystick().GetMagnitude() > 0.f)
	{
		float stickOrientation = controller.GetRightJoystick().GetOrientationDegrees();
		Vec2 stickDirection = Vec2::MakeFromPolarDegrees(stickOrientation);
		m_orientation3D.m_yawDegrees -= stickDirection.x * joystickSensitivity;
		m_orientation3D.m_pitchDegrees -= stickDirection.y * joystickSensitivity;
	}

	if(controller.IsButtonDown(XBOX_BUTTON_LEFT_SHOULDER))
		moveDirection += Vec3(0.f, 0.f, 1.f);
	if (controller.IsButtonDown(XBOX_BUTTON_RIGHT_SHOULDER))
		moveDirection += Vec3(0.f, 0.f, -1.f);
	if (controller.GetLeftTrigger() > 0.2f)
		m_orientation3D.m_rollDegrees += (-rollSpeed);
	if (controller.GetRightTrigger() > 0.2f)
		m_orientation3D.m_rollDegrees += rollSpeed;
	if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		m_position = Vec3();
		m_orientation3D = EulerAngles();
	}
		
	if (controller.IsButtonDown(XBOX_BUTTON_A))
		m_isSprinting = true;
}

