#include "framework.h"
#include "Kursach.h"
#include "TrafficSimulator.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
TrafficSimulator* trafficSimulator;             // имитация потока машин

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void RenderScene(HDC hdc, HWND hWnd);     // Новая функция для отрисовки

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KURSACH, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KURSACH));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KURSACH));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KURSACH);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    RECT rect;
    GetClientRect(hWnd, &rect);

    int roadWidth = rect.right - 100;
    int roadHeight = 300;
    int centerX = rect.right / 2;
    int centerY = rect.bottom / 2;

    RECT repairZone = { centerX - roadWidth / 6, centerY - roadHeight / 2,
                        centerX + roadWidth / 6, centerY };
    RECT trafficLightLeft = { repairZone.left - 30, centerY - 20,
                              repairZone.left - 10, centerY + 20 };
    RECT trafficLightRight = { repairZone.right + 10, centerY - 20,
                               repairZone.right + 30, centerY + 20 };

    trafficSimulator = new TrafficSimulator(roadWidth, roadHeight, centerX, centerY, repairZone, trafficLightLeft, trafficLightRight);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE: // Устанавливаем таймер в WM_CREATE
        SetTimer(hWnd, 1, 500, NULL); // Устанавливаем таймер с интервалом 2000 мс
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RenderScene(hdc, hWnd);  // Передаём HDC и HWND
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
        if (wParam == 1) {
            trafficSimulator->update(2.0); // 2.0 секунды (интервал таймера)
            InvalidateRect(hWnd, NULL, TRUE); // Перерисовка окна
        }
        break;
    case WM_DESTROY:
        delete trafficSimulator;
        KillTimer(hWnd, 1); // Удаляем таймер перед выходом
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RenderScene(HDC hdc, HWND hWnd)
{
    HBRUSH hBrushGray = CreateSolidBrush(RGB(169, 169, 169));
    HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 0, 0));
    HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH hBrushHatch = CreateHatchBrush(HS_BDIAGONAL, RGB(255, 0, 0));
    HPEN hPenWhite = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

    RECT rect;
    GetClientRect(hWnd, &rect);  // Используем HWND для получения размеров окна

    int roadWidth = rect.right - 100;
    int roadHeight = 300;
    int centerX = rect.right / 2;
    int centerY = rect.bottom / 2;

    // Рисуем дорогу
    RECT road = { centerX - roadWidth / 2, centerY - roadHeight / 2,
                  centerX + roadWidth / 2, centerY + roadHeight / 2 };
    FillRect(hdc, &road, hBrushGray);

    // Разделительная линия
    SelectObject(hdc, hPenWhite);
    MoveToEx(hdc, road.left, centerY, NULL);
    LineTo(hdc, road.right, centerY);

    // Ремонтная зона
    RECT repairZone = { centerX - roadWidth / 6, centerY - roadHeight / 2,
                        centerX + roadWidth / 6, centerY };
    FillRect(hdc, &repairZone, hBrushHatch);

    // Светофоры
    RECT leftTrafficLight = { repairZone.left - 30, centerY - 20,
                              repairZone.left - 10, centerY + 20 };
    RECT rightTrafficLight = { repairZone.right + 10, centerY - 20,
                               repairZone.right + 30, centerY + 20 };

    FillRect(hdc, &leftTrafficLight, hBrushRed);
    FillRect(hdc, &rightTrafficLight, hBrushRed);

    // Отрисовка потока машин
    trafficSimulator->draw(hdc);

    // Очистка ресурсов
    DeleteObject(hBrushGray);
    DeleteObject(hBrushRed);
    DeleteObject(hBrushGreen);
    DeleteObject(hBrushHatch);
    DeleteObject(hPenWhite);
}