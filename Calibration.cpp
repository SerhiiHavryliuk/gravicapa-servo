// Calibration.cpp
#include "Calibration.h"

// Конструктор для ініціалізації значень калібрування
CalibrationLib::CalibrationLib(int minTemp, int maxTemp, int minRot, int maxRot) {
  _calibration.minTemperature = minTemp;
  _calibration.maxTemperature = maxTemp;
  _calibration.minRotation = minRot;
  _calibration.maxRotation = maxRot;
}

// Лінійна інтерполяція для зіставлення температури з кутом повороту
int CalibrationLib::mapTemperatureToRotation(int temperature) {
  if (temperature <= _calibration.minTemperature) {
    return _calibration.minRotation;
  }
  if (temperature >= _calibration.maxTemperature) {
    return _calibration.maxRotation;
  }

  return _calibration.minRotation + (temperature - _calibration.minTemperature) * 
         (_calibration.maxRotation - _calibration.minRotation) / 
         (_calibration.maxTemperature - _calibration.minTemperature);
}

// Методи доступу (гетери)
int CalibrationLib::getMinRotation() {
  return _calibration.minRotation;
}

int CalibrationLib::getMaxRotation() {
  return _calibration.maxRotation;
}

int CalibrationLib::getMinTemperature() {
  return _calibration.minTemperature;
}

int CalibrationLib::getMaxTemperature() {
  return _calibration.maxTemperature;
}
