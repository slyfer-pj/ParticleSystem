#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/Job.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/ParticlesManager.hpp"
#include "Engine/Renderer/ParticleSystem.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/EmitterWindow.hpp"
#include "Game/ParticleEditor.hpp"
#include "Game/Zoo.hpp"

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern JobSystem* g_theJobSystem;
extern ParticlesManager* g_theParticlesManager;

static float animationTimer = 0.f;

constexpr float worldCamFov = 60.f;
constexpr float worldCamNearZ = 0.1f;
constexpr float worldCamFarZ = 100.f;
constexpr int numberOfGridLinesPerAxis = 50;

const char* PARTICLE_EFFECT_DATA_DIRECTORY = "Data/ParticleSystemData/";
const char* TEST_PARTICLE_EFFECT_PATH = "Data/ParticleSystemData/Test.xml";

void Game::Startup()
{
	m_currentGamemode = GameMode::ATTRACT;
	InitializeAttractScreenDrawVertices();
	m_uiScreenSize.x = g_gameConfigBlackboard.GetValue("screenSizeWidth", m_uiScreenSize.x);
	m_uiScreenSize.y = g_gameConfigBlackboard.GetValue("screenSizeHeight", m_uiScreenSize.y);
	m_worldCamera.SetViewToRenderTransform(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
	m_stopwatch.Start(&g_theApp->GetGameClock(), 1.f);
	SubscribeEventCallbackFunction("controls", ControlsCommand);
	DebugAddWorldBasis(Mat44(), -1.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);

	m_player = new Player(this, Vec3());
	m_player->m_position = Vec3(-10.f, -10.f, 10.f);
	m_allEntities.push_back(m_player);

	//AddSphereProp();
	AddGridLines();

	Mat44 transMat = (Mat44::CreateTranslation3D(Vec3(3.f, 0.f, 1.f)));
	transMat.Append(m_worldCamera.GetViewToRenderMatrix());
	DebugAddWorldBasis(transMat, -1, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);
}

void Game::ShutDown()
{
	//clean up any running jobs in the job system
	CleanUpJobs();

	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i])
		{
			delete m_allEntities[i];
			m_allEntities[i] = nullptr;
		}
	}
	m_allEntities.clear();

	g_theParticlesManager->KillAllParticleSystems();
	m_particleSystemBeingEdited = nullptr;
	delete m_zoo;
	m_zoo = nullptr;

	m_cube = nullptr;
	m_player = nullptr;
}

void Game::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_remakeGame)
	{
		g_theApp->RemakeGame();
		return;
	}

	m_anyInputFieldActive = false;
	HandleMouseCursor();
	ChangeMode();
	if (m_currentGamemode == GameMode::ATTRACT)
	{
		m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(m_uiScreenSize.x, m_uiScreenSize.y));
		HandleAttractModeInput();
		UpdateAttractMode(deltaSeconds);
	}
	else
	{
		m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, worldCamFov, worldCamNearZ, worldCamFarZ);
		HandleDebugInput();
		HandleGameInput();
		//UpdateSphere(deltaSeconds);
		UpdateEntities(deltaSeconds);
		UpdateMode(deltaSeconds);
	}

	DisablePlayerInputIfRequired();
}

void Game::Render() 
{
	if (m_currentGamemode == GameMode::ATTRACT)
	{
		g_theRenderer->BeginCamera(m_screenCamera);
		{
			g_theRenderer->ClearScreen(Rgba8::GREY);
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);
			g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
			g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
			RenderAttractScreen();
		}
		g_theRenderer->EndCamera(m_screenCamera);
	}
	else
	{
		g_theRenderer->BeginCamera(m_worldCamera);
		{
			g_theRenderer->ClearScreen(Rgba8::GREY);
			RenderSkybox();
			g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
			g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
			RenderEntities();
			DebugRenderWorld(m_worldCamera);
			RenderMode();
		}
		g_theRenderer->EndCamera(m_worldCamera);

		g_theRenderer->BeginCamera(m_screenCamera);
		{
			RenderDebugStats();
			RenderControls();
			DebugRenderScreen(m_screenCamera);
		}
		g_theRenderer->EndCamera(m_screenCamera);
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	{
		g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
		g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		g_theConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()));
	}
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::UpdateImGUIWindows()
{
	if (m_showImguiDemoWindow)
		ImGui::ShowDemoWindow(&m_showImguiDemoWindow);

	if (m_currentGamemode == GameMode::EDITOR)
	{
		if (m_editorWindow)
		{
			m_editorWindow->UpdateWindow();
		}

		CreateOrLoadParticleEffectWindow();
	}
}

