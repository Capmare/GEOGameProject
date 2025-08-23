// Game.cpp
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "Game.h"
#include "utils.h"

// gameplay modules
#include "gameplay/GeoMotors.h"
#include "gameplay/PlayerController.h"
#include "gameplay/CollisionSystem.h"
#include "gameplay/PillarRenderer.h"
#include "gameplay/PlayerRenderer.h"
#include "gameplay/HUDRenderer.h"

using namespace utils;

Game::Game(const Window &window)
    : m_Window{window}
      , m_Viewport{0, 0, window.width, window.height} {
    InitializeGameEngine();

    // clamp dt to ~16.7 ms
    m_MaxElapsedSeconds = 1.f / 60.f;

    // basic motion tuning
    m_Vx = m_Vy = 0.f;
    m_VzEnergy = 0.f;
    m_Accel = 140.f;
    m_Drag = 0.90f;
    m_EnergyDrag = 0.985f;
    m_MaxSpeed = 220.f;
    m_BounceLoss = 0.75f;

    // start roughly center (y-up world thanks to glOrtho(0,w,0,h,...))
    m_Character = ThreeBlade{m_Window.width / 2.f, m_Window.height / 2.f, 0.f, 1.f};

    // world setup -> add a couple of pillars
    AddPillar({m_Window.width / 2.f, m_Window.height / 2.f, 0.f, 1.f});
    AddPillar({200.f, 300.f, 0.f, 1.f});


    // Linear mover to the right
    auto p1 = gameplay::MovablePillar::MakeLinear(ThreeBlade(200.f, 300.f, 0.f),
                                                  60.f, 0.f, 240.f);

    // CCW orbit of radius 100 around anchor (250,250)
    auto anchor = ThreeBlade(250.f, 250.f, 0.f);
    auto start = ThreeBlade(350.f, 250.f, 0.f);
    auto p2 = gameplay::MovablePillar::MakeOrbit(anchor, start, +1.2f, 240.f);

    // Seeker drifting toward a hotspot
    auto p3 = gameplay::MovablePillar::MakeSeek(ThreeBlade(100.f, 100.f, 0.f),
                                                ThreeBlade(500.f, 400.f, 0.f),
                                                150.f, 300.f, 240.f);

    m_Movable.push_back(p1);
    m_Movable.push_back(p2);
    m_Movable.push_back(p3);

    AddReflector(ThreeBlade(450.f, 180.f, 0.f), 90.f);
    AddReflector(ThreeBlade(120.f, 420.f, 0.f), 110.f);


    // pick a spawn that clears the pillars’ influence ring
    SpawnOutsideInfluence(150.f);
}

Game::~Game() {
    CleanupGameEngine();
}

void Game::InitializeGameEngine() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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

    if (!m_pWindow) {
        std::cerr << "BaseGame::Initialize(), SDL_CreateWindow: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GLContext raw = SDL_GL_CreateContext(m_pWindow.get());
    if (!raw) {
        std::cerr << "BaseGame::Initialize(), SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        return;
    }
    m_pContext.reset(raw);

    if (m_Window.isVSyncOn) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "BaseGame::Initialize(), SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
            return;
        }
    } else {
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

    if (TTF_Init() == -1) {
        std::cerr << "BaseGame::Initialize(), TTF_Init: " << TTF_GetError() << std::endl;
        return;
    }

    m_Initialized = true;
}

void Game::CleanupGameEngine() {
    // destroy GL context and window via unique_ptr deleters
    m_pContext.reset();
    m_pWindow.reset();

    TTF_Quit();
    SDL_Quit();
}

