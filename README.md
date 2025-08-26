# PGA Game Formulas – Plain-English Reference (ASCII)


----------------------------------------------------------------
## 2) Incidence and distances
1) Incidence test (point on line):
```cpp
OneBlade l = OneBlade(c, a, b, 0).Normalized();
float s = l | P;          // == 0 means incident
```

2) Point-to-point Euclidean distance:
```cpp
float DistPGA(const ThreeBlade& A, const ThreeBlade& B)
{
    TwoBlade L = A & B;   // line through A and B
    return L.Norm();      
}
```

3) Signed distance from point to unit line:
```cpp
float SignedDistPointLine(const ThreeBlade& P, const OneBlade& unitLine)
{
    return unitLine | P;  // positive on the line's normal side
}
```

4) Radial and tangent unit vectors from join:
```cpp
TwoBlade L = X & C;                   // X=player, C=center
float dx = L[3], dy = L[4];
float n = std::sqrt(dx*dx + dy*dy) + 1e-12f;
float rx =  dx / n, ry =  dy / n;    // radial toward C
float tx = -ry,     ty = +rx;       
```

----------------------------------------------------------------
## 3) Motors

A) Pure translator in 2D (ideal directions only):
```cpp
// e01 -> +X, e02 -> +Y
static const TwoBlade kDirX(1,0,0,0,0,0);
static const TwoBlade kDirY(0,1,0,0,0,0);

Motor MakeTranslator(float dx, float dy)
{
    Motor M{1,0,0,0,0,0,0,0};
    if (std::fabs(dx) > 0) M = Motor::Translation(dx, kDirX) * M;
    if (std::fabs(dy) > 0) M = Motor::Translation(dy, kDirY) * M;
    return M;
}
```
Apply translator to a point:
```cpp
X = (T * X) * ~T;  // X' in world
```

B) Rotation about a finite point C:
```cpp
// axis with Euclidean e12 part
static const TwoBlade kAxisZ = TwoBlade::LineFromPoints(0,0,0, 0,0,1);

Motor MakeRotationAboutPoint(const ThreeBlade& C, float angleRad)
{
    float w = C.Norm(); float x = C[0], y = C[1];
    if (std::fabs(w) > 1e-6f) { x /= w; y /= w; }
    Motor Tneg = MakeTranslator(-x, -y);
    Motor Rz   = Motor::Rotation(angleRad / DEG_TO_RAD, kAxisZ); 
    Motor Tpos = MakeTranslator(+x, +y);
    return Tpos * (Rz * Tneg);
}
```

C) Reflection across a unit line l:
```cpp
// Reflect a point across l (unit):
ThreeBlade ReflectPoint(const ThreeBlade& P, const OneBlade& l_unit)
{
    MultiVector mvP; mvP = P;
    MultiVector mvl; mvl = l_unit;
    MultiVector out = (mvl * mvP) * mvl; // l P l
    return out.Grade3();
}

// Reflect a velocity vector :
void ReflectVelocityAcrossUnitLine(float& vx, float& vy, const OneBlade& l_unit)
{
    Motor T  = MakeTranslator(vx, vy);
    Motor Tr = (MultiVector(l_unit) * T * MultiVector(l_unit)).ToMotor();
    ThreeBlade O(0,0,0);
    ThreeBlade P = (T  * O) * ~T;  // after 1 sec
    ThreeBlade Pr= (Tr * O) * ~Tr; // reflected after 1 sec
    vx = Pr[0] - O[0];
    vy = Pr[1] - O[1];
}
```

----------------------------------------------------------------
## 4) How each gameplay feature uses PGA

A) Player translation every frame
```cpp
Motor T = MakeTranslator(vx * dt, vy * dt);
player = (T * player) * ~T;  // exact PGA translation
```

B) Gravity pull toward each pillar
- Build radial frame from join: `L = X & C`.
- Use `(rx,ry)` from L for direction.
- Accumulate `ax += g * rx; ay += g * ry;` where `g` scales like 1 / max(R, minR).

C) Swirl when inside active pillar influence
- Use tangent `(tx,ty)` from the same join to push along the orbit.
- Small centripetal assist: `a_c = k * vt^2 / R` applied inward (rx,ry).

