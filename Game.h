// Game.h
#pragma once
#include <memory>
#include <vector>
#include <type_traits>
#include <SDL.h>

#include "structs.h"
#include "FlyFish.h"
#include "Gameplay/Maze.h"
#include "Gameplay/MovablePillar.h"
#include "Gameplay/PillarRenderer.h"
#include "Gameplay/ReflecPillar.h"


class Game
{
public:
    explicit Game(const Window& window);
    ~Game();

    void Run();

    void ProcessKeyDownEvent(const SDL_KeyboardEvent& e);
    void ProcessKeyUpEvent(const SDL_KeyboardEvent& e);

private:
    void InitializeGameEngine();
    void CleanupGameEngine();


    void SpawnOutsideInfluence(float minClearance = 150.f);


    void Update(float dt);
    void Draw() const;

    void Integrate(float dt);
    void HandleWallCollisions();

    void DrawPillars() const;
    void DrawCharacter() const;
    void DrawHUD() const;

    // GEOA helpers wrappers that call gameplay::GeoMotors
    Motor      MakeTranslator(float dx, float dy) const;
    Motor      MakeRotationAboutPillar(const ThreeBlade& C, float angRad) const;
    static Motor      Reverse(const Motor& m);
    static ThreeBlade ApplyMotor(const ThreeBlade& X, const Motor& M);

    // simple world editing
    void AddPillar(const ThreeBlade& center);
    void AddMovablePillar(const gameplay::MovablePillar& p) { m_Movable.push_back(p); }
    void AddReflector(const ThreeBlade& c, float triggerR, float cooldown = 0.25f);

    void GEOAClone();

private:


    struct SDLWindowDeleter { void operator()(SDL_Window* w) const noexcept { if (w) SDL_DestroyWindow(w); } };
    struct GLContextDeleter { void operator()(SDL_GLContext c) const noexcept { if (c) SDL_GL_DeleteContext(c); } };

    using SDLWindowPtr = std::unique_ptr<SDL_Window, SDLWindowDeleter>;
    using GLContextPtr = std::unique_ptr<std::remove_pointer_t<SDL_GLContext>, GLContextDeleter>;

    Window      m_Window{};
    Rectf       m_Viewport{};
    SDLWindowPtr m_pWindow{ nullptr };
    GLContextPtr m_pContext{ nullptr };
    bool        m_Initialized{ false };

    float m_MaxElapsedSeconds{ 1.f / 60.f };

    bool m_HoldUp{ false }, m_HoldDown{ false }, m_HoldLeft{ false }, m_HoldRight{ false }, m_HoldBoost{ false };

    ThreeBlade m_Character{ 0.f, 0.f, 0.f, 1.f };
    float      m_CharacterRadius{ 10.f };

    float m_Vx{ 0.f }, m_Vy{ 0.f };
    float m_VzEnergy{ 0.f };
    float m_Accel{ 140.f };
    float m_Drag{ 0.90f };
    float m_EnergyDrag{ 0.985f };
    float m_MaxSpeed{ 220.f };
    float m_BounceLoss{ 0.75f };

    std::vector<std::pair<ThreeBlade,gameplay::PillarType>> m_PillarArray{};
    std::vector<gameplay::MovablePillar> m_Movable;
    std::vector<gameplay::ReflectPillar> m_Reflectors;

    gameplay::Maze m_Maze;

    int   m_CurrentPillarIndex{ 0 };







};