void Game::Run() {
    if (!m_Initialized) {
        std::cerr << "BaseGame::Run(), not initialized\n";
        std::cin.get();
        return;
    }

    bool quit{false};
    auto t1 = std::chrono::steady_clock::now();

    SDL_Event e{};
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: quit = true;
                    break;
                case SDL_KEYDOWN: ProcessKeyDownEvent(e.key);
                    break;
                case SDL_KEYUP: ProcessKeyUpEvent(e.key);
                    break;

                // SDL mouse is top-left origin -> convert to our bottom-left (y-up)
                case SDL_MOUSEMOTION: e.motion.y = int(m_Window.height) - e.motion.y;
                    break;
                case SDL_MOUSEBUTTONDOWN: e.button.y = int(m_Window.height) - e.button.y;
                    break;
                case SDL_MOUSEBUTTONUP: e.button.y = int(m_Window.height) - e.button.y;
                    break;
            }
        }

        if (!quit) {
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
void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent &e) {
    if (e.repeat) return;
    switch (e.keysym.scancode) {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP: m_HoldUp = true;
            break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN: m_HoldDown = true;
            break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT: m_HoldLeft = true;
            break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT: m_HoldRight = true;
            break;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT: m_HoldBoost = true;
            break;

        case SDL_SCANCODE_SPACE: // bounce impulse -> feed some energy into rotation
            m_Vx = -m_Vx;
            m_Vy = -m_Vy;
            m_VzEnergy += 0.35f * std::sqrt(m_Vx * m_Vx + m_Vy * m_Vy);
            m_VzEnergy = std::clamp(m_VzEnergy, -600.f, 600.f);
            break;

        case SDL_SCANCODE_E: {
            int total = int(m_PillarArray.size() + m_Movable.size());
            if (total > 0) {
                m_CurrentPillarIndex = (m_CurrentPillarIndex + 1) % total;
            }
            break;
        }
        case SDL_SCANCODE_Q: {
            int total = int(m_PillarArray.size() + m_Movable.size());
            if (total > 0) {
                m_CurrentPillarIndex = (m_CurrentPillarIndex - 1 + total) % total;
            }
            break;
        }
        case SDL_SCANCODE_RETURN:
            GEOAClone();
            break;
        default: break;
    }
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent &e) {
    switch (e.keysym.scancode) {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP: m_HoldUp = false;
            break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN: m_HoldDown = false;
            break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT: m_HoldLeft = false;
            break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT: m_HoldRight = false;
            break;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT: m_HoldBoost = false;
            break;
        default: break;
    }
}

// update / draw
void Game::Update(float dt) {
    Integrate(dt);
    HandleWallCollisions();
}

void Game::Draw() const {
    glClearColor(0.05f, 0.06f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawPillars();
    DrawCharacter();
    DrawHUD();
}

// pick a spawn that is far enough from the pillars’ influence
void Game::SpawnOutsideInfluence(float minClearance) {
    const float pad = 48.f; // keep away from walls a bit
    struct Cand {
        float x, y;
    };
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
        for (const ThreeBlade &C: m_PillarArray) {
            const float dx = px - C[0];
            const float dy = py - C[1];
            const float d = std::sqrt(dx * dx + dy * dy);
            dmin = std::min(dmin, d);
        }
        return dmin;
    };

    // choose the candidate that maximizes the minimum distance to all pillars
    float bestX = cands[0].x, bestY = cands[0].y;
    float bestMin = -1.f;
    for (const auto &c: cands) {
        float md = minDistToPillars(c.x, c.y);
        if (md > bestMin) {
            bestMin = md;
            bestX = c.x;
            bestY = c.y;
        }
    }

    // if that spot is still too close -> push it outward away from the nearest pillar
    if (!m_PillarArray.empty() && bestMin < minClearance) {
        int nearest = 0;
        float ndx = bestX - m_PillarArray[0][0];
        float ndy = bestY - m_PillarArray[0][1];
        float nr = std::sqrt(ndx * ndx + ndy * ndy);
        for (int i = 1; i < (int) m_PillarArray.size(); ++i) {
            float dx = bestX - m_PillarArray[i][0];
            float dy = bestY - m_PillarArray[i][1];
            float r = std::sqrt(dx * dx + dy * dy);
            if (r < nr) {
                nr = r;
                ndx = dx;
                ndy = dy;
                nearest = i;
            }
        }

        if (nr < 1e-4f) {
            ndx = 1.f;
            ndy = 0.f;
            nr = 1.f;
        }
        const float push = (minClearance - nr) / nr;
        bestX += ndx * push;
        bestY += ndy * push;

        // keep inside the window
        bestX = std::clamp(bestX, pad, m_Window.width - pad);
        bestY = std::clamp(bestY, pad, m_Window.height - pad);
    }

    // place the player and reset motion/energy
    m_Character = ThreeBlade{bestX, bestY, 0.f, 1.f};
    m_Vx = m_Vy = 0.f;
    m_VzEnergy = 0.f;
}

