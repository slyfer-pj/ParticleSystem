#include "Engine/Renderer/ParticlesManager.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ParticleSystem.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Zoo.hpp"
#include "Game/Game.hpp"

extern Renderer* g_theRenderer;
extern ParticlesManager* g_theParticlesManager;
extern AudioSystem* g_theAudio;

const Vec3 PINKY_POS = Vec3(-10.f, 5.f, 1.5f);
const Vec3 MATERIALIZE_PARTICLE_POS = Vec3(-10.f, 15.f, -0.1f);
const Vec3 ATOMIZER_BASE_POS = Vec3(-20.f, 0.f, 13.f);

Zoo::Zoo(Game* game, GameMode zooMode)
	:m_game(game), m_zooMode(zooMode)
{
	SpawnZooParticles();
	LoadMikuModel();
}

Zoo::~Zoo()
{
	g_theParticlesManager->KillAllParticleSystems();
	m_systems.clear();
	delete m_miku;
	m_miku = nullptr;
}

void Zoo::Update(float deltaSeconds)
{
	if (m_zooMode == GameMode::ZOO)
	{
		UpdateParticlePos(deltaSeconds);
	}
	else if (m_zooMode == GameMode::COMBO_ZOO)
	{
		UpdateAtomizer(deltaSeconds);
	}
	g_theParticlesManager->UpdateParticleSystems(deltaSeconds, m_game->GetWorldCamera());
}

void Zoo::Render() const
{
	if (m_zooMode == GameMode::ZOO)
	{
		RenderPinky();
		RenderMiku();
	}
	else if (m_zooMode == GameMode::COMBO_ZOO)
	{
		RenderSphereDome();
	}
	g_theParticlesManager->RenderParticleSystems(m_game->GetWorldCamera());
}

void Zoo::ChangeParticleSystemType()
{
	std::vector<ParticleSystem*> copyOfSystems = m_systems;
	m_systems.clear();
	for (int i = 0; i < copyOfSystems.size(); i++)
	{
		ParticleSystem* changedSystem = g_theParticlesManager->ChangeParticleSystemType(copyOfSystems[i]);
		m_systems.push_back(changedSystem);
	}
}

void Zoo::UpdateParticlePos(float deltaSeconds)
{
	constexpr float radius = 2.f;
	constexpr float duration = 15.f;
	constexpr float speed = 100.f;
	static float timer = 0.f;
	timer += deltaSeconds;
	m_materializeParticle_CurrAngle += speed * deltaSeconds;
	m_materializeParticle_CurrHeight += deltaSeconds * 0.5f;
	Vec3 radialOffset_Hearts = Vec3(radius * CosDegrees(m_materializeParticle_CurrAngle), radius * SinDegrees(m_materializeParticle_CurrAngle), m_materializeParticle_CurrHeight);
	Vec3 radialOffset_Stars = Vec3(radius * CosDegrees(-m_materializeParticle_CurrAngle), radius * SinDegrees(-m_materializeParticle_CurrAngle), m_materializeParticle_CurrHeight);
	//Vec3 radialOffset = m_materializeParticle->GetPosition() + Vec3(1.f, 1.f, 1.f) * deltaSeconds * speed;
	m_materializeParticle_Hearts->SetPosition(MATERIALIZE_PARTICLE_POS + radialOffset_Hearts);
	m_materializeParticle_Stars->SetPosition(MATERIALIZE_PARTICLE_POS + radialOffset_Stars);
	if (timer > duration)
	{
		timer = 0.f;
		m_materializeParticle_CurrAngle = 0.f;
		m_materializeParticle_CurrHeight = 0.f;
		m_materializeParticle_Hearts->SetPosition(MATERIALIZE_PARTICLE_POS);
	}
}

