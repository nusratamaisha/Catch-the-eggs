// Step 2: Added scrolling clouds, hills and house
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

static int winW = 1500, winH = 920;
static float worldL=-1, worldR=1, worldB=-1, worldT=1;

// ── NEW: Cloud struct and storage ──
struct Cloud {
    float x, y, scale, speed;
};
static std::vector<Cloud> clouds;

// ── NEW: random float helper ──
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

// ── NEW: draw one cloud ──
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

void drawGradientBG() {
    // Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 0.9f);  glVertex2f(worldL, 0.15f);
    glColor3f(0.5f, 0.7f, 0.9f);  glVertex2f(worldR, 0.15f);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldR, worldT);
    glColor3f(0.7f, 0.85f, 0.95f); glVertex2f(worldL, worldT);
    glEnd();

    // Sun
    glColor4f(1.f, 0.93f, 0.45f, 0.9f);
    drawCircle(-0.78f, 0.82f, 0.10f, 48);

    // ── NEW: draw all clouds ──
    for(const auto& c : clouds)
        drawCloud(c);

    // ── NEW: distant hills ──
    glColor3f(0.45f, 0.70f, 0.45f);
    drawRect(-0.2f, 0.22f, 1.2f, 0.12f);
    glColor3f(0.35f, 0.60f, 0.35f);
    drawRect( 0.15f, 0.12f, 1.1f, 0.10f);

    // ── NEW: house ──
    float hx = 0.65f, hy = 0.2f, hw = 0.15f, hh = 0.15f;
    // Walls
    glColor3f(0.8f, 0.45f, 0.25f);
    drawRect(hx, hy, hw, hh);
    // Roof
    glColor3f(0.55f, 0.15f, 0.05f);
    glBegin(GL_TRIANGLES);
    glVertex2f(hx - hw, hy + hh);
    glVertex2f(hx + hw, hy + hh);
    glVertex2f(hx,      hy + hh * 1.8f);
    glEnd();
    // Door
    glColor3f(0.4f, 0.25f, 0.1f);
    drawRect(hx + hw*0.4f, hy - hh*0.5f, hw*0.25f, hh*0.5f);
    // Window
    glColor3f(0.6f, 0.8f, 1.0f);
    drawRect(hx - hw*0.4f, hy + hh*0.2f, hw*0.3f, hh*0.3f);

    // Foreground grass
    glColor3f(0.2f, 0.65f, 0.3f);
    drawRect(0, -0.94f, 1.f, 0.14f);

    // Grass blades
    glLineWidth(2);
    glColor3f(0.15f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for(int i = 0; i < 50; i++){
        float x = worldL + 0.04f * i + fmodf(i * 0.03f, 0.02f);
        glVertex2f(x, -0.92f);
        glVertex2f(x + 0.01f, -0.88f);
    }
    glEnd();
}

void display() {
    glClearColor(0.75f, 0.85f, 0.70f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGradientBG();
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

    // ── NEW: initialize 5 clouds ──
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
