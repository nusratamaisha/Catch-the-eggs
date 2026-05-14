#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

static int winW = 1500, winH = 920;
static float worldL=-1, worldR=1, worldB=-1, worldT=1;

void ortho() {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(worldL, worldR, worldB, worldT);
    glMatrixMode(GL_MODELVIEW);
}

void enableSmooth() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x-w,y-h); glVertex2f(x+w,y-h);
    glVertex2f(x+w,y+h); glVertex2f(x-w,y+h);
    glEnd();
}

void drawCircle(float x, float y, float r, int seg=48) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,y);
    for(int i=0;i<=seg;i++){
        float th=2.f*3.14159f*i/seg;
        glVertex2f(x+cosf(th)*r, y+sinf(th)*r);
    }
    glEnd();
}

void drawGradientBG() {
    // Sky
    glBegin(GL_QUADS);
    glColor3f(0.5f,0.7f,0.9f); glVertex2f(worldL,0.15f);
    glColor3f(0.5f,0.7f,0.9f); glVertex2f(worldR,0.15f);
    glColor3f(0.7f,0.85f,0.95f); glVertex2f(worldR,worldT);
    glColor3f(0.7f,0.85f,0.95f); glVertex2f(worldL,worldT);
    glEnd();
    // Sun
    glColor4f(1.f,0.93f,0.45f,0.9f);
    drawCircle(-0.78f,0.82f,0.10f,48);
    // Ground
    glColor3f(0.2f,0.65f,0.3f);
    drawRect(0,-0.94f,1.f,0.14f);
}

void display() {
    glClearColor(0.75f,0.85f,0.70f,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGradientBG();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW=w; winH=h;
    glViewport(0,0,w,h);
    ortho();
}

int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(winW,winH);
    glutCreateWindow("Catch The Eggs");
    ortho(); enableSmooth();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
