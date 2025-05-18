#define UNICODE
#define _UNICODE
#include <windows.h>
#include <windowsx.h>
#include <fstream>

#define IDM_FILE_EXIT 9001
#define IDM_HELP_ABOUT 9002

const wchar_t* WINDOW_STATE_FILE = L"window_state.ini";
const int TITLEBAR_HEIGHT = 32;
const int BUTTON_WIDTH = 45;

HFONT hTitleFont;

enum HoveredButton {
    HoverNone,
    HoverClose,
    HoverMax,
    HoverMin,
    HoverHamburger
};

HoveredButton hoveredBtn = HoverNone;

RECT hamburgerBtn;

void SaveWindowPlacement(HWND hwnd) {
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    if (GetWindowPlacement(hwnd, &wp)) {
        std::wofstream file(WINDOW_STATE_FILE);
        if (file.is_open()) {
            file << wp.showCmd << L" "
                << wp.rcNormalPosition.left << L" "
                << wp.rcNormalPosition.top << L" "
                << wp.rcNormalPosition.right << L" "
                << wp.rcNormalPosition.bottom << L"\n";
        }
    }
}

bool LoadWindowPlacement(HWND hwnd) {
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    std::wifstream file(WINDOW_STATE_FILE);
    if (file.is_open()) {
        file >> wp.showCmd
             >> wp.rcNormalPosition.left
             >> wp.rcNormalPosition.top
             >> wp.rcNormalPosition.right
             >> wp.rcNormalPosition.bottom;
        if (file.fail()) return false;
        if (wp.showCmd == SW_SHOWMINIMIZED) wp.showCmd = SW_SHOWNORMAL;
        return SetWindowPlacement(hwnd, &wp);
    }
    return false;
}

RECT closeBtn, maxBtn, minBtn;