void Game::RenderDebugStats() const
{
	/*if (!m_particleSystemBeingEdited)
		return;*/
	std::string modeString;
	switch (m_currentGamemode)
	{
	case GameMode::ATTRACT: modeString = "Attract"; break;
	case GameMode::EDITOR: modeString = "Editor"; break;
	case GameMode::ZOO: modeString = "Zoo"; break;
	case GameMode::COMBO_ZOO: modeString = "Combo Zoo"; break;
	case GameMode::CPU_PERF_ZOO: modeString = "CPU Perf Zoo"; break;
	case GameMode::GPU_PERF_ZOO: modeString = "GPU Perf Zoo"; break;
	}

	ParticlesDebugData debugData = g_theParticlesManager->GetDebugData();
	std::string debugString;
	debugString.append(Stringf("Mode = %s\n", modeString.c_str()));
	debugString.append(Stringf("Max Particles = %d\n", debugData.m_maxParticles));
	debugString.append(Stringf("Num CPU Pools = %d\n", debugData.m_numPools));
	debugString.append(Stringf("Max Particles Per Pool = %d\n", debugData.m_particlesPerPool));
	debugString.append(Stringf("Total Alive Particles = %d\n", debugData.m_aliveParticles));
	debugString.append(Stringf("Num CPU systems = %d\n", debugData.m_numCPUsystems));
	debugString.append(Stringf("Num GPU systems = %d\n", debugData.m_numGPUsystems));
	Clock& gameClock = g_theApp->GetGameClock();
	debugString.append(Stringf("dt = %.2f ms, fps = %.2f", gameClock.GetDeltaTime() * 1000.f, 1 / gameClock.GetDeltaTime()));

	std::vector<Vertex_PCU> textVerts;
	Vec2 textBoxDimensions = Vec2(400.f, 500.f);	 //magic numbers
	AABB2 textBoxBounds = AABB2(Vec2(m_uiScreenSize.x - textBoxDimensions.x, 0.f), Vec2(m_uiScreenSize.x, textBoxDimensions.y));	//bottom right
	Vec2 textAlignment = Vec2(1.f, 0.f);
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/MyFixedFont");
	font->AddVertsForTextInBox2D(textVerts, textBoxBounds, 20.f, debugString, Rgba8::WHITE, 0.8f, textAlignment);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::RenderControls() const
{
	std::string controlsString = " F1 - toggle mouse, F3 - toggle cpu/gpu particles, Left/Right Arrow - Prev/Next Mode ";

	std::vector<Vertex_PCU> textVerts;
	Vec2 textBoxDimensions = Vec2(m_uiScreenSize.x, 200.f);	 //magic numbers
	Vec2 mins = Vec2(0.f, m_uiScreenSize.y - textBoxDimensions.y);
	AABB2 textBoxBounds = AABB2(mins, mins + textBoxDimensions);	//bottom right
	Vec2 textAlignment = Vec2(0.f, 1.f);
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/MyFixedFont");
	font->AddVertsForTextInBox2D(textVerts, textBoxBounds, 20.f, controlsString, Rgba8::WHITE, 0.8f, textAlignment);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::CleanUpJobs()
{
	if (g_theJobSystem)
	{
		g_theJobSystem->CancelAllJobs();

		//block until all jobs are finished
		/*while (g_theJobSystem->GetNumQueuedJobs() > 0 && g_theJobSystem->GetNumExecutingJobs() > 0)
			;*/

		//retrieve and delete all finished jobs
		/*Job* finishedJob = g_theJobSystem->RetrieveFinishedJob();
		while (finishedJob)
		{
			delete finishedJob;
			finishedJob = g_theJobSystem->RetrieveFinishedJob();
		}*/
	}
}

ParticleSystem* Game::SpawnParticleSystem(const char* filepath, const Vec3& worldPosition, bool withEditorWindow, bool gpuParticles, bool defaultSystem)
{
	ParticleSystem* spawnedSystem = g_theParticlesManager->CreateParticleSystem(filepath, worldPosition, gpuParticles, defaultSystem);

	if (withEditorWindow)
	{
		if (m_editorWindow)
			delete m_editorWindow;
 		m_editorWindow = new ParticleEditor(this, spawnedSystem->GetEmitterDataForAllEmitters(), filepath);
	}

	bool gpuDebug = g_gameConfigBlackboard.GetValue("gpuParticlesStepUpdate", false);
	if (gpuDebug)
		spawnedSystem->ToggleDebugMode();

	return spawnedSystem;
}

void Game::ChangeMode()
{
	if (m_nextGamemode != m_currentGamemode)
	{
		//clean up current mode
		switch (m_currentGamemode)
		{
		case GameMode::EDITOR:
		{
			g_theParticlesManager->KillParticleSystem(m_particleSystemBeingEdited);
			m_particleSystemBeingEdited = nullptr;
			break;
		}
		case GameMode::COMBO_ZOO:
		case GameMode::CPU_PERF_ZOO:
		case GameMode::GPU_PERF_ZOO:
		case GameMode::ZOO:
		{
			delete m_zoo;
			m_zoo = nullptr;
			break;
		}
		default: break;
		}

		//setup next mode
		switch (m_nextGamemode)
		{
		case GameMode::EDITOR:
		{
			SetupEditorMode();
			break;
		}
		case GameMode::COMBO_ZOO:
		case GameMode::CPU_PERF_ZOO:
		case GameMode::GPU_PERF_ZOO:
		case GameMode::ZOO:
		{
			SetupZooMode();
			break;
		}

		}
	}
}

void Game::SetupEditorMode()
{
	m_currentGamemode = GameMode::EDITOR;
	m_particleSystemBeingEdited = SpawnParticleSystem(m_effectPath.c_str(), Vec3(0.f, 0.f, 0.f), true, m_gpuParticles);
	//m_particleSystemBeingEdited->ToggleDebugMode();
}

void Game::SetupZooMode()
{
	m_currentGamemode = m_nextGamemode;
	m_zoo = new Zoo(this, m_currentGamemode);
	m_player->m_position = Vec3(15.f, 0.f, 5.f);
	m_player->m_orientation3D = EulerAngles(180.f, 30.f, 0.f);
}

int arrayToSort[20] = { 5, 7, 12, 1, 9, 15, 28, 18, 23, 6, 11, 45, 18, 4, 33, 17, 21, 11, 20, 19 };
//int arrayToSort[5] = { 5, 4, 3, 2, 1 };

void Game::QuickSort(int low, int high)
{
	if (low < high)
	{
		int pivot = PartitionArray(low, high);
		QuickSort(low, pivot - 1);
		QuickSort(pivot + 1, high);
	}
}

int Game::PartitionArray(int low, int high)
{
	int smallestElementIndex = low;
	int pivotElementIndex = high;

	for (int i = low; i <= high - 1; i++)
	{
		if (arrayToSort[i] < arrayToSort[pivotElementIndex])
		{
			int temp = arrayToSort[smallestElementIndex];
			arrayToSort[smallestElementIndex] = arrayToSort[i];
			arrayToSort[i] = temp;

			smallestElementIndex++;
		}
	}

	int temp = arrayToSort[pivotElementIndex];
	arrayToSort[pivotElementIndex] = arrayToSort[smallestElementIndex];
	arrayToSort[smallestElementIndex] = temp;

	return smallestElementIndex;
}

void Game::UpdateSphere(float deltaSeconds)
{
	static float angle = 0.f;
	constexpr float radius = 5.f;
	angle += 10.f * deltaSeconds;
	sphere->m_position = Vec3(radius * CosDegrees(angle), radius * SinDegrees(angle), 10.f);
}

void Game::CreateOrLoadParticleEffectWindow()
{
	ImGui::Begin("Create or Load Particle Effect");
	{
		ImGui::Text("Current - \"%s\"", m_effectPath.c_str());
		if (ImGui::Button("Load effect"))
		{
			m_effectPath = g_theWindow->GetFileNameFromFileExploreDialogueBox(PARTICLE_EFFECT_DATA_DIRECTORY);
			if (m_effectPath == "")
				m_effectPath = TEST_PARTICLE_EFFECT_PATH;

			g_theParticlesManager->KillParticleSystem(m_particleSystemBeingEdited);
			m_particleSystemBeingEdited = SpawnParticleSystem(m_effectPath.c_str(), Vec3::ZERO, true, m_gpuParticles);
		}
		ImGui::InputText("New effect name", m_newEffectName, sizeof(m_newEffectName));
		if (ImGui::IsItemActive())
		{
			SetActiveInputField();
		}
		std::string newEffectPath = PARTICLE_EFFECT_DATA_DIRECTORY + std::string(m_newEffectName) + ".xml";
		bool duplicateFile = DoesFileExist(newEffectPath);
		ImGui::BeginDisabled(duplicateFile);
		{
			if (ImGui::Button("Create New Effect"))
			{
				g_theParticlesManager->KillParticleSystem(m_particleSystemBeingEdited);
				m_particleSystemBeingEdited = SpawnParticleSystem(newEffectPath.c_str(), Vec3::ZERO, true, m_gpuParticles, true);
				memset(m_newEffectName, '\0', sizeof(m_newEffectName));
				m_effectPath = newEffectPath;
			}
		}
		ImGui::EndDisabled();

		ImGui::InputFloat3("ori", &m_cubeOrientation.m_yawDegrees);

		if (duplicateFile)
		{
			ImGui::Text("File already exists, load it instead or try a new name");
		}

	}
	ImGui::End();
}

void Game::SetActiveInputField()
{
	m_anyInputFieldActive = true;
}

void Game::HandleMouseCursor()
{
	bool gameWindowFocused = g_theWindow->HasFocus();
	if (gameWindowFocused && m_restrictMouse)
	{
		if (g_theConsole->GetMode() == DevConsoleMode::OPEN_FULL)
		{
			g_theInput->SetMouseMode(false, false, false);
		}
		else
		{
			if (m_currentGamemode == GameMode::ATTRACT)
			{
				g_theInput->SetMouseMode(false, false, false);
			}
			else
			{
				g_theInput->SetMouseMode(true, true, true);
			}
		}
	}
}

void Game::HandleAttractModeInput()
{
	const XboxController& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		g_theApp->HandleQuitRequest();
	if (g_theInput->WasKeyJustPressed(' ') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
		m_nextGamemode = GameMode::EDITOR;
}

void Game::HandleGameInput()
{
	if (IsPlayerInputDisabled())
		return;

	const XboxController& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(' ') || controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		m_currentGamemode = GameMode::ATTRACT;
		m_remakeGame = true;
		m_stopwatch.Restart();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		if (m_restrictMouse)
		{
			g_theInput->SetMouseMode(false, false, false);
			m_restrictMouse = false;
		}
		else
		{
			g_theInput->SetMouseMode(true, true, true);
			m_restrictMouse = true;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_gpuParticles = !m_gpuParticles;

		if (m_currentGamemode == GameMode::EDITOR)
		{
			m_particleSystemBeingEdited = g_theParticlesManager->ChangeParticleSystemType(m_particleSystemBeingEdited);
		}
		else if (m_currentGamemode == GameMode::ZOO)
		{
			m_zoo->ChangeParticleSystemType();
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_showImguiDemoWindow = !m_showImguiDemoWindow;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		int modeIndex = static_cast<int>(m_currentGamemode);
		modeIndex--;
		if (modeIndex < 1)
			modeIndex = 5;	//dont go to attract

		m_nextGamemode = static_cast<GameMode>(modeIndex);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		int modeIndex = static_cast<int>(m_currentGamemode);
		modeIndex++;
		if (modeIndex > 5)
			modeIndex = 1;	//dont go to attract

		m_nextGamemode = static_cast<GameMode>(modeIndex);
	}
}

void Game::HandleDebugInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		if (m_particleSystemBeingEdited)
			m_particleSystemBeingEdited->ToggleDebugMode();
	}
	if (g_theInput->WasKeyJustPressed('C'))
	{
		if (m_particleSystemBeingEdited)
			m_particleSystemBeingEdited->DebugGPUUpdateStepNow();
	}
}

void Game::InitializeAttractScreenDrawVertices()
{
	m_attractScreenDrawVertices[0].m_position = Vec3(-2.f, -2.f, 0.f);
	m_attractScreenDrawVertices[1].m_position = Vec3(0.f, 2.f, 0.f);
	m_attractScreenDrawVertices[2].m_position = Vec3(2.f, -2.f, 0.f);

	m_attractScreenDrawVertices[0].m_color = Rgba8(0, 255, 0, 255);
	m_attractScreenDrawVertices[1].m_color = Rgba8(0, 255, 0, 255);
	m_attractScreenDrawVertices[2].m_color = Rgba8(0, 255, 0, 255);
}

void Game::RenderAttractScreen() const
{
	Vertex_PCU tempCopyOfBlinkingTriangle[3];

	for (int i = 0; i < 3; i++)
	{
		tempCopyOfBlinkingTriangle[i].m_position = m_attractScreenDrawVertices[i].m_position;
		tempCopyOfBlinkingTriangle[i].m_color.r = m_attractScreenDrawVertices[i].m_color.r;
		tempCopyOfBlinkingTriangle[i].m_color.g = m_attractScreenDrawVertices[i].m_color.g;
		tempCopyOfBlinkingTriangle[i].m_color.b = m_attractScreenDrawVertices[i].m_color.b;
		tempCopyOfBlinkingTriangle[i].m_color.a = m_attractModeTriangleAlpha;
	}

	TransformVertexArrayXY3D(3, tempCopyOfBlinkingTriangle, 25.f, 0.f, Vec2(m_uiScreenSize.x * 0.5f, m_uiScreenSize.y * 0.5f));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(3, tempCopyOfBlinkingTriangle);
}

void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (m_stopwatch.CheckDurationElapsedAndDecrement())
	{
		float temp = m_attractTriangleMinAlpha;
		m_attractTriangleMinAlpha = m_attractTriangleMaxAlpha;
		m_attractTriangleMaxAlpha = temp;
	}
	float alpha = Interpolate(m_attractTriangleMinAlpha, m_attractTriangleMaxAlpha, m_stopwatch.GetElapsedFraction());
	m_attractModeTriangleAlpha = (unsigned char)alpha;
}

void Game::UpdateEntities(float deltaSeconds)
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Update(deltaSeconds);
	}
}

