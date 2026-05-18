// Step 7: Added collision detection, scoring, particles and float texts
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

static int winW = 1500, winH = 920;
static float worldL=-1, worldR=1, worldB=-1, worldT=1;

struct Cloud {
    float x, y, scale, speed;
};
static std::vector<Cloud> clouds;

struct Chicken {
    float x   = 0.f;
    float y   = 0.70f;
    float vx  = 0.45f;
    float bob = 0.f;
};
static Chicken chicken;

struct Basket {
    float x     = 0.f;
    float y     = -0.8f;
    float halfW = 0.16f;
    float h     = 0.09f;
};
static Basket basket;

enum class ObjType { NormalEgg, BlueEgg, GoldenEgg };

struct Falling {
    ObjType type;
    float   x, y;
    float   vy;
    float   radius;
    float   rot    = 0.f;
    float   rotSpd = 0.f;
    bool    active = true;
};
static std::vector<Falling> objs;

// ── NEW: particle struct for burst effect on catch ──
struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    float size;
    float r, g, b, a;
};
static std::vector<Particle> parts;

// ── NEW: floating score text struct ──
struct FloatText {
    float       x, y;
    std::string s;
    float       life = 1.f;
    float       vy   = 0.35f;
    float       r = 0, g = 0, b = 0;
};
static std::vector<FloatText> floatTexts;

static float spawnTimer = 0.f;
static float spawnEvery = 0.65f;

static int score    = 0;
static int timeLeft = 60;
static int lastTick = 0;

float frand(float a, float b){
    return a + (b-a) * (rand() / (float)RAND_MAX);
}

float windowToWorldX(int mx){
    return worldL + (mx / (float)winW) * (worldR - worldL);
}

void ortho() {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(worldL, worldR, worldB, worldT);
    glMatrixMode(GL_MODELVIEW);
}

void enableSmooth() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x-w, y-h); glVertex2f(x+w, y-h);
    glVertex2f(x+w, y+h); glVertex2f(x-w, y+h);
    glEnd();
}

void drawCircle(float x, float y, float r, int seg=48) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for(int i=0; i<=seg; i++){
        float th = 2.f * 3.14159f * i / seg;
        glVertex2f(x + cosf(th)*r, y + sinf(th)*r);
    }
    glEnd();
}

void drawText(float x, float y, const std::string& s,
              void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for(char c : s) glutBitmapCharacter(font, c);
}

void drawCloud(const Cloud& c) {
    glColor4f(1, 1, 1, 0.9f);
    glPushMatrix();
    glTranslatef(c.x, c.y, 0);
    glScalef(c.scale, c.scale, 1.f);
    drawCircle(-0.05f,  0.00f, 0.06f, 20);
    drawCircle( 0.05f,  0.00f, 0.06f, 20);
    drawCircle( 0.00f,  0.03f, 0.07f, 20);
    drawCircle( 0.00f, -0.02f, 0.05f, 20);
    glPopMatrix();
}

void drawChicken() {
    glPushMatrix();
    glTranslatef(chicken.x, chicken.y + chicken.bob, 0);
    glColor3f(0.95f, 0.95f, 0.95f); drawCircle(0, 0, 0.065f, 36);
    glColor3f(1.f, 1.f, 1.f);       drawCircle(-0.015f, -0.01f, 0.045f, 28);
    glColor3f(0.9f, 0.9f, 0.9f);    drawCircle(0.055f, 0.05f, 0.037f, 28);
    glColor3f(0.9f, 0.2f, 0.2f);
    drawCircle(0.04f,  0.08f, 0.012f, 16);
    drawCircle(0.055f, 0.09f, 0.012f, 16);
    drawCircle(0.07f,  0.08f, 0.012f, 16);
    glColor3f(1.f, 0.45f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.09f, 0.050f); glVertex2f(0.115f, 0.055f); glVertex2f(0.09f, 0.065f);
    glEnd();
    glColor3f(0, 0, 0);
    drawCircle(0.064f, 0.058f, 0.006f, 14);
    glPopMatrix();
}

