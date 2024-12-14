#pragma once
#include "TrafficLight.h"
#include "TrafficSimulator.h"

class Car {
public:
    int x, y;          // Координаты автомобиля
    int speed;         // Скорость автомобиля
    bool direction;    // Направление: true - вправо, false - влево
    RECT repairZone;   // Зона ремонта

    Car(int x, int y, int speed, bool direction, RECT repairZone)
        : x(x), y(y), speed(speed*4), direction(direction), repairZone(repairZone) {
    }

    // Двигать машину
    void move(LightState lightState, int centerY, const std::vector<Car>& cars) {
        for (const auto& otherCar : cars) {
            if (&otherCar != this && isTooClose(otherCar)) {
                return; // Если машина слишком близко к другой, не двигаемся
            }
        }

        if (direction) { // Движение вправо
            if (y > centerY || x > repairZone.right + 50) {
                x += speed;
            }
            else if (canMove(lightState) && y < centerY && x < repairZone.left - 150) {
                x += speed;
            }
            if (y < centerY && lightState == GreenFirstDirection && x >= repairZone.left - 150) {
                y += 130;
            }
            if (y > centerY && x > repairZone.right + 50) {
                y -= 130;
            }
        }
        else { // Движение влево
            if (isInRepairZone()) {
                x -= speed;
            }
            else if (canMove(lightState) && y > centerY && x > repairZone.right + 150) {
                x -= speed;
            }
            else if (x <= repairZone.right + 200) {
                x -= speed;
            }
        }
    }

    // Нарисовать машину
    void draw(HDC hdc) const {
        int carWidth = 30; // Ширина машины
        int carHeight = 20; // Высота машины

        // Создаём прямоугольник для машины
        RECT carRect = { static_cast<LONG>(x - carWidth / 2),
                         static_cast<LONG>(y - carHeight / 2),
                         static_cast<LONG>(x + carWidth / 2),
                         static_cast<LONG>(y + carHeight / 2) };

        // Заливка машины цветом
        HBRUSH hBrushCar = CreateSolidBrush(RGB(0, 0, 255)); // Синий цвет для машин
        FillRect(hdc, &carRect, hBrushCar);
        DeleteObject(hBrushCar);
    }

    // может ли машина ехать
    bool canMove(LightState lightState) const {
        // по направлению определяем какому сигналу следовать
        // если для машины горит зелёный или она в ремонтной зоне можно ехать
        if (direction) {
            return (lightState == GreenFirstDirection || isInRepairZone());
        }
        else {
            return (lightState == GreenSecondDirection || isInRepairZone());
        }
    }

    // находиться ли машина на объезде зоны ремонта
    bool isInRepairZone() const {
        return x >= repairZone.left && x <= repairZone.right;
    }

    // находится ли машина слишком близко к другой
    bool isTooClose(const Car& other) const {
        if (direction == other.direction) { // Только для машин, движущихся в одном направлении
            if (direction) { // Если движение вправо
                return (other.x > x && other.x - x < 80); // Если другая машина впереди и расстояние меньше 80
            }
            else { // Если движение влево
                return (other.x < x && x - other.x < 80); // Если другая машина впереди и расстояние меньше 80
            }
        }
        return false;
    }
};