void Game::UpdateMode(float deltaSeconds)
{
	switch (m_currentGamemode)
	{
	case GameMode::EDITOR:
	{
		UpdateImGUIWindows();

		if (m_editorWindow->RespawnSystem())
		{
			g_theParticlesManager->KillParticleSystem(m_particleSystemBeingEdited);
			m_particleSystemBeingEdited = SpawnParticleSystem(m_effectPath.c_str(), Vec3(5.f, 5.f, 0.f), true, m_gpuParticles);
		}

		//update particle system with new data from the editor
		if (m_editorWindow->IsEditorDataDirty())
		{
			if (m_particleSystemBeingEdited)
			{
				m_particleSystemBeingEdited->UpdateEmitterData(m_editorWindow->GetLatestEmitterData());
				m_particleSystemBeingEdited->SetPosition(m_editorWindow->m_particleSystemPos);
			}
			m_editorWindow->SetEditorDataDirty(false);
		}

		Vec3 currPos = m_particleSystemBeingEdited->GetPosition();
		m_particleSystemBeingEdited->SetPosition(currPos + Vec3(0.f, -1.f, 0.f) * deltaSeconds * 0.f);
		g_theParticlesManager->UpdateParticleSystems(deltaSeconds, m_worldCamera);
		break;
	}
	case GameMode::COMBO_ZOO:
	case GameMode::CPU_PERF_ZOO:
	case GameMode::GPU_PERF_ZOO:
	case GameMode::ZOO:
	{
		m_zoo->Update(deltaSeconds);
		break;
	}
	case GameMode::ATTRACT:
	default:
		break;
	}
}

