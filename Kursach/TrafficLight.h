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

    static constexpr double greenDuration = 40.0;   // Время зеленого света
    static constexpr double redDuration = 55.0;     // Время красного света в обоих направлениях

public:
    TrafficLight() : currentState(GreenFirstDirection), timer(greenDuration) {}

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
