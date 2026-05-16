// Step 4: Added basket with keyboard and mouse controls
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
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

// ── NEW: Basket struct ──
struct Basket {
    float x     = 0.f;
    float y     = -0.8f;
    float halfW = 0.16f;
    float h     = 0.09f;
};
static Basket basket;

float frand(float a, float b){
    return a + (b-a) * (rand() / (float)RAND_MAX);
}

// ── NEW: convert window pixel coords to world coords ──
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

// ── NEW: draw the basket ──
void drawBasket() {
    glPushMatrix();
    glTranslatef(basket.x, basket.y, 0);

    // Shadow underneath
    glColor4f(0, 0, 0, 0.15f);
    drawRect(0, -0.02f, basket.halfW * 0.95f, 0.02f);

    // Main basket body
    glColor3f(0.65f, 0.35f, 0.15f);
    drawRect(0, 0.f, basket.halfW, basket.h * 0.85f);

    // Weave lines
    glColor3f(0.5f, 0.25f, 0.1f);
    glLineWidth(2);
    glBegin(GL_LINES);
    for(float y = -basket.h * 0.7f; y <= basket.h * 0.7f; y += basket.h * 0.28f){
        glVertex2f(-basket.halfW, y);
        glVertex2f( basket.halfW, y);
    }
    glEnd();

    // Top rim
    glColor3f(0.4f, 0.2f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(-basket.halfW,      basket.h * 0.85f);
    glVertex2f( basket.halfW,      basket.h * 0.85f);
    glVertex2f( basket.halfW*0.9f, basket.h * 1.05f);
    glVertex2f(-basket.halfW*0.9f, basket.h * 1.05f);
    glEnd();

    glPopMatrix();
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
    glVertex2f(hx-hw,hy+hh); glVertex2f(hx+hw,hy+hh); glVertex2f(hx,hy+hh*1.8f);
    glEnd();
    glColor3f(0.4f,0.25f,0.1f);  drawRect(hx+hw*0.4f, hy-hh*0.5f, hw*0.25f, hh*0.5f);
    glColor3f(0.6f,0.8f, 1.0f);  drawRect(hx-hw*0.4f, hy+hh*0.2f, hw*0.3f,  hh*0.3f);
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

void display() {
    glClearColor(0.75f, 0.85f, 0.70f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGradientBG();

    // Bamboo stick
    glColor3f(0.5f, 0.35f, 0.15f);
    drawRect(0, 0.65f, 0.92f, 0.018f);

    chicken.y = 0.70f;
    drawChicken();

    // ── NEW: draw basket ──
    drawBasket();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    ortho();
}

// ── NEW: arrow key controls for basket ──
void special(int key, int, int) {
    if(key == GLUT_KEY_LEFT)  basket.x -= 0.08f;
    if(key == GLUT_KEY_RIGHT) basket.x += 0.08f;
    // Clamp so basket never goes off screen
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, basket.x));
    glutPostRedisplay();
}

// ── NEW: keyboard controls (A/D keys) ──
void keyboard(unsigned char key, int, int) {
    if(key == 'a' || key == 'A') basket.x -= 0.08f;
    if(key == 'd' || key == 'D') basket.x += 0.08f;
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, basket.x));
    glutPostRedisplay();
}

// ── NEW: mouse follows basket smoothly ──
void passiveMotion(int mx, int) {
    float wx = windowToWorldX(mx);
    basket.x = std::max(worldL + basket.halfW,
                        std::min(worldR - basket.halfW, wx));
    glutPostRedisplay();
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

    // ── NEW: register input callbacks ──
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(passiveMotion);

    glutMainLoop();
    return 0;
}