void Game::RenderEntities() const
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Render();
	}
}

void Game::RenderMode() const
{
	switch (m_currentGamemode)
	{
	case GameMode::EDITOR:
	{
		g_theParticlesManager->RenderParticleSystems(m_worldCamera);
		break;
	}
	case GameMode::COMBO_ZOO:
	case GameMode::CPU_PERF_ZOO:
	case GameMode::GPU_PERF_ZOO:
	case GameMode::ZOO:
	{
		m_zoo->Render();
	}
	case GameMode::ATTRACT:
	default:
		break;
	}
}

void Game::AddSphereProp()
{
	std::vector<Vertex_PCU> verts;
	verts.reserve(16 * 8 * 6);
	AddVertsForSphere(verts, 16, 8, 0.5f);
	sphere = new Prop(this, verts, "Data/Images/TestUV.png");
	sphere->m_position = Vec3(0.f, 0.f, 0.f);
	sphere->m_angularVelocity3D.m_yawDegrees = 50.f;
	//sphere->m_angularVelocity3D.m_pitchDegrees = 10.f;
	m_allEntities.push_back(sphere);
}

void Game::AddGridLines()
{
	std::vector<Vertex_PCU> verts;
	/*float thicknessOfLine = 0.01f;
	float start = 0.f;
	float length = ((float)numberOfGridLinesPerAxis / 2.f);
	float side = 1.f;
	float additionalThicknessMultiplier = 2.f;
	for (int i = 0; i < numberOfGridLinesPerAxis; i++)
	{
		float additionalThickness = 1.f;
		Rgba8 xAxisColor, yAxisColor;
		if (i != 0)
		{
			xAxisColor = Rgba8::RED;
			yAxisColor = Rgba8::GREEN;
		}
		if (((i + 1) % 10 == 0) || (i % 10 == 0))
			additionalThickness *= additionalThicknessMultiplier;

		side *= -1.f;
		AABB3 xAxisBounds(Vec3(-length, (start - (thicknessOfLine * additionalThickness)) * side, -(thicknessOfLine * additionalThickness)), Vec3(length, (start + (thicknessOfLine * additionalThickness)) * side, (thicknessOfLine * additionalThickness)));
		AddVertsForAABB3D(verts, xAxisBounds, xAxisColor);
		AABB3 yAxisBounds(Vec3((start - (thicknessOfLine * additionalThickness)) * side, -length, -(thicknessOfLine * additionalThickness)), Vec3((start + (thicknessOfLine * additionalThickness)) * side, length, (thicknessOfLine * additionalThickness)));
		AddVertsForAABB3D(verts, yAxisBounds, yAxisColor);

		if (i % 2 == 0)
			start += 1.f;

		additionalThickness = 1.f;
	}*/

	Vec2 dimensions = Vec2(50.f, 50.f);
	Vec2 mins = dimensions * -1.f * 0.5f;
	AABB2 quad = AABB2(mins, mins + dimensions);
	AddVertsForQuad3D(verts, Vec3(quad.m_mins.x, quad.m_maxs.y, 0.f), Vec3(quad.m_mins.x, quad.m_mins.y, 0.f), Vec3(quad.m_maxs.x, quad.m_mins.y, 0.f),
		Vec3(quad.m_maxs.x, quad.m_maxs.y, 0.f), Rgba8(165, 42, 42, 255));

	Prop* gridLines = new Prop(this, verts, nullptr);
	m_allEntities.push_back(gridLines);
}

