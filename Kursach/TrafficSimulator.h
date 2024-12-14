#pragma once

#include <vector>
#include <cstdlib>
#include <random>
#include <thread>
#include <mutex>
#include "framework.h"
#include "Kursach.h"
#include "Car.h"
#include "TrafficLight.h"

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
        RECT repairZone, RECT trafficLightLeft, RECT trafficLightRight)
        : roadWidth(roadWidth), roadHeight(roadHeight), centerX(centerX), centerY(centerY),
        repairZone(repairZone),
        expDist1(1.0 / 12.0), expDist2(1.0 / 9.0),
        timeUntilNextCarLeft(expDist1(rng)), timeUntilNextCarRight(expDist2(rng)) {
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
};