void Zoo::UpdateAtomizer(float deltaSeconds)
{
	constexpr float duration = 5.f;
	constexpr float domeAnimDuration = 1.6f;
	constexpr float domeAlphaDuration = 4.f;
	m_atomizerTimer += deltaSeconds;
	if (m_atomizerTimer > duration)
	{
		m_atomizerTimer = 0.f;

		//clear old streaks
		for (int i = 0; i < m_atomizerStreaks.size(); i++)
		{
			g_theParticlesManager->KillParticleSystem(m_atomizerStreaks[i].m_system);
		}
		m_atomizerStreaks.clear();

		//spawn new streaks
		SpawnAtomizerStreaks();
	}
	for (int i = 0; i < m_atomizerStreaks.size(); i++)
	{
		Vec3 currPos = m_atomizerStreaks[i].m_system->GetPosition();
		m_atomizerStreaks[i].m_system->SetPosition(currPos + (m_atomizerStreaks[i].m_moveDirection * m_atomizerStreaks[i].m_speed * deltaSeconds));
	}

	m_domeYaw += 10.f * deltaSeconds;
	m_domeRadius = Interpolate(0.f, 12.f, SmoothStop3(Clamp((m_atomizerTimer / domeAnimDuration), 0.f, 1.f)));
	m_domeAlpha = static_cast<unsigned char>(Interpolate(200, 0, Clamp(m_atomizerTimer / domeAlphaDuration, 0.f, 1.f)));
}

void Zoo::SpawnZooParticles()
{
	if (m_zooMode == GameMode::ZOO)
	{
		ParticleSystem* campfire = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Campfire.xml", Vec3(-10.f, -15.f, 2.f), false);
		m_systems.push_back(campfire);
		ParticleSystem* rain = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Rain.xml", Vec3(-10.f, -5.f, 12.f), false);
		m_systems.push_back(rain);
		ParticleSystem* flame = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Flame.xml", PINKY_POS, false);
		m_systems.push_back(flame);
		m_materializeParticle_Hearts = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Hearts.xml", MATERIALIZE_PARTICLE_POS, false);
		m_systems.push_back(m_materializeParticle_Hearts);
		m_materializeParticle_Stars = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Stars.xml", MATERIALIZE_PARTICLE_POS, false);
		m_systems.push_back(m_materializeParticle_Stars);
		ParticleSystem* starfield = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Starfield.xml", Vec3::ZERO, false);
		m_systems.push_back(starfield);
		ParticleSystem* blackhole = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Blackhole.xml", Vec3(5.f, 0.f, 3.f), false);
		m_systems.push_back(blackhole);
	}
	else if (m_zooMode == GameMode::CPU_PERF_ZOO)
	{
		Vec3 startPos = Vec3(-20.f, -10.f, 0.f);
		for (int i = 0; i < 5; i++)
		{
			Vec3 spawnPos = startPos + Vec3(i * 10.f, 0.f, 0.f);
			ParticleSystem* blueParticles = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/TestBlue.xml", spawnPos, false);
			m_systems.push_back(blueParticles);
		}

		startPos = Vec3(-20.f, 10.f, 0.f);
		for (int i = 0; i < 5; i++)
		{
			Vec3 spawnPos = startPos + Vec3(i * 10.f, 0.f, 0.f);
			ParticleSystem* blueParticles = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/TestBlue.xml", spawnPos, false);
			m_systems.push_back(blueParticles);
		}
	}
	else if (m_zooMode == GameMode::GPU_PERF_ZOO)
	{
		ParticleSystem* gpuParticle = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Tricolor.xml", Vec3(0.f, 0.f, 30.f), true);
		m_systems.push_back(gpuParticle);
	}
	else if (m_zooMode == GameMode::COMBO_ZOO)
	{
		ParticleSystem* atomizer = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/Atomizer.xml", ATOMIZER_BASE_POS, true);
		m_systems.push_back(atomizer);
		SpawnAtomizerStreaks();
	}
}

