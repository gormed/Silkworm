

/*

    osabs.cpp

    operating system abstraction code

    (windows 32bit)

*/

#include <GL3/gl3w.h>

#include "project.h"
#include "osabs.h"
#include "log.h"


// window procedure

LRESULT CALLBACK windowproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// windows handles

HINSTANCE hins;
WNDCLASSEX wcex;
HWND hwnd;
HDC hDC;
HGLRC hRC;

// last known position of mouse cursor

int lastmx,lastmy;

// main

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    hins=hInstance;

    // initialize log
    Log::init();

    // main renderloop
    int errors = renderloop()?0:1;

    // deinitialize log
    Log::deinit();

    return errors;
}

// callback method
// displays back buffer

void frame_hook()
{
    SwapBuffers(hDC);
    Sleep (1);
}

// callback method
// handles operating system messages

int system_hook(int *quit)
{
    static MSG msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {

        if (msg.message == WM_QUIT)
        {
            *quit = true;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return true;

    }
    else
    {
        return false;
    }
}

// allocate an opengl context

int create_context()
{
    DEVMODE dm;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = windowproc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hins;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = ENGINE;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if(!RegisterClassEx(&wcex))
    {
        return 0;
    }

	memset(&dm,0,sizeof(DEVMODE));
	dm.dmSize       = sizeof(DEVMODE);
	dm.dmPelsWidth  = WINDOW_WIDTH;
	dm.dmPelsHeight = WINDOW_HEIGHT;
	dm.dmBitsPerPel = 32;
	dm.dmFields     = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

	//if (DISP_CHANGE_SUCCESSFUL!=ChangeDisplaySettings(&dm,CDS_FULLSCREEN)) return 0;

    /*hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          ENGINE,
                          PROJECT " " VERSION,
                          WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          WINDOW_WIDTH,
                           WINDOW_HEIGHT,
                          NULL,
                          NULL,
                          hins,
                          NULL);*/

    hwnd = CreateWindowEx(0,
                          ENGINE,
                          PROJECT " " VERSION,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          WINDOW_WIDTH,
                          WINDOW_HEIGHT,
                          NULL,
                          NULL,
                          hins,
                          NULL);

    ShowWindow(hwnd, true);


    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(hDC, &pfd);

    SetPixelFormat(hDC, iFormat, &pfd);

    hRC = wglCreateContext(hDC);

    wglMakeCurrent(hDC, hRC);

    return true;
}

// release the allocated opengl context

int kill_context()
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);

    DestroyWindow(hwnd);

    return true;
}

// window procedure
// gets mouse and keyboard input messages from os and routes to main application

LRESULT CALLBACK windowproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            return 0;

        case WM_SIZE:
            //windowsize(LOWORD(lParam),HIWORD(lParam));
            break;

        case WM_CHAR:
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
                default:
                    keydown(wParam+((int)'d'<<8));
                    break;
            }
            break;

        case 0x020A: // WM_MOUSEWEHEEL

            if (HIWORD(wParam)&0x8000) keydown((int)'m'+((int)'w'<<8)+((int)'b'<<16));
                                  else keydown((int)'m'+((int)'w'<<8)+((int)'f'<<16));
            break;

        case WM_KEYUP:
            keydown(wParam+((int)'u'<<8));
            break;

        case WM_LBUTTONDOWN:
            keydown((int)'m'+((int)'l'<<8)+((int)'d'<<16));
            break;

        case WM_LBUTTONUP:
            keydown((int)'m'+((int)'l'<<8)+((int)'u'<<16));
            break;

        case WM_RBUTTONDOWN:
            keydown((int)'m'+((int)'r'<<8)+((int)'d'<<16));
            break;

        case WM_RBUTTONUP:
            keydown((int)'m'+((int)'r'<<8)+((int)'u'<<16));
            break;

        case WM_MOUSEMOVE:
            mousemove(LOWORD(lParam),HIWORD(lParam));
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

