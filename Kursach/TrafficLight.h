#pragma once

// состояния сигналов светофоров
enum LightState {
    GreenFirstDirection,
    RedBothDirectionsAfterFirst,
    RedBothDirectionsAfterSecond,
    GreenSecondDirection
};

class TrafficLight {
private:
    LightState currentState;
    double timer; // Таймер для текущего состояния

    double greenDuration;   // Время зеленого света
    double redDuration;     // Время красного света в обоих направлениях

public:
    // Конструктор, который принимает значения для зеленого и красного света
    TrafficLight(double greenDur, double redDur)
        : currentState(GreenFirstDirection),
        timer(greenDur),
        greenDuration(greenDur),
        redDuration(redDur) {
    }

    // правило трёх
    TrafficLight(const TrafficLight& light) = default;
    TrafficLight& operator=(const TrafficLight& light) = default;
    ~TrafficLight() {}

    void setDurations(double greenDur, double redDur) {
        greenDuration = greenDur;
        redDuration = redDur;
        timer = greenDuration; // сброс таймера на новое значение
        currentState = GreenFirstDirection; // Сброс состояния
    }

    // Обновляем состояние светофора
    void update(double deltaTime) {
        timer -= deltaTime;

        if (timer <= 0) {
            // Переходим к следующему состоянию
            switch (currentState) {
            case GreenFirstDirection:
                currentState = RedBothDirectionsAfterFirst;
                timer = redDuration;
                break;
            case RedBothDirectionsAfterFirst:
                currentState = GreenSecondDirection; // Правый светофор остается зеленым
                timer = greenDuration;
                break;
            case GreenSecondDirection:
                currentState = RedBothDirectionsAfterSecond;
                timer = redDuration;
                break;
            case RedBothDirectionsAfterSecond:
                currentState = GreenFirstDirection; // Правый светофор остается зеленым
                timer = greenDuration;
                break;
            }
        }
    }

    LightState getCurrentState() const {
        return currentState;
    }
};