// gameplay delegation
void Game::Integrate(float dt) {
    gameplay::InputState in{m_HoldUp, m_HoldDown, m_HoldLeft, m_HoldRight, m_HoldBoost};

    gameplay::PlayerController::Tuning tune{};
    tune.bounceLoss = m_BounceLoss;

    // step movables
    for (auto& mp : m_Movable)
        mp.Step(dt, 0.f, 0.f, m_Window.width, m_Window.height, m_BounceLoss);

    for (auto& rp : m_Reflectors)
        rp.TryReflect(m_Character, m_Vx, m_Vy, dt);

    // combine static + movable
    std::vector<ThreeBlade> pillars;
    pillars.reserve(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    pillars.insert(pillars.end(), m_PillarArray.begin(), m_PillarArray.end());
    for (const auto& mp : m_Movable)    pillars.push_back(mp.C);
    for (const auto& rp : m_Reflectors) pillars.push_back(rp.Center());

    // active index clamped to combined set
    int active = pillars.empty() ? -1
               : std::clamp(m_CurrentPillarIndex, 0, int(pillars.size()) - 1);

    gameplay::PlayerController::StepKinematics(
        m_Character, m_Vx, m_Vy, m_VzEnergy,
        in, pillars, active, dt, tune);
}

void Game::HandleWallCollisions() {
    gameplay::CollisionSystem::ResolveWalls(
        m_Character, m_Vx, m_Vy, m_VzEnergy,
        m_CharacterRadius, m_Window.width, m_Window.height, m_BounceLoss);
}

// drawing
void Game::DrawPillars() const {
    std::vector<ThreeBlade> pillars;
    pillars.reserve(m_PillarArray.size() + m_Movable.size() + m_Reflectors.size());
    pillars.insert(pillars.end(), m_PillarArray.begin(), m_PillarArray.end());
    for (const auto& mp : m_Movable)    pillars.push_back(mp.C);
    for (const auto& rp : m_Reflectors) pillars.push_back(rp.Center());

    int active = pillars.empty() ? -1
               : std::clamp(m_CurrentPillarIndex, 0, int(pillars.size()) - 1);

    gameplay::PillarRenderer::Draw(pillars, active);
}

void Game::DrawCharacter() const {
    gameplay::PlayerRenderer::Draw(m_Character, m_CharacterRadius, m_VzEnergy);
}

void Game::DrawHUD() const {
    gameplay::HUDRenderer::Draw(m_Vx, m_Vy, m_MaxSpeed, m_VzEnergy, m_Window.height);
}

// GEOA wrappers
Motor Game::MakeTranslator(float dx, float dy) const {
    return gameplay::GeoMotors::MakeTranslator(dx, dy);
}

Motor Game::MakeRotationAboutPillar(const ThreeBlade &C, float angRad) const {
    return gameplay::GeoMotors::MakeRotationAboutPoint(C, angRad);
}

Motor Game::Reverse(const Motor &m) {
    return gameplay::GeoMotors::Reverse(m);
}

ThreeBlade Game::ApplyMotor(const ThreeBlade &X, const Motor &M) {
    return gameplay::GeoMotors::Apply(X, M);
}

// world editing helpers
void Game::AddPillar(const ThreeBlade &center) {
    m_PillarArray.push_back(center);
}

void Game::GEOAClone() {
    AddPillar(ThreeBlade{m_Character[0], m_Character[1], 0.f, 1.f});
}

void Game::AddReflector(const ThreeBlade &c, float triggerR, float cooldown) {
    m_Reflectors.push_back(gameplay::ReflectPillar::Make(c, triggerR, cooldown));
}
