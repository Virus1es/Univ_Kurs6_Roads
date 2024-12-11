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
    void move(LightState lightState, int centerY) {
        if (direction) { // Движение вправо
            if (y > centerY || x > repairZone.right + 50) {
                // Если машина в зоне ремонта, она продолжает движение
                x += speed; // Движение вправо
            }
            else {
                // Проверка на светофор для верхней дороги
                if (canMove(lightState) && y < centerY && x < repairZone.left - 150) { // Если машина на верхней дороге и Если не достигли светофора
                    x += speed; // Движение вправо
                }
            }

            // Перестроение на нижнюю дорогу
            if (y < centerY && lightState == GreenFirstDirection && x >= repairZone.left - 150) {
                y += 130; // Перемещаем вниз
            }

            if (y > centerY && x > repairZone.right + 50) {
                y -= 130; // перемещаем обратно
            }
        }
        else { // Движение влево
            if (isInRepairZone()) {
                // Если машина в зоне ремонта, она продолжает движение
                x -= speed; // Движение влево
            }
            else {
                // Проверка на светофор для верхней дороги
                if (canMove(lightState) && y > centerY && x > repairZone.right + 200) { // Если машина на нижней дороге
                     // Если не достигли светофора на 100 пикселей
                    x -= speed; // Движение влево
                }
                else if (x <= repairZone.right + 200)
                    x -= speed; // Движение влево
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
        // Если светофор красный, и машина не в зоне ремонта, она не может двигаться
        return !(lightState == RedBothDirections && !isInRepairZone());
    }

    // находиться ли машина на объезде зоны ремонта
    bool isInRepairZone() const {
        return x >= repairZone.left && x <= repairZone.right;
    }
};