void drawBasket() {
    glPushMatrix();
    glTranslatef(basket.x, basket.y, 0);
    glColor4f(0, 0, 0, 0.15f);
    drawRect(0, -0.02f, basket.halfW * 0.95f, 0.02f);
    glColor3f(0.65f, 0.35f, 0.15f);
    drawRect(0, 0.f, basket.halfW, basket.h * 0.85f);
    glColor3f(0.5f, 0.25f, 0.1f);
    glLineWidth(2);
    glBegin(GL_LINES);
    for(float y = -basket.h * 0.7f; y <= basket.h * 0.7f; y += basket.h * 0.28f){
        glVertex2f(-basket.halfW, y);
        glVertex2f( basket.halfW, y);
    }
    glEnd();
    glColor3f(0.4f, 0.2f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(-basket.halfW,      basket.h * 0.85f);
    glVertex2f( basket.halfW,      basket.h * 0.85f);
    glVertex2f( basket.halfW*0.9f, basket.h * 1.05f);
    glVertex2f(-basket.halfW*0.9f, basket.h * 1.05f);
    glEnd();
    glPopMatrix();
}

void drawEgg(const Falling& o) {
    glPushMatrix();
    glTranslatef(o.x, o.y, 0);
    glRotatef(o.rot, 0, 0, 1);
    if(o.type == ObjType::GoldenEgg) {
        glColor3f(1.f, 0.84f, 0.f);
    } else if(o.type == ObjType::BlueEgg) {
        glColor3f(0.45f, 0.65f, 1.f);
    } else {
        glColor3f(1.f, 1.f, 0.94f);
    }
    drawCircle(0, 0, o.radius, 40);
    glColor4f(1, 1, 1, 0.45f);
    drawCircle(-o.radius * 0.35f, o.radius * 0.25f, o.radius * 0.35f, 28);
    if(o.type == ObjType::GoldenEgg){
        glColor4f(1.f, 0.9f, 0.2f, 0.25f);
        glBegin(GL_TRIANGLE_STRIP);
        float innerR = o.radius * 1.20f;
        float outerR = o.radius * 1.35f;
        for(int i = 0; i <= 56; i++){
            float th = 2.f * 3.14159f * i / 56;
            glVertex2f(cosf(th) * innerR, sinf(th) * innerR);
            glVertex2f(cosf(th) * outerR, sinf(th) * outerR);
        }
        glEnd();
    }
    glPopMatrix();
}

Falling makeEgg(ObjType t) {
    Falling f;
    f.type   = t;
    f.x      = chicken.x + frand(-0.05f, 0.05f);
    f.y      = chicken.y - 0.06f;
    f.vy     = -frand(0.45f, 0.65f);
    f.radius = (t == ObjType::GoldenEgg) ? 0.045f : 0.038f;
    f.rotSpd = frand(-120, 120);
    return f;
}

ObjType getRandomEggType() {
    int r = rand() % 100;
    if(r < 5) {
        return ObjType::GoldenEgg;
    } else if(r < 15) {
        return ObjType::BlueEgg;
    } else {
        return ObjType::NormalEgg;
    }
}

// ── NEW: check if egg circle overlaps basket rectangle ──
bool eggHitsBasket(const Basket& b, const Falling& f) {
    // Find the closest point on the basket rectangle to the egg center
    float cx = std::max(b.x - b.halfW, std::min(f.x, b.x + b.halfW));
    float cy = std::max(b.y - b.h,     std::min(f.y, b.y + b.h));
    float dx = f.x - cx;
    float dy = f.y - cy;
    return (dx*dx + dy*dy) <= (f.radius * f.radius);
}

// ── NEW: spawn coloured particles at a position ──
void addParticles(float px, float py, int n, float r, float g, float b) {
    for(int i = 0; i < n; i++){
        Particle p;
        p.x = px;
        p.y = py;
        float ang = frand(0, 2 * 3.14159f);
        float sp  = frand(0.6f, 1.6f);
        p.vx      = cosf(ang) * sp;
        p.vy      = sinf(ang) * sp;
        p.life    = frand(0.35f, 0.75f);
        p.maxLife = p.life;
        p.size    = frand(0.008f, 0.02f);
        p.r = r;  p.g = g;  p.b = b;  p.a = 1.f;
        parts.push_back(p);
    }
}

// ── NEW: spawn a score label that drifts upward ──
void addFloatText(float px, float py,
                  const std::string& s,
                  float r, float g, float b) {
    FloatText ft;
    ft.x    = px;
    ft.y    = py;
    ft.s    = s;
    ft.r    = r;  ft.g = g;  ft.b = b;
    ft.life = 1.1f;
    floatTexts.push_back(ft);
}

// ── NEW: draw all active particles ──
void drawParticles() {
    for(const auto& p : parts){
        glColor4f(p.r, p.g, p.b, p.a);
        drawCircle(p.x, p.y, p.size, 10);
    }
}

// ── NEW: draw all active float texts ──
void drawFloatTexts() {
    for(const auto& ft : floatTexts){
        glColor3f(ft.r, ft.g, ft.b);
        drawText(ft.x - 0.02f, ft.y, ft.s);
    }
}

void drawGradientBG() {
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 0.9f);   glVertex2f(worldL, 0.15f);
    glColor3f(0.5f, 0.7f, 0.9f);   glVertex2f(worldR, 0.15f);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldR, worldT);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldL, worldT);
    glEnd();
    glColor4f(1.f, 0.93f, 0.45f, 0.9f);
    drawCircle(-0.78f, 0.82f, 0.10f, 48);
    for(const auto& c : clouds) drawCloud(c);
    glColor3f(0.45f, 0.70f, 0.45f); drawRect(-0.2f, 0.22f, 1.2f, 0.12f);
    glColor3f(0.35f, 0.60f, 0.35f); drawRect( 0.15f, 0.12f, 1.1f, 0.10f);
    float hx=0.65f, hy=0.2f, hw=0.15f, hh=0.15f;
    glColor3f(0.8f, 0.45f, 0.25f); drawRect(hx, hy, hw, hh);
    glColor3f(0.55f, 0.15f, 0.05f);
    glBegin(GL_TRIANGLES);
    glVertex2f(hx-hw, hy+hh); glVertex2f(hx+hw, hy+hh); glVertex2f(hx, hy+hh*1.8f);
    glEnd();
    glColor3f(0.4f, 0.25f, 0.1f); drawRect(hx+hw*0.4f, hy-hh*0.5f, hw*0.25f, hh*0.5f);
    glColor3f(0.6f, 0.8f, 1.0f);  drawRect(hx-hw*0.4f, hy+hh*0.2f, hw*0.3f,  hh*0.3f);
    glColor3f(0.2f, 0.65f, 0.3f); drawRect(0, -0.94f, 1.f, 0.14f);
    glLineWidth(2);
    glColor3f(0.15f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for(int i=0; i<50; i++){
        float x = worldL + 0.04f*i + fmodf(i*0.03f, 0.02f);
        glVertex2f(x, -0.92f); glVertex2f(x+0.01f, -0.88f);
    }
    glEnd();
}

