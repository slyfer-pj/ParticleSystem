#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"

class AttractScreen;
class Game;
class App
{
public:
	App() {}
	~App() {}
	void Startup();
	void Run();
	void Shutdown();
	void HandleQuitRequest();
	void RemakeGame();

	bool IsQuitting() const { return s_isQuitting; }
	Clock& GetGameClock() { return m_gameClock; }

	static bool Command_Quit(EventArgs& args);

private:
	static bool s_isQuitting;
	Game* m_theGame = nullptr;
	Clock m_gameClock;

private:
	void BeginFrame();
	void RunFrame();
	void Update();
	void Render() const;
	void EndFrame();
	void HandleKeyboardInput();
};