D) Reflect pillar (one-shot while inside, reset when leaving)
- Detect enter: `R = (X & C).Norm()` .
- Build radial unit line through them, reflect velocity once: `ReflectVelocityAcrossUnitLine`.
- Lock until exit by tracking inside/outside and a latch.

E) Movable pillars 
- Linear: heading from join `S & B` (B random), speed from RNG.
- Orbit: start from anchor translated by `R` along `S & B` dir; rotation uses rotor about anchor.
- Seek: per-step direction from `C & target` and translator motor to move.

F) Maze collision (circle vs AABB) with lines
- Build 4 unit lines that bound the rectangle.
- Signed distances `sL,sR,sB,sT = line | X` tell where the center is.
- Detect edge or corner overlap using distances and point-point checks with `DistPGA`.

G) Collectibles and end gate
- Pickup: `DistPGA(player, collectible) <= r_player + r_collectible`.
- Allow finish only when all collected and `DistPGA(player, endCenter) <= r_sum`.

----------------------------------------------------------------
## 5) Ready-to-use snippets (copy/paste)

Point-point distance:
```cpp
inline float DistPGA(const ThreeBlade& A, const ThreeBlade& B)
{
    return (A & B).Norm();
}
```

Signed distance to a unit line:
```cpp
inline float SignedDist(const ThreeBlade& P, const OneBlade& unitLine)
{
    return unitLine | P;
}
```

Unit line from ax + by + c = 0:
```cpp
inline OneBlade UnitLine(float a, float b, float c)
{
    OneBlade l(c, a, b, 0.f);
    return l.Normalized();
}
```

Radial/tangent from two points:
```cpp
inline void RadialTangent(const ThreeBlade& X, const ThreeBlade& C,
                          float& rx, float& ry, float& tx, float& ty)
{
    TwoBlade L = X & C; float dx=L[3], dy=L[4];
    float n = std::sqrt(dx*dx + dy*dy) + 1e-12f;
    rx =  dx / n; ry =  dy / n;
    tx = -ry;     ty = +rx;
}
```

Translator motor:
```cpp
static const TwoBlade kDirX(1,0,0,0,0,0);
static const TwoBlade kDirY(0,1,0,0,0,0);
inline Motor MakeTranslator(float dx, float dy)
{
    Motor M{1,0,0,0,0,0,0,0};
    if (std::fabs(dx) > 0) M = Motor::Translation(dx, kDirX) * M;
    if (std::fabs(dy) > 0) M = Motor::Translation(dy, kDirY) * M;
    return M;
}
```

Rotation about point C:
```cpp
static const TwoBlade kAxisZ = TwoBlade::LineFromPoints(0,0,0, 0,0,1);
inline Motor MakeRotationAboutPoint(const ThreeBlade& C, float angleRad)
{
    float w=C.Norm(); float x=C[0], y=C[1];
    if (std::fabs(w)>1e-6f) { x/=w; y/=w; }
    Motor Tneg = MakeTranslator(-x, -y);
    Motor Rz   = Motor::Rotation(angleRad / DEG_TO_RAD, kAxisZ);
    Motor Tpos = MakeTranslator(+x, +y);
    return Tpos * (Rz * Tneg);
}
```

Reflect velocity across a unit line:
```cpp
inline void ReflectVelocityAcrossUnitLine(float& vx, float& vy, const OneBlade& l_unit)
{
    Motor T  = MakeTranslator(vx, vy);
    Motor Tr = (MultiVector(l_unit) * T * MultiVector(l_unit)).ToMotor();
    ThreeBlade O(0,0,0);
    ThreeBlade P  = (T  * O) * ~T;
    ThreeBlade Pr = (Tr * O) * ~Tr;
    vx = Pr[0] - O[0];
    vy = Pr[1] - O[1];
}
```

Maze AABB lines from rect {x,y,w,h}:
```cpp
inline void MakeBoxUnitLines(float x, float y, float w, float h,
                             OneBlade& L, OneBlade& R, OneBlade& B, OneBlade& T)
{
    L = UnitLine(+1.f,  0.f, -x);        // +x - left >= 0
    R = UnitLine(-1.f,  0.f, +(x+w));    // -x + right >= 0
    B = UnitLine( 0.f, +1.f, -y);        // +y - bottom >= 0
    T = UnitLine( 0.f, -1.f, +(y+h));    // -y + top >= 0
}
```

