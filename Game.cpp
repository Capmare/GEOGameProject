// Game.cpp
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <random>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "Game.h"

// gameplay modules
#include "gameplay/GeoMotors.h"
#include "gameplay/PlayerController.h"
#include "gameplay/CollisionSystem.h"
#include "Gameplay/CollisionSystemMaze.h"
#include "gameplay/PillarRenderer.h"
#include "gameplay/PlayerRenderer.h"
#include "gameplay/HUDRenderer.h"
#include "Gameplay/MazeGenerator.h"
#include "Gameplay/MazeRenderer.h"

using namespace utils;

namespace {

// map a MovablePillar mode to a PillarType for coloring
static gameplay::PillarType ToPillarType(const gameplay::MovablePillar& mp) {
    using M = gameplay::MovablePillar::Mode;
    switch (mp.mode) {
        case M::Linear: return gameplay::PillarType::Linear;
        case M::Seek:   return gameplay::PillarType::Seek;
        default:        return gameplay::PillarType::Movable; // orbit/others
    }
}

} // anon

Game::Game(const Window& window)
    : m_Window{window}
{
    m_Viewport = SDL_Rect{0, 0, int(window.width), int(window.height)};

    // seed RNG for spawners and active-rotation
    std::random_device rd;
    m_Rng.seed(rd());

    InitializeGameEngine();

    // clamp dt to ~16.7 ms
    m_MaxElapsedSeconds = 1.f / 60.f;

    // tuning
    m_Vx = m_Vy = 0.f;
    m_VzEnergy   = 0.f;
    m_Accel      = 140.f;
    m_Drag       = 0.90f;
    m_EnergyDrag = 0.985f;
    m_MaxSpeed   = 220.f;
    m_BounceLoss = 0.75f;
    m_CharacterRadius = 5.f;

    // start center (y-up world via glOrtho)
    m_Character = ThreeBlade{ m_Window.width / 2.f, m_Window.height / 2.f, 0.f, 1.f };

    // random maze each run
    gameplay::MazeGenerator::Generate(
        m_Maze, 14, 10, 80.f, 20.f,
        m_Window.width, m_Window.height, 0); // seed 0 -> random_device
    // spawn at maze start
    m_Character = m_Maze.startCenter;
    m_Vx = m_Vy = 0.f;
    m_VzEnergy = 0.f;
    m_Maze.endRadius = 22.f;

    // random pillars (0..2 of each type)
    SpawnRandomPillars(2, 80.f);

    // pick an initial active pillar if any
    int total = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    if (total > 0) {
        std::uniform_int_distribution<int> pick(0, total - 1);
        m_CurrentPillarIndex = pick(m_Rng);
    } else {
        m_CurrentPillarIndex = -1;
    }
}

Game::~Game()
{
    CleanupGameEngine();
}

void Game::InitializeGameEngine()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "BaseGame::Initialize(), SDL_Init: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    m_pWindow.reset(SDL_CreateWindow(
        m_Window.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        int(m_Window.width),
        int(m_Window.height),
        SDL_WINDOW_OPENGL));

    if (!m_pWindow)
    {
        std::cerr << "BaseGame::Initialize(), SDL_CreateWindow: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GLContext raw = SDL_GL_CreateContext(m_pWindow.get());
    if (!raw)
    {
        std::cerr << "BaseGame::Initialize(), SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        return;
    }
    m_pContext.reset(raw);

    if (m_Window.isVSyncOn)
    {
        if (SDL_GL_SetSwapInterval(1) < 0)
        {
            std::cerr << "BaseGame::Initialize(), SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
            return;
        }
    }
    else
    {
        SDL_GL_SetSwapInterval(0);
    }

    // bottom-left origin, y-up
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_Window.width, 0, m_Window.height, -1, 1);
    glViewport(0, 0, int(m_Window.width), int(m_Window.height));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (TTF_Init() == -1)
    {
        std::cerr << "BaseGame::Initialize(), TTF_Init: " << TTF_GetError() << std::endl;
        return;
    }

    m_Initialized = true;
}

void Game::CleanupGameEngine()
{
    m_pContext.reset();
    m_pWindow.reset();

    TTF_Quit();
    SDL_Quit();
}

