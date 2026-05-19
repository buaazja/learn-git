#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <atomic>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msimg32.lib")

const char g_szClassName[] = "EarthWindow";
HINSTANCE hInst;
HWND hWnd;
HDC hDC;
HGLRC hRC;
std::atomic<float> earthRotation(0.0f);
GLuint earthTexture = 0;
GLuint starTexture = 0;

struct Star {
    float x, y, z;
    float size;
    float brightness;
    float twinkleSpeed;
    float hue;
};

struct Nebula {
    float x, y, z;
    float size;
    float r, g, b;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

Star stars[300];
Nebula nebulae[5];
int shootingStarTimer = 0;
float shootingStarX = 0, shootingStarY = 0, shootingStarZ = 0;
float shootingStarVelX = 0, shootingStarVelY = 0;
bool showShootingStar = false;

void gluPerspective(float fovY, float aspect, float zNear, float zFar) {
    float f = 1.0f / (float)tan(fovY * 3.14159f / 360.0f);
    float m[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zFar + zNear) / (zNear - zFar), -1,
        0, 0, (2 * zFar * zNear) / (zNear - zFar), 0
    };
    glMultMatrixf(m);
}

void gluLookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ) {
    float forward[3] = { centerX - eyeX, centerY - eyeY, centerZ - eyeZ };
    float len = sqrt(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
    forward[0] /= len; forward[1] /= len; forward[2] /= len;

    float side[3] = {
        forward[1]*upZ - forward[2]*upY,
        forward[2]*upX - forward[0]*upZ,
        forward[0]*upY - forward[1]*upX
    };
    len = sqrt(side[0]*side[0] + side[1]*side[1] + side[2]*side[2]);
    side[0] /= len; side[1] /= len; side[2] /= len;

    float up[3] = {
        side[1]*forward[2] - side[2]*forward[1],
        side[2]*forward[0] - side[0]*forward[2],
        side[0]*forward[1] - side[1]*forward[0]
    };

    float m[16] = {
        side[0], up[0], -forward[0], 0,
        side[1], up[1], -forward[1], 0,
        side[2], up[2], -forward[2], 0,
        -(side[0]*eyeX + side[1]*eyeY + side[2]*eyeZ),
        -(up[0]*eyeX + up[1]*eyeY + up[2]*eyeZ),
        (forward[0]*eyeX + forward[1]*eyeY + forward[2]*eyeZ),
        1
    };
    glMultMatrixf(m);
}

HBITMAP LoadBitmapFromFile(const char* filename) {
    return (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

GLuint CreateTextureFromBitmap(HBITMAP hBitmap) {
    if (!hBitmap) return 0;

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    unsigned char* data = new unsigned char[bmp.bmWidth * bmp.bmHeight * 4];
    BITMAPINFO bi = {sizeof(BITMAPINFO), bmp.bmWidth, -bmp.bmHeight, 1, 32, BI_RGB, 0, 0, 0, 0, 0};
    HDC hScreenDC = GetDC(NULL);
    GetDIBits(hScreenDC, hBitmap, 0, bmp.bmHeight, data, &bi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hScreenDC);

    for (int i = 0; i < bmp.bmWidth * bmp.bmHeight; i++) {
        unsigned char temp = data[i*4];
        data[i*4] = data[i*4+2];
        data[i*4+2] = temp;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    delete[] data;
    DeleteObject(hBitmap);
    return texture;
}

GLuint CreateProceduralEarthTexture() {
    unsigned char* data = new unsigned char[512 * 256 * 3];

    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 512; x++) {
            int idx = (y * 512 + x) * 3;
            float lat = (float)y / 256.0f * 180.0f - 90.0f;
            float lon = (float)x / 512.0f * 360.0f - 180.0f;

            float noise = (float)(rand() % 100) / 100.0f * 0.3f;
            float landProb = sin(lat * 3.14159f / 180.0f) * 0.5f + 0.5f;
            landProb += noise;

            if (landProb > 0.55f) {
                if (abs(lat) < 15.0f) {
                    data[idx] = 34; data[idx+1] = 139; data[idx+2] = 34;
                } else if (abs(lat) > 60.0f) {
                    data[idx] = 255; data[idx+1] = 250; data[idx+2] = 250;
                } else {
                    data[idx] = 0; data[idx+1] = 100; data[idx+2] = 0;
                }
            } else {
                if (abs(lat) > 65.0f) {
                    data[idx] = 255; data[idx+1] = 250; data[idx+2] = 250;
                } else {
                    data[idx] = 10; data[idx+1] = 50; data[idx+2] = 150;
                }
            }
        }
    }

    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 512; x++) {
            int idx = (y * 512 + x) * 3;
            float lat = (float)y / 256.0f * 180.0f - 90.0f;
            if (abs(lat) < 5.0f) {
                float swirl = sin((float)x / 512.0f * 20.0f) * 0.5f + 0.5f;
                data[idx] = (unsigned char)(data[idx] * (1 - swirl * 0.3f));
                data[idx+1] = (unsigned char)(data[idx+1] * (1 - swirl * 0.3f) + 30 * swirl);
                data[idx+2] = (unsigned char)(data[idx+2] + 20 * swirl);
            }
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    delete[] data;
    return texture;
}

void initStars() {
    for (int i = 0; i < 300; i++) {
        stars[i].x = (float)(rand() % 3000 - 1500) / 100.0f;
        stars[i].y = (float)(rand() % 1500 - 750) / 100.0f;
        stars[i].z = -8.0f + (float)(rand() % 200) / 100.0f;
        stars[i].size = 0.5f + (float)(rand() % 100) / 100.0f * 2.0f;
        stars[i].brightness = 0.5f + (float)(rand() % 100) / 100.0f * 0.5f;
        stars[i].twinkleSpeed = 1.0f + (float)(rand() % 100) / 100.0f * 3.0f;
        stars[i].hue = (float)(rand() % 360);
    }
}

void initNebulae() {
    for (int i = 0; i < 5; i++) {
        nebulae[i].x = (float)(rand() % 2000 - 1000) / 80.0f;
        nebulae[i].y = (float)(rand() % 1200 - 600) / 80.0f;
        nebulae[i].z = -6.0f;
        nebulae[i].size = 3.0f + (float)(rand() % 100) / 100.0f * 5.0f;

        float hue = (float)(rand() % 360) / 360.0f;
        nebulae[i].r = 0.5f + 0.5f * sin(hue * 6.28f);
        nebulae[i].g = 0.5f + 0.5f * sin(hue * 6.28f + 2.09f);
        nebulae[i].b = 0.5f + 0.5f * sin(hue * 6.28f + 4.18f);
    }
}

void drawNebula() {
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);

    for (int i = 0; i < 5; i++) {
        Nebula& n = nebulae[i];

        glColor4f(n.r * 0.15f, n.g * 0.15f, n.b * 0.2f, 0.1f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(n.x, n.y, n.z);
        for (int j = 0; j <= 20; j++) {
            float angle = (float)j / 20.0f * 6.28f;
            glVertex3f(n.x + cos(angle) * n.size, n.y + sin(angle) * n.size * 0.6f, n.z);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void drawStars() {
    glDisable(GL_LIGHTING);
    glDepthMask(FALSE);

    drawNebula();

    static float time = 0;
    time += 0.016f;

    for (int i = 0; i < 300; i++) {
        float twinkle = sin(time * stars[i].twinkleSpeed) * 0.3f + 0.7f;
        float brightness = stars[i].brightness * twinkle;

        float hue = stars[i].hue;
        float r = 1.0f;
        float g = 0.9f + 0.1f * sin(hue * 0.017f);
        float b = 0.8f + 0.2f * sin(hue * 0.017f + 1.0f);

        glPointSize(stars[i].size * twinkle);
        glColor3f(r * brightness, g * brightness, b * brightness);
        glBegin(GL_POINTS);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
        glEnd();

        if (stars[i].size > 1.5f) {
            glPointSize(stars[i].size * 0.5f);
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_POINTS);
            glVertex3f(stars[i].x, stars[i].y, stars[i].z);
            glEnd();
        }
    }

    glDepthMask(TRUE);
    glEnable(GL_LIGHTING);
}

void updateShootingStar() {
    if (!showShootingStar) {
        shootingStarTimer++;
        if (shootingStarTimer > 300) {
            showShootingStar = true;
            shootingStarTimer = 0;
            shootingStarX = (float)(rand() % 800 - 400) / 50.0f;
            shootingStarY = (float)(rand() % 400 + 100) / 50.0f;
            shootingStarZ = -5.0f;
            shootingStarVelX = (float)(rand() % 100 + 50) / 100.0f;
            shootingStarVelY = -(float)(rand() % 100 + 50) / 100.0f;
        }
    } else {
        shootingStarX += shootingStarVelX;
        shootingStarY += shootingStarVelY;

        glDisable(GL_LIGHTING);
        glDepthMask(FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glLineWidth(2.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_LINES);
        glVertex3f(shootingStarX, shootingStarY, shootingStarZ);
        glVertex3f(shootingStarX - shootingStarVelX * 3, shootingStarY - shootingStarVelY * 3, shootingStarZ);
        glEnd();

        glPointSize(3.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POINTS);
        glVertex3f(shootingStarX, shootingStarY, shootingStarZ);
        glEnd();

        glDepthMask(TRUE);
        glEnable(GL_LIGHTING);

        if (shootingStarY < -5.0f) {
            showShootingStar = false;
            shootingStarTimer = 0;
        }
    }
}

void drawTexturedSphere(float radius, int segments, int rings) {
    for (int ring = 0; ring < rings; ring++) {
        float theta1 = ((float)ring / rings) * 3.14159f;
        float theta2 = ((float)(ring + 1) / rings) * 3.14159f;
        float v1 = (float)ring / rings;
        float v2 = (float)(ring + 1) / rings;

        for (int seg = 0; seg < segments; seg++) {
            float phi1 = ((float)seg / segments) * 2.0f * 3.14159f;
            float phi2 = ((float)(seg + 1) / segments) * 2.0f * 3.14159f;
            float u1 = (float)seg / segments;
            float u2 = (float)(seg + 1) / segments;

            float x1 = radius * sin(theta1) * cos(phi1);
            float y1 = radius * cos(theta1);
            float z1 = radius * sin(theta1) * sin(phi1);

            float x2 = radius * sin(theta1) * cos(phi2);
            float y2 = radius * cos(theta1);
            float z2 = radius * sin(theta1) * sin(phi2);

            float x3 = radius * sin(theta2) * cos(phi2);
            float y3 = radius * cos(theta2);
            float z3 = radius * sin(theta2) * sin(phi2);

            float x4 = radius * sin(theta2) * cos(phi1);
            float y4 = radius * cos(theta2);
            float z4 = radius * sin(theta2) * sin(phi1);

            glBegin(GL_QUADS);
            glNormal3f(x1 / radius, y1 / radius, z1 / radius);
            glTexCoord2f(u1, v1);
            glVertex3f(x1, y1, z1);

            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glTexCoord2f(u2, v1);
            glVertex3f(x2, y2, z2);

            glNormal3f(x3 / radius, y3 / radius, z3 / radius);
            glTexCoord2f(u2, v2);
            glVertex3f(x3, y3, z3);

            glNormal3f(x4 / radius, y4 / radius, z4 / radius);
            glTexCoord2f(u1, v2);
            glVertex3f(x4, y4, z4);
            glEnd();
        }
    }
}

void drawEarth() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, earthTexture);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawTexturedSphere(1.0f, 48, 32);

    glDisable(GL_TEXTURE_2D);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.02f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    GLfloat lightPos0[] = { 5.0f, 5.0f, 5.0f, 0.0f };
    GLfloat lightAmbient0[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse0[] = { 1.0f, 0.95f, 0.9f, 1.0f };
    GLfloat lightSpecular0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);

    GLfloat lightPos1[] = { -3.0f, -2.0f, -3.0f, 0.0f };
    GLfloat lightAmbient1[] = { 0.1f, 0.15f, 0.3f, 1.0f };
    GLfloat lightDiffuse1[] = { 0.2f, 0.3f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);

    GLfloat lightPos2[] = { 0.0f, 3.0f, 0.0f, 0.0f };
    GLfloat lightAmbient2[] = { 0.15f, 0.1f, 0.05f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbient2);

    GLfloat matAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 30.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    drawStars();
    updateShootingStar();

    glRotatef(earthRotation.load(std::memory_order_relaxed) * 0.5f, 0.0f, 1.0f, 0.0f);
    drawEarth();

    SwapBuffers(hDC);
}

BOOL setupPixelFormat(HDC hDC) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        32,
        0, 0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) return FALSE;
    return SetPixelFormat(hDC, pixelFormat, &pfd);
}

DWORD WINAPI rotationThread(LPVOID lpParam) {
    while (true) {
        float cur = earthRotation.load(std::memory_order_relaxed);
        float next = cur + 0.5f;
        if (next > 360.0f) next -= 360.0f;
        earthRotation.store(next, std::memory_order_relaxed);
        Sleep(16);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEX wc;
    MSG Msg;
    srand((unsigned int)time(NULL));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Earth with Terrain - Starry Sky",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    hDC = GetDC(hWnd);
    if (!setupPixelFormat(hDC)) {
        MessageBox(NULL, "Setup Pixel Format Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    earthTexture = CreateProceduralEarthTexture();
    initStars();
    initNebulae();

    HANDLE hThread = CreateThread(NULL, 0, rotationThread, NULL, 0, NULL);

    while (true) {
        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
            if (Msg.message == WM_QUIT) break;
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        renderScene();
    }

    CloseHandle(hThread);
    glDeleteTextures(1, &earthTexture);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    DestroyWindow(hWnd);

    return Msg.wParam;
}
