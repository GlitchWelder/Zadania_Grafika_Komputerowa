// OpenGL 3.3 Core Profile — Obiekt 1: Korkociąg | Obiekt 2: Piramida
// Kompilacja (Windows/MSYS2):
//   g++ main.cpp glad.c -o lab.exe -I include -I C:/msys64/ucrt64/include -L C:/msys64/ucrt64/lib -lglfw3 -lopengl32 -lgdi32 -std=c++17 -D_USE_MATH_DEFINES
// Wymaga: GLFW3, GLAD (glad.c w tym samym katalogu), GLM
// Sterowanie: 1/2 — wybór obiektu | strzałki / PageUp / PageDown — obrót | Home — reset

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <stack>
#include <vector>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// Parametry korkociągu i piramidy
// ─────────────────────────────────────────────────────────────────────────────
static const int   HELIX_POINTS  = 400;   // liczba punktów korkociągu
static const float HELIX_TURNS   = 5.0f;  // liczba pełnych obrotów
static const float HELIX_RADIUS  = 0.5f;  // promień spirali
static const float HELIX_HEIGHT  = 2.4f;  // wysokość korkociągu (wzdłuż osi Y)
static const float HELIX_PS_MIN  = 1.5f;  // minimalny rozmiar punktu
static const float HELIX_PS_MAX  = 18.0f; // maksymalny rozmiar punktu

static const int   PYR_N         = 8;     // liczba boków podstawy piramidy
static const float PYR_BASE_R    = 0.75f; // promień podstawy
static const float PYR_HEIGHT    = 1.8f;  // wysokość piramidy

// ─────────────────────────────────────────────────────────────────────────────
// Globalne zmienne stanu
// ─────────────────────────────────────────────────────────────────────────────
static int   objectNumber = 1;
static float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
// Shadery GLSL 3.30
// ─────────────────────────────────────────────────────────────────────────────
static const char *VS = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aCol;
layout(location = 2) in float aPS;
out vec3 vCol;
uniform mat4 MVP;
void main() {
    gl_Position  = MVP * vec4(aPos, 1.0);
    gl_PointSize = aPS;
    vCol         = aCol;
}
)glsl";

static const char *FS = R"glsl(
#version 330 core
in  vec3 vCol;
out vec4 fCol;
uniform bool uIsPoints;
void main() {
    // gl_PointCoord jest zdefiniowane tylko dla GL_POINTS
    if (uIsPoints) {
        vec2 c = gl_PointCoord - vec2(0.5);
        if (dot(c, c) > 0.25) discard;
    }
    fCol = vec4(vCol, 1.0);
}
)glsl";

// ─────────────────────────────────────────────────────────────────────────────
// Struktura wierzchołka
// ─────────────────────────────────────────────────────────────────────────────
struct Vtx { float x, y, z, r, g, b, ps; };

