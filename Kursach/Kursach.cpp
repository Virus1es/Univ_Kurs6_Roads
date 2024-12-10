// Kursach.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Kursach.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KURSACH, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
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

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KURSACH));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_KURSACH);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
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
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Кисти и перья
        HBRUSH hBrushGray = CreateSolidBrush(RGB(169, 169, 169)); // Серый цвет для дороги
        HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 0, 0));      // Красный цвет светофоров
        HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));    // Зелёный цвет светофоров
        HBRUSH hBrushHatch = CreateHatchBrush(HS_BDIAGONAL, RGB(255, 0, 0)); // Полосатый для ремонта
        HPEN hPenWhite = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));          // Белый для разметки

        // Координаты элементов дороги
        int roadWidth = ps.rcPaint.right - 100; // Полная ширина дороги
        int roadHeight = 300;                  // Высота секции дороги
        int centerX = ps.rcPaint.right / 2;    // Центр окна
        int centerY = ps.rcPaint.bottom / 2;   // Центр окна

        // Рисуем дорогу
        RECT road = { centerX - roadWidth / 2, centerY - roadHeight / 2,
                      centerX + roadWidth / 2, centerY + roadHeight / 2 };
        FillRect(hdc, &road, hBrushGray);

        // Рисуем разделительную линию между полосами
        SelectObject(hdc, hPenWhite);
        MoveToEx(hdc, road.left, centerY, NULL);
        LineTo(hdc, road.right, centerY);

        // Рисуем ремонтную зону
        int repairZoneWidth = roadWidth / 3; // Ширина ремонтной зоны (например, треть дороги)
        RECT repairZone = { centerX - repairZoneWidth / 2, centerY - roadHeight / 2,
                            centerX + repairZoneWidth / 2, centerY };
        FillRect(hdc, &repairZone, hBrushHatch);

        // Рисуем светофоры
        int lightSize = 20; // Размер светофоров
        RECT leftTrafficLight = { repairZone.left - lightSize - 10, centerY - lightSize - 10,
                                  repairZone.left - 10, centerY + lightSize - 10 };
        RECT rightTrafficLight = { repairZone.right + 10, centerY - lightSize - 10,
                                   repairZone.right + lightSize + 10, centerY + lightSize - 10 };

        // Устанавливаем сигнал светофоров (красный)
        FillRect(hdc, &leftTrafficLight, hBrushRed);
        FillRect(hdc, &rightTrafficLight, hBrushRed);

        // Очистка ресурсов
        DeleteObject(hBrushGray);
        DeleteObject(hBrushRed);
        DeleteObject(hBrushGreen);
        DeleteObject(hBrushHatch);
        DeleteObject(hPenWhite);

        EndPaint(hWnd, &ps);
    }
    break;
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}