void drawHUD() {
    glColor3f(0, 0, 0);
    drawText(-0.95f, 0.92f, "Score: " + std::to_string(score));
    drawText(-0.22f, 0.92f, "Time:  " + std::to_string(timeLeft));
}

void display() {
    glClearColor(0.75f, 0.85f, 0.70f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGradientBG();
    glColor3f(0.5f, 0.35f, 0.15f);
    drawRect(0, 0.65f, 0.92f, 0.018f);
    chicken.y = 0.70f;
    drawChicken();
    for(const auto& o : objs) drawEgg(o);
    drawBasket();

    // ── NEW: draw particles and float texts on top ──
    drawParticles();
    drawFloatTexts();

    drawHUD();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    ortho();
}

void special(int key, int, int) {
    if(key == GLUT_KEY_LEFT)  basket.x -= 0.08f;
    if(key == GLUT_KEY_RIGHT) basket.x += 0.08f;
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, basket.x));
}

void keyboard(unsigned char key, int, int) {
    if(key == 'a' || key == 'A') basket.x -= 0.08f;
    if(key == 'd' || key == 'D') basket.x += 0.08f;
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, basket.x));
}

void passiveMotion(int mx, int) {
    float wx = windowToWorldX(mx);
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, wx));
}

void updateScene(float dt) {
    // Chicken movement
    chicken.x += chicken.vx * dt;
    if(chicken.x >  0.82f){ chicken.x =  0.82f; chicken.vx *= -1; }
    if(chicken.x < -0.82f){ chicken.x = -0.82f; chicken.vx *= -1; }
    chicken.bob = 0.01f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.008f);

    // Cloud movement
    for(auto& c : clouds){
        c.x += c.speed * dt;
        if(c.x > worldR + c.scale * 0.15f){
            c.x     = worldL - c.scale * 0.15f;
            c.y     = frand(0.5f, worldT - 0.1f);
            c.scale = frand(0.5f, 1.2f);
            c.speed = frand(0.02f, 0.08f);
        }
    }

    // Spawn eggs
    spawnTimer += dt;
    if(spawnTimer >= spawnEvery){
        objs.push_back(makeEgg(getRandomEggType()));
        spawnTimer = 0;
        spawnEvery = std::max(0.35f, spawnEvery - 0.002f);
    }

    // Move eggs + check collision
    for(auto& o : objs){
        if(!o.active) continue;
        o.y   += o.vy    * dt;
        o.rot += o.rotSpd * dt;

        // ── NEW: collision check ──
        if(eggHitsBasket(basket, o)){
            o.active = false;

            if(o.type == ObjType::NormalEgg){
                score += 1;
                addParticles(o.x, o.y, 12, 1.f, 1.f, 0.9f);
                addFloatText(o.x, o.y, "+1", 0.f, 0.6f, 0.f);
            } else if(o.type == ObjType::BlueEgg){
                score += 5;
                addParticles(o.x, o.y, 16, 0.4f, 0.6f, 1.f);
                addFloatText(o.x, o.y, "+5", 0.1f, 0.45f, 1.f);
            } else if(o.type == ObjType::GoldenEgg){
                score += 10;
                addParticles(o.x, o.y, 20, 1.f, 0.84f, 0.f);
                addFloatText(o.x, o.y, "+10", 0.95f, 0.7f, 0.f);
            }
        }

        if(o.y < worldB - 0.25f)
            o.active = false;
    }

    // Remove inactive eggs
    objs.erase(
        std::remove_if(objs.begin(), objs.end(),
                       [](const Falling& f){ return !f.active; }),
        objs.end());

    // ── NEW: update particles ──
    for(auto& p : parts){
        p.x    += p.vx * dt;
        p.y    += p.vy * dt;
        p.vy   -= 1.6f * dt;   // gravity on particles
        p.life -= dt;
        p.a     = std::max(0.f, p.life / p.maxLife);
    }
    parts.erase(
        std::remove_if(parts.begin(), parts.end(),
                       [](const Particle& p){ return p.life <= 0; }),
        parts.end());

    // ── NEW: update float texts ──
    for(auto& ft : floatTexts){
        ft.y    += ft.vy * dt;
        ft.life -= dt;
    }
    floatTexts.erase(
        std::remove_if(floatTexts.begin(), floatTexts.end(),
                       [](const FloatText& t){ return t.life <= 0; }),
        floatTexts.end());

    // Timer countdown
    static float accumulator = 0;
    accumulator += dt;
    if(accumulator >= 1.0f){
        timeLeft--;
        accumulator = 0;
        if(timeLeft < 0) timeLeft = 0;
    }
}

void timerFunc(int) {
    int   now = glutGet(GLUT_ELAPSED_TIME);
    float dt  = (now - lastTick) / 1000.f;
    lastTick  = now;
    dt = std::min(dt, 0.033f);
    updateScene(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
}

int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Catch The Eggs");
    ortho();
    enableSmooth();
    for(int i = 0; i < 5; i++){
        clouds.push_back({
            frand(worldL, worldR),
            frand(0.5f, worldT - 0.1f),
            frand(0.5f, 1.2f),
            frand(0.02f, 0.08f)
        });
    }
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(passiveMotion);
    lastTick = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerFunc, 0);
    glutMainLoop();
    return 0;
}