void Game::Run()
{
    if (!m_Initialized)
    {
        std::cerr << "BaseGame::Run(), not initialized\n";
        std::cin.get();
        return;
    }

    bool quit{ false };
    auto t1 = std::chrono::steady_clock::now();

    SDL_Event e{};
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
            case SDL_QUIT: quit = true; break;
            case SDL_KEYDOWN: ProcessKeyDownEvent(e.key); break;
            case SDL_KEYUP:   ProcessKeyUpEvent(e.key);   break;

            // SDL mouse is top-left origin -> convert to our bottom-left (y-up)
            case SDL_MOUSEMOTION:     e.motion.y = int(m_Window.height) - e.motion.y;  break;
            case SDL_MOUSEBUTTONDOWN: e.button.y = int(m_Window.height) - e.button.y;  break;
            case SDL_MOUSEBUTTONUP:   e.button.y = int(m_Window.height) - e.button.y;  break;
            }
        }

        if (!quit)
        {
            auto t2 = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(t2 - t1).count();
            t1 = t2;
            dt = std::min(dt, m_MaxElapsedSeconds);

            Update(dt);
            Draw();
            SDL_GL_SwapWindow(m_pWindow.get());
        }
    }
}

// input
void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
    if (e.repeat) return;
    switch (e.keysym.scancode)
    {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:    m_HoldUp = true; break;
    case SDL_SCANCODE_S:
    case SDL_SCANCODE_DOWN:  m_HoldDown = true; break;
    case SDL_SCANCODE_A:
    case SDL_SCANCODE_LEFT:  m_HoldLeft = true; break;
    case SDL_SCANCODE_D:
    case SDL_SCANCODE_RIGHT: m_HoldRight = true; break;
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT: m_HoldBoost = true; break;

    case SDL_SCANCODE_SPACE:
        m_Vx = -m_Vx; m_Vy = -m_Vy;
        m_VzEnergy += 0.35f * std::sqrt(m_Vx * m_Vx + m_Vy * m_Vy);
        m_VzEnergy = std::clamp(m_VzEnergy, -600.f, 600.f);
        break;

    case SDL_SCANCODE_E:
    {
        // manual next active (cycles)
        int total = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
        if (total > 0) {
            if (m_CurrentPillarIndex < 0) m_CurrentPillarIndex = 0;
            else m_CurrentPillarIndex = (m_CurrentPillarIndex + 1) % total;
            m_ActiveRotateTimer = 0.f; // reset timer
        }
        break;
    }
    case SDL_SCANCODE_Q:
    {
        // manual prev active (cycles)
        int total = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
        if (total > 0) {
            if (m_CurrentPillarIndex < 0) m_CurrentPillarIndex = 0;
            else m_CurrentPillarIndex = (m_CurrentPillarIndex - 1 + total) % total;
            m_ActiveRotateTimer = 0.f; // reset timer
        }
        break;
    }
    case SDL_SCANCODE_R:
    {
        // regenerate pillars
        SpawnRandomPillars(2, 80.f);
        std::cout << "Random pillars regenerated\n";
        // pick a fresh active
        int total = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
        if (total > 0) {
            std::uniform_int_distribution<int> pick(0, total - 1);
            m_CurrentPillarIndex = pick(m_Rng);
        } else {
            m_CurrentPillarIndex = -1;
        }
        m_ActiveRotateTimer = 0.f;
        break;
    }
    case SDL_SCANCODE_RETURN:
        GEOAClone();
        break;
    default: break;
    }
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
    switch (e.keysym.scancode)
    {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:    m_HoldUp = false; break;
    case SDL_SCANCODE_S:
    case SDL_SCANCODE_DOWN:  m_HoldDown = false; break;
    case SDL_SCANCODE_A:
    case SDL_SCANCODE_LEFT:  m_HoldLeft = false; break;
    case SDL_SCANCODE_D:
    case SDL_SCANCODE_RIGHT: m_HoldRight = false; break;
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT: m_HoldBoost = false; break;
    default: break;
    }
}

// update / draw
void Game::Update(float dt)
{
    Integrate(dt);
    HandleWallCollisions();
}