Camera& Game::GetWorldCamera() 
{
	return m_worldCamera;
}

bool Game::ControlsCommand(EventArgs& args)
{
	UNUSED(args);
	g_theConsole->AddLine(g_theConsole->COMMAND, "--- controls ---");
	g_theConsole->AddLine(g_theConsole->COMMAND, "W/A/S/D/Left Joystick - Move");
	g_theConsole->AddLine(g_theConsole->COMMAND, "Q/E - Up/Down");
	g_theConsole->AddLine(g_theConsole->COMMAND, "Mouse/Right Joystick - Aim");
	g_theConsole->AddLine(g_theConsole->COMMAND, "F1 - Toggle mouse control");
	g_theConsole->AddLine(g_theConsole->COMMAND, "~ - Open dev console");
	g_theConsole->AddLine(g_theConsole->COMMAND, "Escape- Exit");
	g_theConsole->AddLine(g_theConsole->COMMAND, "Space/Start - New game");

	return false;
}

void Game::DisablePlayerInputIfRequired()
{
	if (m_anyInputFieldActive)
		m_playerInputDisabled = true;
	else
		m_playerInputDisabled = false;
}

void Game::RenderSkybox() const
{
	std::vector<Vertex_PCU> cubeVerts;
	AddVertsForAABB3D(cubeVerts, AABB3(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f)));
	Shader* skyboxShader = g_theRenderer->CreateOrGetShader("Data/Shaders/Skybox");
	g_theRenderer->BindShader(skyboxShader);

	Texture* skyboxTexture = g_theRenderer->CreateSkyboxTexture(
		"LightSkybox", 
		"Data/Images/frontlabeled.png",
		"Data/Images/backlabeled.png",
		"Data/Images/leftlabeled.png",
		"Data/Images/rightlabeled.png",
		"Data/Images/toplabeled.png",
		"Data/Images/bottomlabeled.png"
	);

	g_theRenderer->SetModelMatrix(Mat44::CreateTranslation3D(m_worldCamera.GetPosition()));
	g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, false);
	g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->BindTexture(skyboxTexture);
	g_theRenderer->DrawVertexArray((int)cubeVerts.size(), cubeVerts.data());
	g_theRenderer->BindShaderByName("Default");
}

bool Game::IsPlayerInputDisabled() const
{
	return m_playerInputDisabled;
}

