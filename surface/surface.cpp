//
// Created by Khang on 8/23/2025.
//

#include "surface.h"
#include <stdio.h>

const char g_szClassName[] = "qcrWindowClass";

LRESULT CALLBACK MSG_CALLBACK(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    xcSurface *ptr;
    if (msg == WM_CREATE) // upon window creation, bind window state with window handle
    {
        CREATESTRUCTW* createStructW = (CREATESTRUCTW*)lparam;
        ptr = (xcSurface*)createStructW->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)ptr);
    } else // get window state pointer for every event
    {
        ptr = (xcSurface*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }

    switch(msg) {
    case WM_LBUTTONDOWN:
        {
            char szFileName[MAX_PATH];
            GetModuleFileName(GetModuleHandle(NULL), szFileName, MAX_PATH);
            printf("this program is: %s\n", szFileName);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        // Draw something here...
        break;
    case WM_SIZE:
        {
            ptr->width = LOWORD(lparam);
            ptr->height = HIWORD(lparam);
            printf("hwnd wxh: %dx%d\n", ptr->width, ptr->height);
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

bool xcSurface::init() {
	WNDCLASSEX wc;

    hInstance = GetModuleHandle(NULL);

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = MSG_CALLBACK;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;

    wc.hIcon  = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_XC_ICON));
    wc.hIconSm  = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_XC_ICON), IMAGE_ICON, 16, 16, 0);

    if(!RegisterClassEx(&wc))
    {
        printf("Window Registration Failed! Error!");
        return false;
    }

    // Step 2: Creating the Window
    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "xc",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
        NULL, NULL, hInstance, this);

    if(hWnd == NULL) {
        printf("Window Creation Failed! Error!");
        return false;
    }

    ShowWindow(hWnd, SW_NORMAL);
    UpdateWindow(hWnd);

    return true;
}

void xcSurface::run() {
	// Step 3: The Message Loop
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("msg.wParam: %llu\n", msg.wParam);

    if (msg.wParam == 0) printf("window closed\n");

    return;
}

void xcSurface::cleanup() {
	DestroyWindow(hWnd);
}