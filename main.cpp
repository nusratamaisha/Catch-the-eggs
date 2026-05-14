
#include <GL/glut.h>
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

// ── NEW: Chicken struct ──
struct Chicken {
    float x   = 0.f;
    float y   = 0.70f;
    float vx  = 0.45f;
    float bob = 0.f;
};
static Chicken chicken;

float frand(float a, float b){
    return a + (b-a) * (rand() / (float)RAND_MAX);
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

// ── NEW: draw chicken using OpenGL primitives ──
void drawChicken() {
    glPushMatrix();
    glTranslatef(chicken.x, chicken.y + chicken.bob, 0);

    // Main body
    glColor3f(0.95f, 0.95f, 0.95f);
    drawCircle(0, 0, 0.065f, 36);

    // Lower body
    glColor3f(1.f, 1.f, 1.f);
    drawCircle(-0.015f, -0.01f, 0.045f, 28);

    // Head
    glColor3f(0.9f, 0.9f, 0.9f);
    drawCircle(0.055f, 0.05f, 0.037f, 28);

    // Red comb (three circles)
    glColor3f(0.9f, 0.2f, 0.2f);
    drawCircle(0.04f,  0.08f, 0.012f, 16);
    drawCircle(0.055f, 0.09f, 0.012f, 16);
    drawCircle(0.07f,  0.08f, 0.012f, 16);

    // Orange beak
    glColor3f(1.f, 0.45f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.09f,  0.050f);
    glVertex2f(0.115f, 0.055f);
    glVertex2f(0.09f,  0.065f);
    glEnd();

    // Black eye
    glColor3f(0, 0, 0);
    drawCircle(0.064f, 0.058f, 0.006f, 14);

    glPopMatrix();
}

void drawGradientBG() {
    // Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 0.9f);   glVertex2f(worldL, 0.15f);
    glColor3f(0.5f, 0.7f, 0.9f);   glVertex2f(worldR, 0.15f);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldR, worldT);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldL, worldT);
    glEnd();

    // Sun
    glColor4f(1.f, 0.93f, 0.45f, 0.9f);
    drawCircle(-0.78f, 0.82f, 0.10f, 48);

    // Clouds
    for(const auto& c : clouds)
        drawCloud(c);

    // Distant hills
    glColor3f(0.45f, 0.70f, 0.45f);
    drawRect(-0.2f, 0.22f, 1.2f, 0.12f);
    glColor3f(0.35f, 0.60f, 0.35f);
    drawRect( 0.15f, 0.12f, 1.1f, 0.10f);

    // House
    float hx = 0.65f, hy = 0.2f, hw = 0.15f, hh = 0.15f;
    glColor3f(0.8f, 0.45f, 0.25f);
    drawRect(hx, hy, hw, hh);
    glColor3f(0.55f, 0.15f, 0.05f);
    glBegin(GL_TRIANGLES);
    glVertex2f(hx-hw, hy+hh); glVertex2f(hx+hw, hy+hh); glVertex2f(hx, hy+hh*1.8f);
    glEnd();
    glColor3f(0.4f, 0.25f, 0.1f);
    drawRect(hx+hw*0.4f, hy-hh*0.5f, hw*0.25f, hh*0.5f);
    glColor3f(0.6f, 0.8f, 1.0f);
    drawRect(hx-hw*0.4f, hy+hh*0.2f, hw*0.3f,  hh*0.3f);

    // Foreground grass
    glColor3f(0.2f, 0.65f, 0.3f);
    drawRect(0, -0.94f, 1.f, 0.14f);

    // Grass blades
    glLineWidth(2);
    glColor3f(0.15f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for(int i = 0; i < 50; i++){
        float x = worldL + 0.04f*i + fmodf(i*0.03f, 0.02f);
        glVertex2f(x, -0.92f);
        glVertex2f(x+0.01f, -0.88f);
    }
    glEnd();
}

void display() {
    glClearColor(0.75f, 0.85f, 0.70f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGradientBG();

    // ── NEW: bamboo stick (crossbar) ──
    glColor3f(0.5f, 0.35f, 0.15f);
    drawRect(0, 0.65f, 0.92f, 0.018f);

    // ── NEW: draw chicken on the stick ──
    chicken.y = 0.70f;
    drawChicken();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    ortho();
}

int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Catch The Eggs");
    ortho();
    enableSmooth();

    // Initialize 5 clouds
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
    glutMainLoop();
    return 0;
}


