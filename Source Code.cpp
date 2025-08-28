#define FREEGLUT_STATIC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 700;

bool startedDrawing = false;
bool doneDrawing = false;
bool isScalingDown = true;
bool startRotation = false;
float timeStartRotation;
bool timeRecorded = false;

std::vector<std::pair<float, float>> points;
float patternRotationAngle = 0.0f;
const float patternRotationSpeed = 0.001f;
float patternScaleFactor = 1.0f;
const float patternScaleSpeed = 0.001f;

const int numParticles = 100;
std::vector<std::pair<float, float>> particles;
std::vector<float> particleSizes;
std::vector<float> particlePhases;
std::vector<std::vector<std::pair<float, float>>> particleVertices;

const int numTriangles = 3;
std::vector<std::vector<std::pair<float, float>>> startingPointVertices;
std::vector<float> rotationAngles;
std::vector<float> rotationSpeeds = { 0.3f, -0.2f, 0.1f };
const float startPointX = 0.0f;
const float startPointY = 0.7f;
float startingPointRadius = 0.05f;
float startPointScaleFactor = 1.0f;
const float startPointScaleSpeed = 0.01f;

float startElementAlpha = 0.8f;
float endElementAlpha = 0.0f;

const float alphaChangeRate = 0.002f;

bool resetting = false;
bool resettingFadeIn = true;
float resetMaskAlpha = 0.0f;

float offsetStep = 0.02f;
float alphaStep = 0.1f;

void setDynamicColor(float alpha = 1.0f, int layer = 0) {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    int colorParameter = 5;
    if (!isScalingDown) {
        float r = (sin(time + layer) + colorParameter) / (colorParameter + 1);
        float g = (sin(time + layer + 2.0f) + colorParameter) / (colorParameter + 1);
        float b = (sin(time + layer + 4.0f) + colorParameter) / (colorParameter + 1);
        glColor4f(r, g, b, alpha);
    }
    else {
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
    }
}

void drawText(const char* text, float x, float y, float spacing, float alpha, float r, float g, float b) {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glColor4f(r, g, b, alpha);
    glRasterPos2f(x, y);
    for (size_t i = 0; i < strlen(text); ++i) {
        float offsetX = sin(time + particlePhases[i]) * 0.005f;
        float offsetY = cos(time + particlePhases[i]) * 0.005f;
        if (!doneDrawing)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        else
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
        x += spacing;
        glRasterPos2f(x + offsetX, y + offsetY);
    }
}

void initParticles() {
    for (int i = 0; i < numParticles; ++i) {
        float x = (rand() % 2000 - 1000) / 1000.0f;
        float y = (rand() % 2000 - 1000) / 1000.0f;
        particles.push_back({ x, y });

        float size = (rand() % 5 + 2) / 200.0f;
        particleSizes.push_back(size);

        float phase = rand() % 1000 / 1000.0f * 2 * M_PI;
        particlePhases.push_back(phase);

        std::vector<std::pair<float, float>> vertices;
        for (int j = 0; j < 3; ++j) {
            float vx = (rand() % 1000 - 500) / 500.0f;
            float vy = (rand() % 1000 - 500) / 500.0f;
            vertices.push_back({ vx, vy });
        }
        particleVertices.push_back(vertices);
    }
}

void drawParticles() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    for (int i = 0; i < numParticles; ++i) {
        float x = particles[i].first;
        float y = particles[i].second;
        float offsetX = sin(time + particlePhases[i]) * 0.01f;
        float offsetY = cos(time + particlePhases[i]) * 0.01f;
        float alpha = (sin(time + particlePhases[i]) + 1) / 2 * 0.6 + 0.2;
        setDynamicColor(alpha);

        glBegin(GL_TRIANGLES);
        for (const auto& vertex : particleVertices[i]) {
            float vx = vertex.first * particleSizes[i];
            float vy = vertex.second * particleSizes[i];
            glVertex2f(x + offsetX + vx, y + offsetY + vy);
        }
        glEnd();
    }
}

void initStartingPoint() {
    for (int i = 0; i < numTriangles; i++) {
        std::vector<std::pair<float, float>> vertices;
        float startAngle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
        for (int j = 0; j < 3; ++j) {
            float angleOffset = (110.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f) * M_PI / 180.0f;
            float angle = startAngle + angleOffset * j;
            float vx = startPointX + startingPointRadius * cos(angle);
            float vy = startPointY + startingPointRadius * sin(angle);
            vertices.push_back({ vx, vy });
        }
        startingPointVertices.push_back(vertices);
        rotationAngles.push_back(0.0f);
    }
}

void drawStartingPoint() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    for (size_t i = 0; i < startingPointVertices.size(); i++) {
        glPushMatrix();
        rotationAngles[i] = fmod(time * rotationSpeeds[i] * 360.0f, 360.0f);
        glTranslatef(startPointX, startPointY, 0.0f);
        glRotatef(rotationAngles[i], 0.0f, 0.0f, 1.0f);
        glScalef(startPointScaleFactor, startPointScaleFactor, 1.0f);
        glTranslatef(-startPointX, -startPointY, 0.0f);
        if (startedDrawing) {
            startPointScaleFactor += startPointScaleSpeed;
            startElementAlpha -= alphaChangeRate;
            if (startElementAlpha < 0.0f) startElementAlpha = 0.0f;
        }
        setDynamicColor(startElementAlpha);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (const auto& vertex : startingPointVertices[i]) {
            glVertex2f(vertex.first, vertex.second);
        }
        glEnd();
        glPopMatrix();
    }
}

