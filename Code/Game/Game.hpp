#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ParticleEditorBaseModule.hpp"
#include "Game/ParticleEditorShapeModule.hpp"
#include "Game/CurveEditor.hpp"

class Entity;
class Prop;
class Player;
class ParticleEmitter;
class ParticleEditor;
class Zoo;

enum class GameMode
{
	ATTRACT,
	EDITOR,
	ZOO,
	COMBO_ZOO,
	CPU_PERF_ZOO,
	GPU_PERF_ZOO,
};

class Game 
{
public:
	Game() {};
	void Startup();
	void Update(float deltaSeconds);
	void Render();
	void ShutDown();
	Camera& GetWorldCamera();
	static bool ControlsCommand(EventArgs& args);
	
	bool IsPlayerInputDisabled() const;
	void SetActiveInputField();

private:
	Vertex_PCU m_attractScreenDrawVertices[3];
	unsigned char m_attractModeTriangleAlpha = 0;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Stopwatch m_stopwatch;
	float m_attractTriangleMinAlpha = 50.f;
	float m_attractTriangleMaxAlpha = 255.f;
	std::vector<Entity*> m_allEntities;
	Prop* m_cube = nullptr;
	Player* m_player = nullptr;
	ParticleSystem* m_particleSystemBeingEdited = nullptr;
	Vec2 m_uiScreenSize = Vec2(500.f, 300.f);
	bool m_remakeGame = false;
	bool m_restrictMouse = true;
	bool m_showImguiDemoWindow = false;
	bool m_threadedParticles = false;
	bool m_gpuParticles = false;
	int m_particleSystemIndex = 0;
	bool m_animate = false;
	bool m_playerInputDisabled = false;
	GameMode m_currentGamemode = GameMode::ATTRACT;
	GameMode m_nextGamemode = GameMode::ATTRACT;
	Zoo* m_zoo = nullptr;
	Prop* sphere = nullptr;
	bool m_anyInputFieldActive = false;

	//imgui emitter window propety value holders
	ParticleEditor* m_editorWindow = nullptr;

	//create or load effect window vars
	std::string m_effectPath = "Data/ParticleSystemData/Test.xml";
	char m_newEffectName[100] = { '\0' };
	EulerAngles m_cubeOrientation;
	
private:
	void HandleMouseCursor();
	void HandleAttractModeInput();
	void HandleGameInput();
	void HandleDebugInput();
	void InitializeAttractScreenDrawVertices();
	void UpdateAttractMode(float deltaSeconds);
	void UpdateEntities(float deltaSeconds);
	void UpdateMode(float deltaSeconds);
	void RenderAttractScreen() const;
	void RenderEntities() const;
	void RenderMode() const;
	void AddSphereProp();
	void AddGridLines();
	void UpdateImGUIWindows();
	void RenderDebugStats() const;
	void RenderControls() const;
	void CleanUpJobs();
	ParticleSystem* SpawnParticleSystem(const char* filepath, const Vec3& worldPosition, bool withEditorWindow, bool gpuParticles, bool defaultSystem = false);
	void ChangeMode();
	void SetupEditorMode();
	void SetupZooMode();
	void QuickSort(int low, int high);
	int PartitionArray(int low, int high);
	void UpdateSphere(float deltaSeconds);
	void CreateOrLoadParticleEffectWindow();
	void DisablePlayerInputIfRequired();
	void RenderSkybox() const;
};