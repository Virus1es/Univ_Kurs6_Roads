#pragma once

#include <algorithm> // Для std::max
#include <vector>
#include <cstdlib>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>
#include <limits>
#include <string>
#include <sstream>      // Для std::ostringstream
#include <iomanip> // Нужен для std::setprecision
#include <cmath>
#include "framework.h"
#include "Kursach.h"
#include "Car.h"
#include "TrafficLight.h"

using namespace std;

class TrafficSimulator {
private:
    int roadWidth, roadHeight, centerX, centerY;
    RECT repairZone;
    std::vector<Car> cars;
    TrafficLight leftTrafficLight;
    TrafficLight rightTrafficLight;

    std::mt19937 rng;
    std::exponential_distribution<> expDist1, expDist2;

    double timeUntilNextCarLeft;
    double timeUntilNextCarRight;

public:

    TrafficSimulator(int roadWidth, int roadHeight, int centerX, int centerY,
        RECT repairZone, RECT trafficLightLeft, RECT trafficLightRight, double greenDur, double redDur)
        : roadWidth(roadWidth), roadHeight(roadHeight), centerX(centerX), centerY(centerY),
        repairZone(repairZone),
        expDist1(1.0 / 12.0), expDist2(1.0 / 9.0),
        leftTrafficLight(greenDur, redDur), rightTrafficLight(greenDur, redDur),
        timeUntilNextCarLeft(expDist1(rng)), timeUntilNextCarRight(expDist2(rng)) 
    {
    }

    void updateTrafficLightDurations(double greenDur, double redDur) {
        leftTrafficLight.setDurations(greenDur, redDur);
        rightTrafficLight.setDurations(greenDur, redDur);
    }

    void update(double deltaTime) {
        // Обновляем состояние светофоров
        leftTrafficLight.update(deltaTime);
        rightTrafficLight.update(deltaTime);

        // Добавляем машины, если таймеры истекли
        timeUntilNextCarLeft -= deltaTime;
        timeUntilNextCarRight -= deltaTime;

        if (timeUntilNextCarLeft <= 0) {
            cars.emplace_back(centerX - roadWidth / 2, centerY - roadHeight / 4, 5, true, repairZone);
            timeUntilNextCarLeft = expDist1(rng);
        }
        if (timeUntilNextCarRight <= 0) {
            cars.emplace_back(centerX + roadWidth / 2, centerY + roadHeight / 4, 5, false, repairZone);
            timeUntilNextCarRight = expDist2(rng);
        }

        // Двигаем автомобили
        for (auto& car : cars) {
            LightState lightState = car.direction ? leftTrafficLight.getCurrentState() : rightTrafficLight.getCurrentState();
            
            // Передаем состояние светофора, centerY и ссылку на TrafficSimulator
            car.move(lightState, centerY, cars);
        }

        // Удаляем автомобили, которые выехали за пределы дороги
        cars.erase(
            std::remove_if(cars.begin(), cars.end(), [&](const Car& car) {
                return (car.direction && car.x > centerX + roadWidth / 2) ||
                    (!car.direction && car.x < centerX - roadWidth / 2) ||
                    (car.y > centerY + roadHeight / 2); // Условие для нижней дороги
                }),
            cars.end()
        );
    }

    void draw(HDC hdc) {
        // Рисуем ремонтную зону
        HBRUSH hBrushHatch = CreateHatchBrush(HS_BDIAGONAL, RGB(255, 0, 0));
        FillRect(hdc, &repairZone, hBrushHatch);
        DeleteObject(hBrushHatch);

        // Рисуем светофоры
        HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 0, 0));
        HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));

        RECT leftLightRect = { repairZone.left - 30, centerY - 20,
                               repairZone.left - 10, centerY + 20 };
        RECT rightLightRect = { repairZone.right + 10, centerY - 20,
                                repairZone.right + 30, centerY + 20 };

        FillRect(hdc, &leftLightRect,
            leftTrafficLight.getCurrentState() == GreenFirstDirection ? hBrushGreen : hBrushRed);
        FillRect(hdc, &rightLightRect,
            rightTrafficLight.getCurrentState() == GreenSecondDirection ? hBrushGreen : hBrushRed);

        DeleteObject(hBrushRed);
        DeleteObject(hBrushGreen);

        // Рисуем автомобили
        for (const auto& car : cars) {
            car.draw(hdc);
        }
    }

    // Функция для расчета среднего времени ожидания
    double calculateAverageWaitTime(double N1, double N2, double G1, double G2, double R) {
        double totalCycleTime = G1 + G2 + R; // Общее время цикла
        double waitTime1 = (N1 / (N1 + N2)) * (G1 + R / 2); // Среднее время ожидания для первого направления
        double waitTime2 = (N2 / (N1 + N2)) * (G2 + R / 2); // Среднее время ожидания для второго направления
        return waitTime1 + waitTime2; // Общее среднее время ожидания
    }

    // N1 - количество автомобилей, ожидающих проезда в первом направлении 
    // N2 - количество автомобилей, ожидающих проезда во втором направлении
    // R - время красного света
    string findOptimalGreenTimes(double N1, double N2, double R) {
        double minWaitTime = 1e9; // Инициализация с большим значением
        double optimalG1 = 0.0;
        double optimalG2 = 0.0;

        // Пробегаем значения G1 от 1 до 60 секунд
        for (double G1 = 1.0; G1 <= 120.0; G1 += 1.0) {
            // Пробегаем значения G2 от 1 до 60 секунд
            for (double G2 = 1.0; G2 <= 120.0; G2 += 1.0) {
                double waitTime = calculateAverageWaitTime(N1, N2, G1, G2, R);
                // Если текущее время ожидания меньше минимального, обновляем оптимальные значения
                if (waitTime < minWaitTime) {
                    minWaitTime = waitTime;
                    optimalG1 = G1;
                    optimalG2 = G2;
                }
            }
        }
        // Формируем строку с результатами
        ostringstream result;
        result << fixed << setprecision(2); // Устанавливаем фиксированное количество знаков после запятой
        result << "Оптимальное время зеленого света для первого направления: " << optimalG1 << " секунд; "
            << "Оптимальное время зеленого света для второго направления: " << optimalG2 << " секунд; "
            << "Минимальное среднее время ожидания: " << minWaitTime << " секунд";

        return result.str(); // Возвращаем строку с результатами
    }

};