void drawSymmetricPattern() {
    glPushMatrix();
    glScalef(patternScaleFactor, patternScaleFactor, 1.0f);
    glRotatef(patternRotationAngle, 0.0f, 0.0f, 1.0f);
    int numSymmetry = 6;
    int numLayers = 5;
    for (int i = 0; i < numSymmetry; i++) {
        float angle = i * (2 * M_PI / numSymmetry);
        float cosA = cos(angle + patternRotationAngle);
        float sinA = sin(angle + patternRotationAngle);
        for (int layer = 0; layer < numLayers; ++layer) {
            float lineWidth = 7.0f - layer;
            float alpha = 0.8f - layer * alphaStep;
            float offset = layer * offsetStep;
            glLineWidth(lineWidth);
            setDynamicColor(alpha, layer);
            glBegin(GL_LINE_STRIP);
            for (const auto& point : points) {
                float x = (point.first + offset) * cosA - (point.second + offset) * sinA;
                float y = (point.first + offset) * sinA + (point.second + offset) * cosA;
                glVertex2f(x, y);
            }
            glEnd();
        }
    }
    glPopMatrix();
}

void updatePatternState() {
    if (doneDrawing) {
        if (startRotation) {
            if (!timeRecorded) {
                timeStartRotation = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                timeRecorded = true;
            }
            else {
                float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                patternRotationAngle = fmod((time - timeStartRotation) * patternRotationSpeed * 360.0f, 360.0f);
            }
        }
        else if (isScalingDown) {
            patternScaleFactor -= patternScaleSpeed / 2;
            if (patternScaleFactor <= 0.8f)
                isScalingDown = false;
        }
        else {
            patternScaleFactor += patternScaleSpeed * 2;
            if (patternScaleFactor >= 1.1f)
                startRotation = true;
        }
    }
}

void mouseMotion(int x, int y) {
    float normalizedX = x / float(SCR_WIDTH) * 2 - 1;
    float normalizedY = 1 - y / float(SCR_HEIGHT) * 2;
    if (startedDrawing) {
        points.push_back(std::make_pair(normalizedX, normalizedY));
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            float normalizedX = x / float(SCR_WIDTH) * 2 - 1;
            float normalizedY = 1 - y / float(SCR_HEIGHT) * 2;
            bool nearStartingPoint = fabs(normalizedX - startPointX) < 0.1f &&
                fabs(normalizedY - startPointY) < 0.1f;
            if (nearStartingPoint && !doneDrawing) {
                startedDrawing = true;
            }
        }
        else if (state == GLUT_UP) {
            startedDrawing = false;
            doneDrawing = true;
        }
    }
}

void keyboard(unsigned char key, int x, int y) {
    if ((key == 'r' || key == 'R') && !resetting) {
        resetting = true;
        resettingFadeIn = true;
    }
    if ((key == 's' || key == 'S') && !resetting) {
        offsetStep -= 0.001f;
    }
    if ((key == 'w' || key == 'W') && !resetting) {
        offsetStep += 0.001f;
    }
}

void reset() {
    if (resetting) {
        if (resettingFadeIn) {
            resetMaskAlpha += 0.005f;
            if (resetMaskAlpha >= 1.0f) {
                resetMaskAlpha = 1.0f;
                startedDrawing = false;
                doneDrawing = false;
                isScalingDown = true;
                startRotation = false;
                timeRecorded = false;
                timeStartRotation = 0.0f;
                points.clear();
                patternRotationAngle = 0.0f;
                patternScaleFactor = 1.0f;
                startPointScaleFactor = 1.0f;
                startElementAlpha = 0.8f;
                endElementAlpha = 0.0f;
                resettingFadeIn = false;
                offsetStep = 0.02f;
                alphaStep = 0.1f;
            }
        }
        else {
            resetMaskAlpha -= 0.01f;
            if (resetMaskAlpha <= 0.0f) {
                resetMaskAlpha = 0.0f;
                resetting = false;
            }
        }
        glColor4f(0.1f, 0.1f, 0.1f, resetMaskAlpha);
        glBegin(GL_QUADS);
        glVertex2f(-1, -1);
        glVertex2f(1, -1);
        glVertex2f(1, 1);
        glVertex2f(-1, 1);
        glEnd();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawParticles();
    drawStartingPoint();
    drawText("drag and draw", -0.25f, 0.5f, 0.035f, startElementAlpha, 1.0f, 1.0f, 1.0f);
    if (!points.empty()) {
        updatePatternState();
        drawSymmetricPattern();
    }
    if (startRotation) {
        endElementAlpha += alphaChangeRate;
        if (endElementAlpha >= 0.9f) endElementAlpha = 0.9f;
        drawText("press 'r' to restart...", 0.2f, -0.85f, 0.03f, endElementAlpha, 1.0f, 1.0f, 1.0f);
    }
    reset();
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("Draw a unique pattern!");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    initStartingPoint();
    initParticles();
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);
    glutKeyboardFunc(keyboard);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glutMainLoop();
    return 0;
}
