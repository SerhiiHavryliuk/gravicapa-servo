// Calibration.h
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>  // Для використання типів даних Arduino

struct Calibration {
    int minTemperature;  // Мінімальна температура
    int maxTemperature;  // Максимальна температура
    int minRotation;     // Мінімальний кут повороту (для minTemperature)
    int maxRotation;     // Максимальний кут повороту (для maxTemperature)
};

// Оголошення функції для зіставлення температури з кутом повороту
class CalibrationLib {
  public:
    CalibrationLib(int minTemp, int maxTemp, int minRot, int maxRot);
    int mapTemperatureToRotation(int temperature);  // Функція для інтерполяції температури
    
    // Методи для доступу до приватних значень
    int getMinRotation();  // Повертає minRotation
    int getMaxRotation();  // Повертає maxRotation
    int getMinTemperature();  // Повертає minTemperature
    int getMaxTemperature();  // Повертає maxTemperature

  private:
    Calibration _calibration;  // Структура для збереження параметрів
};

#endif
