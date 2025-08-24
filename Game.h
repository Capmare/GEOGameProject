#pragma once

#include <vector>
#include <memory>
#include <random>
#include <SDL.h>

#include "utils.h"                    // Window
#include "FlyFish.h"                  // OneBlade, TwoBlade, ThreeBlade, Motor
#include "Gameplay/Maze.h"            // gameplay::Maze
#include "Gameplay/MovablePillar.h"   // gameplay::MovablePillar
#include "Gameplay/PillarRenderer.h"
#include "Gameplay/ReflecPillar.h"   // gameplay::ReflectPillar

class Game
{
public:
    explicit Game(const Window& window);
    ~Game();

    void Run();

    // world editing helpers
    void AddPillar(const ThreeBlade& center);
    void GEOAClone();
    void AddReflector(const ThreeBlade& c, float triggerR, float cooldown = 0.f);

    // PGA wrappers
    Motor MakeTranslator(float dx, float dy) const;
    Motor MakeRotationAboutPillar(const ThreeBlade& C, float angRad) const;
    Motor Reverse(const Motor& m);
    ThreeBlade ApplyMotor(const ThreeBlade& X, const Motor& M);

private:
    // engine
    void InitializeGameEngine();
    void CleanupGameEngine();

    void ProcessKeyDownEvent(const SDL_KeyboardEvent& e);
    void ProcessKeyUpEvent(const SDL_KeyboardEvent& e);

    void Update(float dt);
    void Draw() const;

    void DrawPillars() const;
    void DrawCharacter() const;
    void DrawHUD() const;

    void Integrate(float dt);
    void HandleWallCollisions();
    void SpawnOutsideInfluence(float minClearance);

    // random spawner for pillars
    void SpawnRandomPillars(int maxPerType = 2, float margin = 80.f);

private:
    // window + GL
    Window m_Window;
    SDL_Rect m_Viewport{0,0,0,0};

    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> m_pWindow{nullptr, SDL_DestroyWindow};
    std::unique_ptr<void,       void(*)(void*)>       m_pContext{nullptr, SDL_GL_DeleteContext};

    bool  m_Initialized{false};
    float m_MaxElapsedSeconds{1.f/60.f};

    // player
    ThreeBlade m_Character{};
    float m_Vx{0.f}, m_Vy{0.f}, m_VzEnergy{0.f};
    float m_Accel{140.f}, m_Drag{0.90f}, m_EnergyDrag{0.985f}, m_MaxSpeed{220.f};
    float m_BounceLoss{0.75f};
    float m_CharacterRadius{5.f};

    // input
    bool m_HoldUp{false}, m_HoldDown{false}, m_HoldLeft{false}, m_HoldRight{false}, m_HoldBoost{false};

    // pillars
    std::vector<std::pair<ThreeBlade, gameplay::PillarType>> m_PillarArray; // static (Normal/white)
    std::vector<gameplay::MovablePillar>  m_Movable;     // Linear/Seek/Orbit
    std::vector<gameplay::ReflectPillar>  m_Reflectors;  // Reflect

    // single active pillar (for swirl), index into combined pillar list
    int   m_CurrentPillarIndex{-1};
    bool  m_AutoRotateActive{true};
    float m_ActiveRotateTimer{0.f};
    float m_ActiveRotatePeriod{3.f};   // seconds, change active pillar periodically

    // RNG (spawners + active rotation)
    std::mt19937 m_Rng;

    // maze
    gameplay::Maze m_Maze;
};
