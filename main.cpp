// Step 9: Added menu screen with Start, Resume, Help and Exit buttons
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

enum class ObjType { NormalEgg, BlueEgg, GoldenEgg, Poop };

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

struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    float size;
    float r, g, b, a;
};
static std::vector<Particle> parts;

struct FloatText {
    float       x, y;
    std::string s;
    float       life = 1.f;
    float       vy   = 0.35f;
    float       r = 0, g = 0, b = 0;
};
static std::vector<FloatText> floatTexts;

// ── NEW: screen state enum ──
enum class Screen { Menu, Help, Playing, GameOver };
static Screen screenState = Screen::Menu;

// ── NEW: simple button struct ──
struct Button {
    float       x1, y1, x2, y2;
    std::string label;
    void (*action)();
};
static std::vector<Button> menuButtons;

static float spawnTimer = 0.f;
static float spawnEvery = 0.65f;

static int  score     = 0;
static int  timeLeft  = 60;
static int  lastTick  = 0;
static int  lives     = 3;
static int  highScore = 0;

float frand(float a, float b){
    return a + (b-a) * (rand() / (float)RAND_MAX);
}

// ── NEW: convert mouse pixel position to world coords ──
struct Vec2 { float x, y; };
Vec2 windowToWorld(int mx, int my){
    float wx = worldL + (mx / (float)winW) * (worldR - worldL);
    float wy = worldB + ((winH - my) / (float)winH) * (worldT - worldB);
    return {wx, wy};
}

float windowToWorldX(int mx){
    return worldL + (mx / (float)winW) * (worldR - worldL);
}