// ─────────────────────────────────────────────────────────────────────────────
// Kompilacja shadera
// ─────────────────────────────────────────────────────────────────────────────
static GLuint compileShader(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[512];
        glGetShaderInfoLog(s, 512, nullptr, buf);
        std::cerr << "[blad shadera] " << buf << "\n";
    }
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tworzenie VAO/VBO z wektora wierzchołków
// ─────────────────────────────────────────────────────────────────────────────
static void makeVAO(const std::vector<Vtx> &verts, GLuint &vao, GLuint &vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(Vtx),
                 verts.data(), GL_STATIC_DRAW);
    // aPos - layout 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vtx), (void*)0);
    glEnableVertexAttribArray(0);
    // aCol - layout 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vtx), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // aPS - layout 2
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                          sizeof(Vtx), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Obiekt 1 - Korkociąg wokół osi Y (kolor: czerwono-pomarańczowo-żółty gradient)
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<Vtx> buildHelix() {
    std::vector<Vtx> v;
    for (int i = 0; i < HELIX_POINTS; ++i) {
        float t   = (float)i / (float)(HELIX_POINTS - 1); // 0..1
        float ang = t * HELIX_TURNS * 2.0f * (float)M_PI;
        // korkociąg wokół osi Y: x i z tworzą koło, y rośnie
        float x   = HELIX_RADIUS * std::cos(ang);
        float y   = -HELIX_HEIGHT / 2.0f + t * HELIX_HEIGHT;
        float z   = HELIX_RADIUS * std::sin(ang);
        // Kolor: od głębokiej czerwieni (0.6,0,0) przez pomarańcz (1,0.5,0) do jasnej żółci (1,1,0.2)
        float r, g, b;
        if (t < 0.5f) {
            float s = t * 2.0f;
            r = 0.6f + s * 0.4f;
            g = s * 0.5f;
            b = 0.0f;
        } else {
            float s = (t - 0.5f) * 2.0f;
            r = 1.0f;
            g = 0.5f + s * 0.5f;
            b = s * 0.2f;
        }
        float ps = HELIX_PS_MIN + t * (HELIX_PS_MAX - HELIX_PS_MIN);
        v.push_back({x, y, z, r, g, b, ps});
    }
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// Obiekt 2 - Piramida N-kątna (hierarchiczne modelowanie)
// ─────────────────────────────────────────────────────────────────────────────

// Kanoniczna ściana boczna k=0 w przestrzeni modelu.
// Wierzchołki bazowe leżą na krawędzi 0-tej wielokąta, apex wspólny dla wszystkich ścian.
// Pozostałe ściany uzyskuje się przez obrót o k*dAng wokół Y (w drawPyramid).
static std::vector<Vtx> buildSideFace() {
    float dAng = 2.0f * (float)M_PI / (float)PYR_N;
    float bx = PYR_BASE_R * std::cos(dAng);
    float bz = PYR_BASE_R * std::sin(dAng);
    std::vector<Vtx> v;
    v.push_back({ PYR_BASE_R, 0.0f,       0.0f,  0.35f, 0.1f, 0.75f, 1.0f }); // wierzchołek 0 podstawy
    v.push_back({ bx,         0.0f,       bz,    0.35f, 0.1f, 0.75f, 1.0f }); // wierzchołek 1 podstawy
    v.push_back({ 0.0f,       PYR_HEIGHT, 0.0f,  0.0f,  0.8f, 1.0f,  1.0f }); // apex
    return v;
}

// Podstawa piramidy (GL_TRIANGLE_FAN: środek + N wierzchołków obwodu)
static std::vector<Vtx> buildBase() {
    std::vector<Vtx> v;
    // Środek
    v.push_back({0.0f, 0.0f, 0.0f,  0.15f, 0.05f, 0.4f, 1.0f});
    for (int i = 0; i <= PYR_N; ++i) {
        float ang = (float)i / (float)PYR_N * 2.0f * (float)M_PI;
        float x   = PYR_BASE_R * std::cos(ang);
        float z   = PYR_BASE_R * std::sin(ang);
        v.push_back({x, 0.0f, z,  0.25f, 0.08f, 0.6f, 1.0f});
    }
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// Stos macierzy (zastępuje glPushMatrix / glPopMatrix)
// ─────────────────────────────────────────────────────────────────────────────
static void drawMesh(GLint uMVP, std::stack<glm::mat4> &ms,
                     GLuint vao, GLsizei count, GLenum mode) {
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(ms.top()));
    glBindVertexArray(vao);
    glDrawArrays(mode, 0, count);
}

// Rysowanie piramidy hierarchicznie
static void drawPyramid(GLint uMVP, std::stack<glm::mat4> &ms,
                        GLuint sideVAO, GLuint baseVAO) {
    // Podstawa (GL_TRIANGLE_FAN: środek + N+1 wierzchołków)
    drawMesh(uMVP, ms, baseVAO, PYR_N + 2, GL_TRIANGLE_FAN);

    // N ścian bocznych - każda to kanoniczna ściana k=0 obrócona o k*dAng wokół Y.
    // Apex (0, PYR_HEIGHT, 0) jest wspólny i niezmieniony przez obrót wokół Y.
    float dAng = 2.0f * (float)M_PI / (float)PYR_N;
    for (int k = 0; k < PYR_N; ++k) {
        ms.push(ms.top()); // glPushMatrix - zapisz bieżącą macierz
        ms.top() *= glm::rotate(glm::mat4(1.f),
                                (float)k * dAng,
                                glm::vec3(0.f, 1.f, 0.f));
        drawMesh(uMVP, ms, sideVAO, 3, GL_TRIANGLES);
        ms.pop(); // glPopMatrix - wróć do zapisanej macierzy
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Obsługa klawiatury
// ─────────────────────────────────────────────────────────────────────────────
static void keyCallback(GLFWwindow *, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    const float step = 5.0f;
    switch (key) {
        case GLFW_KEY_1:         objectNumber = 1; break;
        case GLFW_KEY_2:         objectNumber = 2; break;
        case GLFW_KEY_UP:        rotX -= step;     break;
        case GLFW_KEY_DOWN:      rotX += step;     break;
        case GLFW_KEY_LEFT:      rotY -= step;     break;
        case GLFW_KEY_RIGHT:     rotY += step;     break;
        case GLFW_KEY_PAGE_UP:   rotZ -= step;     break;
        case GLFW_KEY_PAGE_DOWN: rotZ += step;     break;
        case GLFW_KEY_HOME:      rotX = rotY = rotZ = 0.0f; break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(800, 600,
        "Lab GK - 1: Korkociag | 2: Piramida", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetKeyCallback(win, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Nie mozna zaladowac GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Program shaderów
    GLuint vs   = compileShader(GL_VERTEX_SHADER,   VS);
    GLuint fs   = compileShader(GL_FRAGMENT_SHADER, FS);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glUseProgram(prog);
    GLint uMVP      = glGetUniformLocation(prog, "MVP");
    GLint uIsPoints = glGetUniformLocation(prog, "uIsPoints");

    // ── Obiekt 1: Korkociąg ──────────────────────────────────────────────────
    std::vector<Vtx> helixVerts = buildHelix();
    GLuint helixVAO, helixVBO;
    makeVAO(helixVerts, helixVAO, helixVBO);

    // ── Obiekt 2: Piramida ───────────────────────────────────────────────────
    std::vector<Vtx> sideVerts = buildSideFace();
    GLuint sideVAO, sideVBO;
    makeVAO(sideVerts, sideVAO, sideVBO);

    std::vector<Vtx> baseVerts = buildBase();
    GLuint baseVAO, baseVBO;
    makeVAO(baseVerts, baseVAO, baseVBO);

    // ── Pętla renderowania ───────────────────────────────────────────────────
    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.04f, 0.04f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
        glm::mat4 proj = glm::perspective(glm::radians(40.0f), aspect, 0.1f, 50.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(2.f, 2.0f, 5.f),
            glm::vec3(0.f, 0.5f, 0.f),
            glm::vec3(0.f, 1.0f, 0.f));
        glm::mat4 model(1.f);
        model = glm::rotate(model, glm::radians(rotX), {1.f, 0.f, 0.f});
        model = glm::rotate(model, glm::radians(rotY), {0.f, 1.f, 0.f});
        model = glm::rotate(model, glm::radians(rotZ), {0.f, 0.f, 1.f});

        glm::mat4 MVP = proj * view * model;

        if (objectNumber == 1) {
            // Korkociąg - gl_PointCoord aktywne
            glUniform1i(uIsPoints, 1);
            glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(MVP));
            glBindVertexArray(helixVAO);
            glDrawArrays(GL_POINTS, 0, (GLsizei)helixVerts.size());
        } else {
            // Piramida - gl_PointCoord wyłączone (niezdefiniowane dla trójkątów)
            glUniform1i(uIsPoints, 0);
            std::stack<glm::mat4> ms;
            ms.push(MVP);
            drawPyramid(uMVP, ms, sideVAO, baseVAO);
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    // Sprzątanie
    glDeleteVertexArrays(1, &helixVAO); glDeleteBuffers(1, &helixVBO);
    glDeleteVertexArrays(1, &sideVAO);  glDeleteBuffers(1, &sideVBO);
    glDeleteVertexArrays(1, &baseVAO);  glDeleteBuffers(1, &baseVBO);
    glDeleteProgram(prog);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
