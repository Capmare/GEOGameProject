#include "Gameplay/MazeRenderer.h"
#include <SDL_opengl.h>
#include <cmath>

namespace gameplay {

    static void DrawRect(float x, float y, float w, float h)
    {
        glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
        glEnd();
    }

    static void DrawCircle(float cx, float cy, float r)
    {
        glBegin(GL_LINE_LOOP);
        const int N = 48;
        for (int i = 0; i < N; ++i) {
            float a = (2.0f * 3.14159265f * i) / N;
            float x = cx + r * std::cos(a);
            float y = cy + r * std::sin(a);
            glVertex2f(x, y);
        }
        glEnd();
    }

    void MazeRenderer::Draw(const Maze& m)
    {
        // walls
        glColor4f(0.9f, 0.85f, 0.7f, 0.8f);
        for (const auto& w : m.walls) DrawRect(w.x, w.y, w.w, w.h);

        // end point ring
        glColor4f(0.2f, 1.0f, 0.4f, 1.0f);
        DrawCircle(m.endCenter[0], m.endCenter[1], m.endRadius);
    }

} // namespace gameplay