bool isOver(const Button& b, Vec2 wp){
    return wp.x >= b.x1 && wp.x <= b.x2 &&
           wp.y >= b.y1 && wp.y <= b.y2;
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

void drawPoop(const Falling& o) {
    glPushMatrix();
    glTranslatef(o.x, o.y, 0);
    glRotatef(o.rot, 0, 0, 1);
    glColor3f(0.45f, 0.25f, 0.05f);
    drawCircle(0.f,            0.f,           o.radius * 1.0f,  20);
    drawCircle(0.f, o.radius * 1.0f,          o.radius * 0.75f, 20);
    drawCircle(0.f, o.radius * 1.75f,         o.radius * 0.50f, 20);
    drawCircle(0.f, o.radius * 2.25f,         o.radius * 0.28f, 20);
    glColor4f(1.f, 1.f, 1.f, 0.20f);
    drawCircle(-o.radius * 0.3f, o.radius * 0.3f, o.radius * 0.25f, 12);
    glColor4f(0.5f, 0.85f, 0.1f, 0.75f);
    glLineWidth(2.f);
    glBegin(GL_LINES);
    glVertex2f(-o.radius * 0.5f, o.radius * 2.7f);
    glVertex2f(-o.radius * 0.9f, o.radius * 3.4f);
    glVertex2f( o.radius * 0.5f, o.radius * 2.7f);
    glVertex2f( o.radius * 0.9f, o.radius * 3.4f);
    glVertex2f( 0.f,             o.radius * 2.8f);
    glVertex2f( 0.f,             o.radius * 3.5f);
    glEnd();
    glPopMatrix();
}

void drawObj(const Falling& o) {
    if(o.type == ObjType::Poop) {
        drawPoop(o);
    } else {
        drawEgg(o);
    }
}

Falling makeObj(ObjType t) {
    Falling f;
    f.type   = t;
    f.x      = chicken.x + frand(-0.05f, 0.05f);
    f.y      = chicken.y - 0.06f;
    f.vy     = -frand(0.45f, 0.65f);
    if(t == ObjType::GoldenEgg) {
        f.radius = 0.045f;
    } else if(t == ObjType::Poop) {
        f.radius = 0.030f;
    } else {
        f.radius = 0.038f;
    }
    f.rotSpd = frand(-120, 120);
    return f;
}

ObjType getRandomObjType() {
    int r = rand() % 100;
    if(r < 5) {
        return ObjType::GoldenEgg;
    } else if(r < 15) {
        return ObjType::BlueEgg;
    } else if(r < 25) {
        return ObjType::Poop;
    } else {
        return ObjType::NormalEgg;
    }
}

bool objHitsBasket(const Basket& b, const Falling& f) {
    float cx = std::max(b.x - b.halfW, std::min(f.x, b.x + b.halfW));
    float cy = std::max(b.y - b.h,     std::min(f.y, b.y + b.h));
    float dx = f.x - cx;
    float dy = f.y - cy;
    return (dx*dx + dy*dy) <= (f.radius * f.radius);
}

void addParticles(float px, float py, int n, float r, float g, float b) {
    for(int i = 0; i < n; i++){
        Particle p;
        p.x = px;  p.y = py;
        float ang = frand(0, 2 * 3.14159f);
        float sp  = frand(0.6f, 1.6f);
        p.vx = cosf(ang)*sp;  p.vy = sinf(ang)*sp;
        p.life = frand(0.35f, 0.75f);
        p.maxLife = p.life;
        p.size = frand(0.008f, 0.02f);
        p.r = r;  p.g = g;  p.b = b;  p.a = 1.f;
        parts.push_back(p);
    }
}

void addFloatText(float px, float py, const std::string& s,
                  float r, float g, float b) {
    FloatText ft;
    ft.x = px;  ft.y = py;  ft.s = s;
    ft.r = r;   ft.g = g;   ft.b = b;
    ft.life = 1.1f;
    floatTexts.push_back(ft);
}

void drawParticles() {
    for(const auto& p : parts){
        glColor4f(p.r, p.g, p.b, p.a);
        drawCircle(p.x, p.y, p.size, 10);
    }
}

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

void drawHeart(float cx, float cy, float sz) {
    float r = sz * 0.32f;
    drawCircle(cx - r * 0.72f, cy + sz * 0.05f, r, 20);
    drawCircle(cx + r * 0.72f, cy + sz * 0.05f, r, 20);
    glBegin(GL_TRIANGLES);
    glVertex2f(cx - sz * 0.78f, cy + sz * 0.05f);
    glVertex2f(cx + sz * 0.78f, cy + sz * 0.05f);
    glVertex2f(cx,              cy - sz * 0.78f);
    glEnd();
}

void drawHUD() {
    glColor3f(0, 0, 0);
    drawText(-0.95f, 0.92f, "Score: " + std::to_string(score));
    drawText(-0.22f, 0.92f, "Time:  " + std::to_string(timeLeft));
    for(int i = 0; i < 3; i++){
        if(i < lives) {
            glColor3f(0.90f, 0.15f, 0.20f);
        } else {
            glColor3f(0.38f, 0.38f, 0.38f);
        }
        drawHeart(0.62f + i * 0.095f, 0.930f, 0.038f);
    }
}

void drawGameOver() {
    drawGradientBG();
    glColor4f(0, 0, 0, 0.70f);
    drawRect(0, 0, 0.82f, 0.35f);
    glColor3f(1, 1, 1);
    drawText(-0.16f,  0.10f, "GAME OVER");
    if(timeLeft <= 0) {
        drawText(-0.18f,  0.02f, "Time is up!");
    } else {
        drawText(-0.22f,  0.02f, "You caught poop!");
    }
    drawText(-0.32f, -0.08f,
             "Score: " + std::to_string(score) +
             "   Best: " + std::to_string(highScore));
    drawText(-0.44f, -0.18f, "Press R to restart  or  Esc for Menu");
}

// forward declare so setupMenuButtons can reference it
void startGame();
void helpScreen();
void exitGame();

// ── NEW: setup menu buttons with positions and actions ──
void setupMenuButtons() {
    float bw = 0.36f, bh = 0.07f;
    float cx = 0.f, baseY = 0.15f;
    menuButtons.clear();
    menuButtons.push_back({
        cx-bw, baseY-bh, cx+bw, baseY+bh,
        "Start", &startGame
    });
    menuButtons.push_back({
        cx-bw, baseY-0.12f-bh, cx+bw, baseY-0.12f+bh,
        "Resume", [](){
            if(screenState == Screen::Menu)
                screenState = Screen::Playing;
        }
    });
    menuButtons.push_back({
        cx-bw, baseY-0.24f-bh, cx+bw, baseY-0.24f+bh,
        "Help", &helpScreen
    });
    menuButtons.push_back({
        cx-bw, baseY-0.36f-bh, cx+bw, baseY-0.36f+bh,
        "Exit", &exitGame
    });
}

void startGame() {
    objs.clear();
    parts.clear();
    floatTexts.clear();
    score      = 0;
    timeLeft   = 60;
    lives      = 3;
    spawnTimer = 0.f;
    spawnEvery = 0.65f;
    basket     = Basket();
    chicken    = Chicken();
    lastTick   = glutGet(GLUT_ELAPSED_TIME);
    screenState = Screen::Playing;
}

void helpScreen() {
    screenState = Screen::Help;
}

void exitGame() {
    exit(0);
}

// ── NEW: draw the main menu ──
void drawMenu() {
    drawGradientBG();

    // Title
    glColor3f(0.1f, 0.1f, 0.1f);
    drawText(-0.30f, 0.72f, "CATCH THE EGGS", GLUT_BITMAP_TIMES_ROMAN_24);

    // Show chicken and basket on menu too
    chicken.y = 0.70f;
    drawChicken();
    drawBasket();

    // Draw each button
    for(auto& b : menuButtons){
        float bx = (b.x1 + b.x2) / 2.f;
        float by = (b.y1 + b.y2) / 2.f;
        float bw = (b.x2 - b.x1) / 2.f;
        float bh = (b.y2 - b.y1) / 2.f;

        // Shadow
        glColor4f(0, 0, 0, 0.2f);
        drawRect(bx, by - 0.008f, bw, bh);

        // Button body
        glColor3f(0.3f, 0.7f, 0.3f);
        drawRect(bx, by, bw, bh);

        // Button border
        glColor3f(0.2f, 0.55f, 0.2f);
        glLineWidth(2.f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x1, b.y1);
        glVertex2f(b.x2, b.y1);
        glVertex2f(b.x2, b.y2);
        glVertex2f(b.x1, b.y2);
        glEnd();

        // Label
        glColor3f(0, 0, 0);
        drawText(bx - 0.06f, by - 0.01f, b.label);
    }

    // High score at bottom
    glColor3f(0, 0, 0);
    drawText(-0.22f, -0.42f, "High Score: " + std::to_string(highScore));
}

// ── NEW: draw help screen ──
void drawHelp() {
    drawGradientBG();
    glColor3f(0, 0, 0);
    drawText(-0.95f, 0.80f, "HOW TO PLAY", GLUT_BITMAP_TIMES_ROMAN_24);
    drawText(-0.95f, 0.65f, "- Move basket:  Mouse  or  Left/Right arrows  or  A/D");
    drawText(-0.95f, 0.52f, "- Normal Egg  =  +1 point  (white)");
    drawText(-0.95f, 0.40f, "- Blue Egg    =  +5 points  (blue)");
    drawText(-0.95f, 0.28f, "- Golden Egg  =  +10 points  (gold)");
    drawText(-0.95f, 0.16f, "- Poop        =  -10 points and lose 1 life  (brown)");
    drawText(-0.95f, 0.04f, "- Lose all 3 lives or run out of time = Game Over");
    drawText(-0.95f, -0.10f,"- Press R during game over to restart");
    drawText(-0.95f, -0.22f,"- Press Esc to return to menu anytime");
    drawText(-0.95f, -0.88f,"Click anywhere to return to Menu.");
}

void display() {
    glClearColor(0.75f, 0.85f, 0.70f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if(screenState == Screen::Menu){
        drawMenu();
    } else if(screenState == Screen::Help){
        drawHelp();
    } else if(screenState == Screen::GameOver){
        drawGameOver();
    } else {
        // Playing
        drawGradientBG();
        glColor3f(0.5f, 0.35f, 0.15f);
        drawRect(0, 0.65f, 0.92f, 0.018f);
        chicken.y = 0.70f;
        drawChicken();
        for(const auto& o : objs) drawObj(o);
        drawBasket();
        drawParticles();
        drawFloatTexts();
        drawHUD();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    ortho();
}

// ── NEW: mouse click handles menu button presses ──
void mouseClick(int button, int state, int x, int y) {
    if(state != GLUT_DOWN) return;
    Vec2 wp = windowToWorld(x, y);

    if(screenState == Screen::Menu){
        for(auto& b : menuButtons){
            if(isOver(b, wp)){
                b.action();
                return;
            }
        }
    } else if(screenState == Screen::Help){
        screenState = Screen::Menu;
    } else if(screenState == Screen::GameOver){
        screenState = Screen::Menu;
    }
}

void special(int key, int, int) {
    if(screenState == Screen::Playing){
        if(key == GLUT_KEY_LEFT)  basket.x -= 0.08f;
        if(key == GLUT_KEY_RIGHT) basket.x += 0.08f;
        basket.x = std::max(worldL + basket.halfW,
                            std::min(worldR - basket.halfW, basket.x));
    }
}

void keyboard(unsigned char key, int, int) {
    if(screenState == Screen::Playing){
        if(key == 'a' || key == 'A') basket.x -= 0.08f;
        if(key == 'd' || key == 'D') basket.x += 0.08f;
        basket.x = std::max(worldL + basket.halfW,
                            std::min(worldR - basket.halfW, basket.x));
        if(key == 'r' || key == 'R') startGame();
    }
    if(screenState == Screen::GameOver){
        if(key == 'r' || key == 'R') startGame();
    }
    // Esc always goes back to menu or quits
    if(key == 27){
        if(screenState == Screen::Playing ||
           screenState == Screen::GameOver ||
           screenState == Screen::Help){
            screenState = Screen::Menu;
        } else {
            exit(0);
        }
    }
}

void passiveMotion(int mx, int) {
    if(screenState == Screen::Playing){
        float wx = windowToWorldX(mx);
        basket.x = std::max(worldL + basket.halfW,
                            std::min(worldR - basket.halfW, wx));
    }
}

void updateScene(float dt) {
    // Always animate clouds and chicken on menu too
    chicken.x += chicken.vx * dt;
    if(chicken.x >  0.82f){ chicken.x =  0.82f; chicken.vx *= -1; }
    if(chicken.x < -0.82f){ chicken.x = -0.82f; chicken.vx *= -1; }
    chicken.bob = 0.01f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.008f);

    for(auto& c : clouds){
        c.x += c.speed * dt;
        if(c.x > worldR + c.scale * 0.15f){
            c.x     = worldL - c.scale * 0.15f;
            c.y     = frand(0.5f, worldT - 0.1f);
            c.scale = frand(0.5f, 1.2f);
            c.speed = frand(0.02f, 0.08f);
        }
    }

    // Only run game logic while playing
    if(screenState != Screen::Playing) return;

    spawnTimer += dt;
    if(spawnTimer >= spawnEvery){
        objs.push_back(makeObj(getRandomObjType()));
        spawnTimer = 0;
        spawnEvery = std::max(0.35f, spawnEvery - 0.002f);
    }

    for(auto& o : objs){
        if(!o.active) continue;
        o.y   += o.vy     * dt;
        o.rot += o.rotSpd * dt;

        if(objHitsBasket(basket, o)){
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
            } else if(o.type == ObjType::Poop){
                lives--;
                score = std::max(0, score - 10);
                addParticles(o.x, o.y, 15, 0.45f, 0.25f, 0.05f);
                addFloatText(o.x, o.y, "-10", 0.6f, 0.3f, 0.f);
                if(lives <= 0){
                    highScore   = std::max(highScore, score);
                    screenState = Screen::GameOver;
                }
            }
        }
        if(o.y < worldB - 0.25f) o.active = false;
    }

    objs.erase(std::remove_if(objs.begin(), objs.end(),
               [](const Falling& f){ return !f.active; }), objs.end());

    for(auto& p : parts){
        p.x    += p.vx * dt;
        p.y    += p.vy * dt;
        p.vy   -= 1.6f * dt;
        p.life -= dt;
        p.a     = std::max(0.f, p.life / p.maxLife);
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(),
                [](const Particle& p){ return p.life <= 0; }), parts.end());

    for(auto& ft : floatTexts){
        ft.y    += ft.vy * dt;
        ft.life -= dt;
    }
    floatTexts.erase(std::remove_if(floatTexts.begin(), floatTexts.end(),
                     [](const FloatText& t){ return t.life <= 0; }), floatTexts.end());

    static float accumulator = 0;
    accumulator += dt;
    if(accumulator >= 1.0f){
        timeLeft--;
        accumulator = 0;
        if(timeLeft <= 0){
            highScore   = std::max(highScore, score);
            screenState = Screen::GameOver;
        }
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

    // ── NEW: build menu buttons once at startup ──
    setupMenuButtons();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(passiveMotion);
    lastTick = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerFunc, 0);
    glutMainLoop();
    return 0;
}