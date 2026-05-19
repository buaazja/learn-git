#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

const char g_szClassName[] = "EarthWindow";
HINSTANCE hInst;
HWND hWnd;
HDC hDC;
HGLRC hRC;
float earthRotation = 0.0f;

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

void drawStars() {
    glPointSize(1.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        float x = (float)(rand() % 2000 - 1000) / 100.0f;
        float y = (float)(rand() % 1200 - 600) / 100.0f;
        float z = -5.0f + (float)(rand() % 100) / 50.0f;
        glVertex3f(x, y, z);
    }
    glEnd();
}

void drawSphere(float radius, int segments, int rings) {
    for (int ring = 0; ring < rings; ring++) {
        float theta1 = ((float)ring / rings) * 3.14159f;
        float theta2 = ((float)(ring + 1) / rings) * 3.14159f;

        for (int seg = 0; seg < segments; seg++) {
            float phi1 = ((float)seg / segments) * 2.0f * 3.14159f;
            float phi2 = ((float)(seg + 1) / segments) * 2.0f * 3.14159f;

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
            glVertex3f(x1, y1, z1);
            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glVertex3f(x2, y2, z2);
            glNormal3f(x3 / radius, y3 / radius, z3 / radius);
            glVertex3f(x3, y3, z3);
            glNormal3f(x4 / radius, y4 / radius, z4 / radius);
            glVertex3f(x4, y4, z4);
            glEnd();
        }
    }
}

void drawEarth() {
    glColor3f(0.2f, 0.5f, 0.9f);
    drawSphere(1.0f, 24, 18);

    glColor3f(0.1f, 0.4f, 0.1f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.3f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    float h = 0.4f;
    float r = 0.15f;
    float segs = 12;
    for (int i = 0; i < (int)segs; i++) {
        float theta1 = ((float)i / segs) * 2.0f * 3.14159f;
        float theta2 = ((float)(i + 1) / segs) * 2.0f * 3.14159f;
        glVertex3f(r * cos(theta1), -h/2, r * sin(theta1));
        glVertex3f(r * cos(theta2), -h/2, r * sin(theta2));
        glVertex3f(r * cos(theta2), h/2, r * sin(theta2));
        glVertex3f(r * cos(theta1), h/2, r * sin(theta1));
    }
    glEnd();
    glPopMatrix();

    glColor3f(0.1f, 0.3f, 0.1f);
    glPushMatrix();
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.5f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    h = 0.35f;
    r = 0.12f;
    for (int i = 0; i < (int)segs; i++) {
        float theta1 = ((float)i / segs) * 2.0f * 3.14159f;
        float theta2 = ((float)(i + 1) / segs) * 2.0f * 3.14159f;
        glVertex3f(r * cos(theta1), -h/2, r * sin(theta1));
        glVertex3f(r * cos(theta2), -h/2, r * sin(theta2));
        glVertex3f(r * cos(theta2), h/2, r * sin(theta2));
        glVertex3f(r * cos(theta1), h/2, r * sin(theta1));
    }
    glEnd();
    glPopMatrix();
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 5.0f, 5.0f, 5.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

    glDisable(GL_LIGHTING);
    drawStars();
    glEnable(GL_LIGHTING);

    glRotatef(earthRotation * 0.5f, 0.0f, 1.0f, 0.0f);
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
        earthRotation += 1.0f;
        if (earthRotation > 360.0f) earthRotation -= 360.0f;
        Sleep(16);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    WNDCLASSEX wc;
    MSG Msg;

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
        "Rotating Earth with Stars",
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
    setupPixelFormat(hDC);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

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
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    DestroyWindow(hWnd);

    return Msg.wParam;
}