void Zoo::RenderPinky() const
{
	Camera worldCam = m_game->GetWorldCamera();
	Vec3 pinkyPos = PINKY_POS;
	Vec2 pinkySize = Vec2(2.f, 3.f);
	//Vec3 forward = (worldCam.GetPosition() - pinkyPos).GetNormalized();
	Vec3 forward = Vec3(1.f, 0.f, 0.f);
	Vec3 up = Vec3(0.f, 0.f, 1.f);	//world up
	Vec3 right = CrossProduct3D(up, forward);
	
	Vec3 topLeft = pinkyPos + up * pinkySize.y * 0.5f + (-right) * pinkySize.x * 0.5f;
	Vec3 botLeft = pinkyPos + (-up) * pinkySize.y * 0.5f + (-right) * pinkySize.x * 0.5f;
	Vec3 botRight = pinkyPos + (-up) * pinkySize.y * 0.5f + right * pinkySize.x * 0.5f;
	Vec3 topRight = pinkyPos + up * pinkySize.y * 0.5f + right * pinkySize.x * 0.5f;

	std::vector<Vertex_PCU> verts;
	AddVertsForQuad3D(verts, topLeft, botLeft, botRight, topRight);
	g_theRenderer->BindShaderByName("Default");
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetModelMatrix(Mat44::IDENTITY);
	g_theRenderer->SetModelColor(Rgba8::WHITE);
	Texture* pinkyTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Pinky.png");
	g_theRenderer->BindTexture(pinkyTex);
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}

void Zoo::RenderMiku() const
{
	Shader* litShader = g_theRenderer->CreateOrGetShader("Data/Shaders/SpriteLit");
	g_theRenderer->BindShader(litShader);
	g_theRenderer->BindVertexBuffer(m_miku->GetVertexBuffer());
	g_theRenderer->BindIndexBuffer(m_miku->GetIndexBuffer());
	g_theRenderer->SetModelMatrix(Mat44::IDENTITY);
	g_theRenderer->SetModelColor(Rgba8::WHITE);
	//g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	Texture* mikuTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Models/miku/miku_base.png");
	g_theRenderer->BindTexture(mikuTex);
	g_theRenderer->DrawIndexed(m_miku->GetElementCount());
}

void Zoo::LoadMikuModel()
{
	mesh_import_options options;
	Mat44 transform = Mat44::CreateTranslation3D(MATERIALIZE_PARTICLE_POS);
	Mat44 ori;
	ori.SetFromText("j,k,i");
	transform.Append(ori);
	//transform.Append(Mat44::CreateTranslation3D(MATERIALIZE_PARTICLE_POS));
	options.m_transform = transform;
	MeshBuilder builder;
	builder.ImportFromOBJFile("Data/Models/miku/miku.obj", options);
	m_miku = new Mesh(g_theRenderer);
	m_miku->UpdateFromBuilder(builder);
}

void Zoo::RenderSphereDome() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForSphere(verts, 16, 16, m_domeRadius);
	Mat44 transformMatrix = Mat44::CreateTranslation3D(ATOMIZER_BASE_POS);
	transformMatrix.Append(Mat44::CreateZRotationDegrees(m_domeYaw));
	TransformVertexArray3D(int(verts.size()), verts.data(), transformMatrix);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetModelColor(Rgba8(0, 225, 255, m_domeAlpha));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
}

void Zoo::SpawnAtomizerStreaks()
{
	RandomNumberGenerator rng;
	constexpr int NUM_STREAKS = 50;
	m_atomizerStreaks.reserve(NUM_STREAKS);
	for (int i = 0; i < NUM_STREAKS; i++)
	{
		Vec3 randDirInSphere = rng.GetRandomDirectionInSphere();
		Vec3 spawnPos = ATOMIZER_BASE_POS + ((randDirInSphere) * 10.f);
		ParticleSystem* atomizerStreak = g_theParticlesManager->CreateParticleSystem("Data/ParticleSystemData/AtomizerStreak.xml", spawnPos, false);
		m_systems.push_back(atomizerStreak);
		std::vector<ParticleEmitterData> emitterData = atomizerStreak->GetEmitterDataForAllEmitters();
		ConeEmitter* coneEmitter = dynamic_cast<ConeEmitter*>(emitterData[0].m_shape);
		coneEmitter->m_coneForward = randDirInSphere;
		atomizerStreak->UpdateEmitterData(emitterData);

		AtomizerStreakData data;
		data.m_system = atomizerStreak;
		data.m_moveDirection = coneEmitter->m_coneForward;
		data.m_speed = rng.GetRandomFloatInRange(5.f, 8.f);
		m_atomizerStreaks.push_back(data);
	}

	//play sound
	SoundID atomizerSound = g_theAudio->CreateOrGetSound("Data/Audio/Atomizer.wav");
	g_theAudio->StartSound(atomizerSound);
}