void DrawTitleBar(HDC hdc, RECT clientRect, HWND hwnd) {
    RECT titleBarRect = clientRect;
    titleBarRect.bottom = titleBarRect.top + TITLEBAR_HEIGHT;

    HBRUSH titleBrush = CreateSolidBrush(RGB(230, 230, 230));
    FillRect(hdc, &titleBarRect, titleBrush);
    DeleteObject(titleBrush);

    HFONT marlettFont = CreateFont(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Marlett"
    );

    SetBkMode(hdc, TRANSPARENT);

    auto DrawHoverBackground = [&](RECT rect, HoveredButton btnType) {
        if (hoveredBtn == btnType) {
            HBRUSH hoverBrush = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &rect, hoverBrush);
            DeleteObject(hoverBrush);
        }
    };

    hamburgerBtn = { 10, 4, 10 + 24, 4 + 24 };

    DrawHoverBackground(hamburgerBtn, HoverHamburger);

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(20, 20, 20));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    int lineSpacing = 6;
    int startX = hamburgerBtn.left + 4;
    int endX = hamburgerBtn.right - 4;
    for (int i = 0; i < 3; ++i) {
        int y = hamburgerBtn.top + 4 + i * lineSpacing;
        MoveToEx(hdc, startX, y, NULL);
        LineTo(hdc, endX, y);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);

    SetTextColor(hdc, RGB(20, 20, 20));
    SelectObject(hdc, hTitleFont);
    TextOut(hdc, hamburgerBtn.right + 10, 8, L"aaToolbox Manager", 18);

    SelectObject(hdc, marlettFont);
    SetTextColor(hdc, RGB(20, 20, 20));

    closeBtn = { clientRect.right - BUTTON_WIDTH, 0, clientRect.right, TITLEBAR_HEIGHT };
    maxBtn   = { clientRect.right - 2 * BUTTON_WIDTH, 0, clientRect.right - BUTTON_WIDTH, TITLEBAR_HEIGHT };
    minBtn   = { clientRect.right - 3 * BUTTON_WIDTH, 0, clientRect.right - 2 * BUTTON_WIDTH, TITLEBAR_HEIGHT };

    DrawHoverBackground(closeBtn, HoverClose);
    DrawHoverBackground(maxBtn, HoverMax);
    DrawHoverBackground(minBtn, HoverMin);

    DrawText(hdc, L"r", 1, &closeBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, L"1", 1, &maxBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, L"0", 1, &minBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hTitleFont);
    DeleteObject(marlettFont);

    
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isMaximized = false;

    switch (uMsg) {
    case WM_NCHITTEST: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hwnd, &pt);

        if (pt.y < TITLEBAR_HEIGHT) {
            if (PtInRect(&closeBtn, pt) || PtInRect(&maxBtn, pt) || PtInRect(&minBtn, pt) || PtInRect(&hamburgerBtn, pt)) {
                return HTCLIENT;
            }
            return HTCAPTION;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    case WM_MOUSEMOVE: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        HoveredButton newHover = HoverNone;
        if (PtInRect(&closeBtn, pt)) newHover = HoverClose;
        else if (PtInRect(&maxBtn, pt)) newHover = HoverMax;
        else if (PtInRect(&minBtn, pt)) newHover = HoverMin;
        else if (PtInRect(&hamburgerBtn, pt)) newHover = HoverHamburger;

        if (newHover != hoveredBtn) {
            hoveredBtn = newHover;
            InvalidateRect(hwnd, NULL, TRUE);
        }

        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);

        return 0;
    }

    case WM_MOUSELEAVE: {
        if (hoveredBtn != HoverNone) {
            hoveredBtn = HoverNone;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        POINT pt = { x, y };

        if (PtInRect(&hamburgerBtn, pt)) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, IDM_FILE_EXIT, L"Exit");
            AppendMenu(hMenu, MF_STRING, IDM_HELP_ABOUT, L"About");

            POINT screenPt = pt;
            ClientToScreen(hwnd, &screenPt);

            int cmd = TrackPopupMenu(
                hMenu,
                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY,
                screenPt.x,
                screenPt.y,
                0,
                hwnd,
                NULL
            );

            if (cmd == IDM_FILE_EXIT) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            } else if (cmd == IDM_HELP_ABOUT) {
                MessageBox(hwnd, L"aaToolbox Manager v1.0\nCreated by Aubrey", L"About", MB_OK | MB_ICONINFORMATION);
            }

            DestroyMenu(hMenu);
        } 
        else if (PtInRect(&closeBtn, pt)) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        } 
        else if (PtInRect(&maxBtn, pt)) {
            if (isMaximized) {
                ShowWindow(hwnd, SW_RESTORE);
                isMaximized = false;
            } else {
                ShowWindow(hwnd, SW_MAXIMIZE);
                isMaximized = true;
            }
        } 
        else if (PtInRect(&minBtn, pt)) {
            ShowWindow(hwnd, SW_MINIMIZE);
        }

        return 0;
    }


    case WM_GETMINMAXINFO: {
        LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;

        mmi->ptMinTrackSize.x = 400;
        mmi->ptMinTrackSize.y = 300;

        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor) {
            MONITORINFO monitorInfo = { sizeof(monitorInfo) };
            if (GetMonitorInfo(hMonitor, &monitorInfo)) {
                RECT rcWork = monitorInfo.rcWork;
                RECT rcMonitor = monitorInfo.rcMonitor;

                mmi->ptMaxPosition.x = rcWork.left - rcMonitor.left;
                mmi->ptMaxPosition.y = rcWork.top - rcMonitor.top;
                mmi->ptMaxSize.x = rcWork.right - rcWork.left;
                mmi->ptMaxSize.y = rcWork.bottom - rcWork.top;
            }
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        case IDM_HELP_ABOUT:
            MessageBox(hwnd, L"aaToolbox Manager v1.0\nCreated by Aubrey", L"About", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        break;

    case WM_DESTROY:
        SaveWindowPlacement(hwnd);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        DrawTitleBar(hdc, clientRect, hwnd);

        COLORREF backgroundColor = RGB(245, 245, 245);

        RECT contentRect = clientRect;
        contentRect.top += TITLEBAR_HEIGHT;

        HBRUSH backgroundBrush = CreateSolidBrush(backgroundColor);
        FillRect(hdc, &contentRect, backgroundBrush);
        DeleteObject(backgroundBrush);

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"CustomWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    hTitleFont = CreateFont(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
    );

    HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        L"aaToolbox Manager",
        WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );



    if (!hwnd) return 0;

    if (!LoadWindowPlacement(hwnd)) {
        ShowWindow(hwnd, nCmdShow);
    } else {
        ShowWindow(hwnd, SW_SHOWDEFAULT);
    }
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hTitleFont);

    return 0;
}
