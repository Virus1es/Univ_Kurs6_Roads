#include "framework.h"
#include "Kursach.h"
#include "TrafficSimulator.h"
#include <thread>
#include <mutex>
#include <atomic>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
TrafficSimulator* trafficSimulator;             // имитация потока машин
std::thread simulationThread;                   // Поток для обновления симуляции
std::mutex trafficMutex;                        // Мьютекс для синхронизации
std::atomic<bool> stopSimulation(false);        // Флаг завершения потока
std::thread calculationThread;

int roadWidth, roadHeight, centerX, centerY;

RECT repairZone, trafficLightLeft, trafficLightRight;

double greenDuration = 40.0;
double redDuration = 55.0;



// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void RenderScene(HDC hdc, HWND hWnd);     // Новая функция для отрисовки
void RestartSimulation();                 // перезапуск симуляции
INT_PTR CALLBACK ResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

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

void SimulationLoop() {
    while (!stopSimulation) {
        {
            std::lock_guard<std::mutex> lock(trafficMutex);
            trafficSimulator->update(2.0); // Обновляем симуляцию каждые 2 секунды
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Интервал обновления
    }
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

    roadWidth = rect.right - 100;
    roadHeight = 300;
    centerX = rect.right / 2;
    centerY = rect.bottom / 2;

    repairZone = { centerX - roadWidth / 6, centerY - roadHeight / 2,
                        centerX + roadWidth / 6, centerY };
    trafficLightLeft = { repairZone.left - 30, centerY - 20,
                              repairZone.left - 10, centerY + 20 };
    trafficLightRight = { repairZone.right + 10, centerY - 20,
                               repairZone.right + 30, centerY + 20 };

    trafficSimulator = new TrafficSimulator(roadWidth, roadHeight, 
                                            centerX, centerY, 
                                            repairZone, 
                                            trafficLightLeft, trafficLightRight, 
                                            greenDuration, redDuration);

    simulationThread = std::thread(SimulationLoop);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

INT_PTR CALLBACK LightDurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static double* pGreenDuration;
    static double* pRedDuration;

    switch (message) {
    case WM_INITDIALOG:
        pGreenDuration = reinterpret_cast<double*>(lParam);
        pRedDuration = pGreenDuration + 1;

        // Установите текущие значения в текстовые поля
        SetDlgItemInt(hDlg, IDC_GREEN_DURATION, static_cast<UINT>(*pGreenDuration), FALSE);
        SetDlgItemInt(hDlg, IDC_RED_DURATION, static_cast<UINT>(*pRedDuration), FALSE);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            BOOL greenValid, redValid;
            UINT greenValue = GetDlgItemInt(hDlg, IDC_GREEN_DURATION, &greenValid, FALSE);
            UINT redValue = GetDlgItemInt(hDlg, IDC_RED_DURATION, &redValid, FALSE);

            if (greenValid && redValid) {
                *pGreenDuration = greenValue;
                *pRedDuration = redValue;
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"Введите корректные значения.", L"Ошибка", MB_OK | MB_ICONERROR);
            }
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static std::string* resultText;

    switch (message) {
    case WM_INITDIALOG:
        resultText = reinterpret_cast<std::string*>(lParam);
        SetDlgItemTextA(hDlg, IDC_RESULT_TEXT, resultText->c_str());
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK InputValuesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static double* inputValues;

    switch (message) {
    case WM_INITDIALOG:
        inputValues = reinterpret_cast<double*>(lParam);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            // Получаем значения из текстовых полей
            char buffer[256];
            GetDlgItemTextA(hDlg, IDC_VALUEN1, buffer, sizeof(buffer));
            inputValues[0] = std::stod(buffer); // Преобразуем строку в double

            GetDlgItemTextA(hDlg, IDC_VALUEN2, buffer, sizeof(buffer));
            inputValues[1] = std::stod(buffer);

            GetDlgItemTextA(hDlg, IDC_VALUER, buffer, sizeof(buffer));
            inputValues[2] = std::stod(buffer);

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Вызов диалога
void OpenLightDurationDialog(HWND hWnd) {
    double durations[2] = { greenDuration, redDuration };

    if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LIGHT_DURATION), hWnd, LightDurationDlgProc, reinterpret_cast<LPARAM>(durations)) == IDOK) {
        greenDuration = durations[0];
        redDuration = durations[1];

        // Обновляем TrafficSimulator с новыми значениями
        trafficSimulator->updateTrafficLightDurations(greenDuration, redDuration);

        MessageBox(hWnd, L"Настройки времени светофора обновлены.", L"Успех", MB_OK | MB_ICONINFORMATION);
    }
}

void CalculateAndShowResults(double N1, double N2, double R) {
    // Выполняем расчеты
    std::string result = trafficSimulator->findOptimalGreenTimes(N1, N2, R);

    // Создаем новый объект std::string на куче
    std::string* resultPtr = new std::string(result);

    // Отобразите результаты в диалоговом окне
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RESULT_DIALOG), NULL, ResultDlgProc, reinterpret_cast<LPARAM>(resultPtr));
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
        case IDC_RESTART_BUTTON: // Обработчик для кнопки перезапуска
            RestartSimulation();
            break;
        case IDD_LIGHT_DURATION: // обработчик для вызова диалога
            OpenLightDurationDialog(hWnd);
            break;
        case IDC_CALCULATE_BUTTON: // Обработчик для кнопки запуска расчетов
        {
            // Открываем диалог для ввода значений
            double inputValues[3] = { 0.0, 0.0, 0.0 };
            if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INPUT_VALUES), hWnd, InputValuesDlgProc, reinterpret_cast<LPARAM>(inputValues)) == IDOK) {
                // Если пользователь нажал OK, создаем поток для выполнения расчетов
                std::thread calculationThread([inputValues]() {
                    try {
                        CalculateAndShowResults(inputValues[0], inputValues[1], inputValues[2]);
                    }
                    catch (const std::exception& e) {
                        // Обработка исключения, например, логирование
                        std::cerr << "Ошибка в потоке расчетов: " << e.what() << std::endl;
                    }
                    });
                calculationThread.detach(); // Отсоединяем поток, чтобы он работал независимо
            }
        }
        break;
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
        stopSimulation = true; // Устанавливаем флаг остановки
        if (simulationThread.joinable()) {
            simulationThread.join(); // Ожидаем завершения потока
        }
        delete trafficSimulator;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// перезапуск симуляции
void RestartSimulation() {

    // Остановить текущую симуляцию
    stopSimulation = true;
    if (simulationThread.joinable()) {
        simulationThread.join(); // Ожидаем завершения потока
    }

    // Создать новый экземпляр TrafficSimulator с текущими значениями
    trafficSimulator = new TrafficSimulator(roadWidth, roadHeight,
        centerX, centerY,
        repairZone,
        trafficLightLeft, trafficLightRight,
        greenDuration, redDuration);

    // Запустить новый поток симуляции
    stopSimulation = false;
    simulationThread = std::thread(SimulationLoop);

    MessageBox(NULL, L"Симуляция перезапущена.", L"Успех", MB_OK | MB_ICONINFORMATION);
}

void RenderScene(HDC hdc, HWND hWnd) {
    std::lock_guard<std::mutex> lock(trafficMutex); // Блокируем доступ к данным
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