void Game::Draw() const
{
    glClearColor(0.05f, 0.06f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    gameplay::MazeRenderer::Draw(m_Maze);
    DrawPillars();
    DrawCharacter();
    DrawHUD();
}

// pick a spawn that is far enough from the pillars' influence
void Game::SpawnOutsideInfluence(float minClearance)
{
    const float pad = 48.f; // keep away from walls a bit
    struct Cand { float x, y; };
    std::vector<Cand> cands = {
        {pad, pad},
        {m_Window.width - pad, pad},
        {m_Window.width - pad, m_Window.height - pad},
        {pad, m_Window.height - pad},
        {m_Window.width * 0.5f, m_Window.height - pad},
        {m_Window.width * 0.5f, pad},
        {pad, m_Window.height * 0.5f},
        {m_Window.width - pad, m_Window.height * 0.5f}
    };

    auto minDistToPillars = [&](float px, float py) -> float {
        float dmin = std::numeric_limits<float>::max();
        for (const auto& [C, T] : m_PillarArray) {
            const float dx = px - C[0];
            const float dy = py - C[1];
            const float d  = std::sqrt(dx*dx + dy*dy);
            dmin = std::min(dmin, d);
        }
        return dmin;
    };

    float bestX = cands[0].x, bestY = cands[0].y;
    float bestMin = -1.f;
    for (const auto& c : cands) {
        float md = minDistToPillars(c.x, c.y);
        if (md > bestMin) { bestMin = md; bestX = c.x; bestY = c.y; }
    }

    if (!m_PillarArray.empty() && bestMin < minClearance) {
        int nearest = 0;
        float ndx = bestX - m_PillarArray[0].first[0];
        float ndy = bestY - m_PillarArray[0].first[1];
        float nr  = std::sqrt(ndx*ndx + ndy*ndy);
        for (int i = 1; i < (int)m_PillarArray.size(); ++i) {
            float dx = bestX - m_PillarArray[i].first[0];
            float dy = bestY - m_PillarArray[i].first[1];
            float r  = std::sqrt(dx*dx + dy*dy);
            if (r < nr) { nr = r; ndx = dx; ndy = dy; nearest = i; }
        }

        if (nr < 1e-4f) { ndx = 1.f; ndy = 0.f; nr = 1.f; }
        const float push = (minClearance - nr) / nr;
        bestX += ndx * push;
        bestY += ndy * push;

        bestX = std::clamp(bestX, pad, m_Window.width  - pad);
        bestY = std::clamp(bestY, pad, m_Window.height - pad);
    }

    m_Character = ThreeBlade{ bestX, bestY, 0.f, 1.f };
    m_Vx = m_Vy = 0.f;
    m_VzEnergy = 0.f;
}

// gameplay delegation
void Game::Integrate(float dt)
{
    gameplay::CollisionSystemMaze::Resolve(
        m_Maze, m_Character, m_Vx, m_Vy, m_CharacterRadius, m_BounceLoss);

    bool reflected = false;
    for (auto& rp : m_Reflectors)
        reflected |= rp.TryReflect(m_Character, m_Vx, m_Vy, dt);



    for (auto& mp : m_Movable)
        mp.Step(dt, 0.f, 0.f, m_Window.width, m_Window.height, m_BounceLoss);

    std::vector<std::pair<ThreeBlade,gameplay::PillarType>> pillars;
    pillars.reserve(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    pillars.insert(pillars.end(), m_PillarArray.begin(), m_PillarArray.end());
    for (const auto& mp : m_Movable)    pillars.emplace_back(mp.C, ToPillarType(mp));
    for (const auto& rp : m_Reflectors) pillars.emplace_back(rp.Center(), gameplay::PillarType::Reflect);

    int total = int(pillars.size());
    int active = -1;
    if (total > 0) {
        // clamp current
        if (m_CurrentPillarIndex < 0 || m_CurrentPillarIndex >= total) m_CurrentPillarIndex = 0;

        if (m_AutoRotateActive) {
            m_ActiveRotateTimer += dt;
            if (m_ActiveRotateTimer >= m_ActiveRotatePeriod) {
                m_ActiveRotateTimer = 0.f;
                std::uniform_int_distribution<int> pick(0, total - 1);
                m_CurrentPillarIndex = pick(m_Rng);
            }
        }
        active = m_CurrentPillarIndex;
    } else {
        m_CurrentPillarIndex = -1;
        active = -1;
    }

    // blades-only for kinematics
    std::vector<ThreeBlade> blades;
    blades.reserve(pillars.size());
    std::transform(pillars.begin(), pillars.end(), std::back_inserter(blades),
                   [](const auto& pr){ return pr.first; });

    // wrap single active index into a set (vector) to match the signature
    std::vector<int> activeSet;
    if (active >= 0) activeSet.push_back(active);

    // 4) player kinematics (PGA-based)
    gameplay::InputState in{ m_HoldUp, m_HoldDown, m_HoldLeft, m_HoldRight, m_HoldBoost };
    gameplay::PlayerController::Tuning tune{}; tune.bounceLoss = m_BounceLoss;

    gameplay::PlayerController::StepKinematics(
        m_Character, m_Vx, m_Vy, m_VzEnergy, in, blades, activeSet, dt, tune);


    // 5) reflect post-kinematics (instant if we crossed into radius this frame)
    bool reflectedAfter = false;
    for (auto& rp : m_Reflectors)
        reflectedAfter |= rp.TryReflect(m_Character, m_Vx, m_Vy, dt);

    if (reflectedAfter) {
        gameplay::CollisionSystemMaze::DepenetratePosition(
            m_Maze, m_Character, m_CharacterRadius, 16, 0.75f);
    }

    // 6) end point check -> regenerate maze + pillars immediately
    {
        TwoBlade L = m_Character & m_Maze.endCenter;
        float R = L.Norm();
        if (R <= (m_Maze.endRadius + m_CharacterRadius)) {
            std::cout << "Reached end point\n";

            // new random maze
            gameplay::MazeGenerator::Generate(
                m_Maze, 14, 10, 80.f, 20.f,
                m_Window.width, m_Window.height, 0);

            // spawn player at the new start
            m_Character = m_Maze.startCenter;
            m_Vx = 0.f; m_Vy = 0.f; m_VzEnergy = 0.f;

            // new random set of pillars (0..2 of each)
            SpawnRandomPillars(2, 80.f);

            // pick a fresh active pillar
            int newTotal = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
            if (newTotal > 0) {
                std::uniform_int_distribution<int> pick(0, newTotal - 1);
                m_CurrentPillarIndex = pick(m_Rng);
            } else {
                m_CurrentPillarIndex = -1;
            }
            m_ActiveRotateTimer = 0.f;

            return;
        }
    }
}

void Game::SpawnRandomPillars(int maxPerType, float margin)
{
    // wipe existing
    m_PillarArray.clear();
    m_Movable.clear();
    m_Reflectors.clear();

    // distributions using member RNG
    std::uniform_int_distribution<int> count(0, maxPerType);
    std::uniform_real_distribution<float> xDist(margin, m_Window.width  - margin);
    std::uniform_real_distribution<float> yDist(margin, m_Window.height - margin);
    std::uniform_real_distribution<float> angDist(0.f, 6.2831853f);

    // params for movers
    std::uniform_real_distribution<float> speedDist(40.f, 120.f);
    std::uniform_real_distribution<float> omegaDist(-1.8f, 1.8f);
    std::uniform_real_distribution<float> orbitRDist(80.f, 160.f);
    std::uniform_real_distribution<float> maxSpeedDist(100.f, 180.f);
    std::uniform_real_distribution<float> accelDist(250.f, 480.f);
    std::uniform_real_distribution<float> triggerDist(70.f, 140.f);

    // counts (0..2)
    const int nNormal = count(m_Rng);
    const int nMovable = count(m_Rng);  // orbit movers
    const int nLinear  = count(m_Rng);
    const int nSeek    = count(m_Rng);
    const int nReflect = count(m_Rng);

    // Normal static pillars (white)
    for (int i = 0; i < nNormal; ++i) {
        ThreeBlade c(xDist(m_Rng), yDist(m_Rng), 0.f);
        m_PillarArray.emplace_back(c, gameplay::PillarType::Normal);
    }

    // Movable (orbit around a random anchor)
    for (int i = 0; i < nMovable; ++i) {
        float ax = xDist(m_Rng), ay = yDist(m_Rng);
        float R  = orbitRDist(m_Rng);
        float a  = angDist(m_Rng);
        float sx = ax + R * std::cos(a);
        float sy = ay + R * std::sin(a);
        float w  = omegaDist(m_Rng);
        m_Movable.push_back(
            gameplay::MovablePillar::MakeOrbit(
                ThreeBlade(ax, ay, 0.f),
                ThreeBlade(sx, sy, 0.f),
                w, 240.f));
    }

    // Linear movers with random direction/speed
    for (int i = 0; i < nLinear; ++i) {
        float x = xDist(m_Rng), y = yDist(m_Rng);
        float s = speedDist(m_Rng);
        float a = angDist(m_Rng);
        float vx = s * std::cos(a);
        float vy = s * std::sin(a);
        m_Movable.push_back(
            gameplay::MovablePillar::MakeLinear(
                ThreeBlade(x, y, 0.f),
                vx, vy, 240.f));
    }

    // Seekers: random start, random target, random accel/maxSpeed
    for (int i = 0; i < nSeek; ++i) {
        float x  = xDist(m_Rng),  y  = yDist(m_Rng);
        float tx = xDist(m_Rng),  ty = yDist(m_Rng);
        float ms = maxSpeedDist(m_Rng);
        float ac = accelDist(m_Rng);
        m_Movable.push_back(
            gameplay::MovablePillar::MakeSeek(
                ThreeBlade(x, y, 0.f),
                ThreeBlade(tx, ty, 0.f),
                ms, ac, 240.f));
    }

    // Reflectors: random center and trigger radius
    for (int i = 0; i < nReflect; ++i) {
        float x = xDist(m_Rng), y = yDist(m_Rng);
        float tr = triggerDist(m_Rng);
        m_Reflectors.push_back(
            gameplay::ReflectPillar::Make(
                ThreeBlade(x, y, 0.f), tr));
    }

    // reset active rotation timer and choose a new active if possible
    m_ActiveRotateTimer = 0.f;
    int total = int(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    if (total > 0) {
        std::uniform_int_distribution<int> pick(0, total - 1);
        m_CurrentPillarIndex = pick(m_Rng);
    } else {
        m_CurrentPillarIndex = -1;
    }
}

void Game::HandleWallCollisions()
{
    gameplay::CollisionSystem::ResolveWalls(
        m_Character, m_Vx, m_Vy, m_VzEnergy,
        m_CharacterRadius, m_Window.width, m_Window.height, m_BounceLoss);
}

// drawing
void Game::DrawPillars() const
{
    std::vector<std::pair<ThreeBlade,gameplay::PillarType>> pillars;
    pillars.reserve(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    pillars.insert(pillars.end(), m_PillarArray.begin(), m_PillarArray.end());
    for (const auto& mp : m_Movable)    pillars.emplace_back(mp.C, ToPillarType(mp));
    for (const auto& rp : m_Reflectors) pillars.emplace_back(rp.Center(), gameplay::PillarType::Reflect);

    int active = pillars.empty() ? -1
               : std::clamp(m_CurrentPillarIndex, 0, int(pillars.size()) - 1);

    gameplay::PillarRenderer::Draw(pillars, active);
}

void Game::DrawCharacter() const
{
    gameplay::PlayerRenderer::Draw(m_Character, m_CharacterRadius, m_VzEnergy);
}

void Game::DrawHUD() const
{
    gameplay::HUDRenderer::Draw(m_Vx, m_Vy, m_MaxSpeed, m_VzEnergy, m_Window.height);
}

// PGA wrappers
Motor Game::MakeTranslator(float dx, float dy) const
{
    return gameplay::GeoMotors::MakeTranslator(dx, dy);
}

Motor Game::MakeRotationAboutPillar(const ThreeBlade& C, float angRad) const
{
    return gameplay::GeoMotors::MakeRotationAboutPoint(C, angRad);
}

Motor Game::Reverse(const Motor& m)
{
    return gameplay::GeoMotors::Reverse(m);
}

ThreeBlade Game::ApplyMotor(const ThreeBlade& X, const Motor& M)
{
    return gameplay::GeoMotors::Apply(X, M);
}

// world editing helpers
void Game::AddPillar(const ThreeBlade& center)
{
    m_PillarArray.emplace_back(center, gameplay::PillarType::Normal);
}

void Game::GEOAClone()
{
    AddPillar(ThreeBlade{ m_Character[0], m_Character[1], 0.f, 1.f });
}

void Game::AddReflector(const ThreeBlade& c, float triggerR, float cooldown)
{
    (void)cooldown;
    m_Reflectors.push_back(gameplay::ReflectPillar::Make(c, triggerR, cooldown));
}
