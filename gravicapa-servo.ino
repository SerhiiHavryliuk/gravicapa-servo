#include <ESP32Servo.h>

// TFT Display -----------------------------------------------------------------
#define TEXT "Fire team!"  // Text that will be printed on screen in any font

#include "Free_Fonts.h"  // Include the header file attached to this sketch

#include "SPI.h"
#include "TFT_eSPI.h"  // Graphics and font library for ST7735 driver chip

// Calibrations ---------------------------------------------------------------
#include <Calibration.h>                       // Підключаємо бібліотеку
// todo: калібровку треба завершити, задумка хороша, реалізація ще з костилями
CalibrationLib calibration(-10, 40, 12, 140);  // Задаємо діапазони температур та кутів

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// TFT Display -----------------------------------------------------------------
#define TFT_GREY 0x5AEB  // New colour
unsigned long drawTime = 0;
int activeItemMenu = 1;      // Активний пункт меню (зараз 2 шт)
int startTemperature = -10;  // Стартова температураб стартове значення (-10 С)
int deltaTemperature = 30;   // Наростання температури за 1 хв, стартове значення (30 С)
// TFT Display -----------------------------------------------------------------


// Servo -----------------------------------------------------------------
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

// поки не зрозумфло як її використовувати, подумати щоб видалити
int pos = 0;  // variable to store the servo position

// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPin = 27;

// поки не зрозумфло як її використовувати, подумати щоб видалити
int currentRotation = 0;     // змінна для відстеження поточного кута
int startServoPosition = 0;  // змінна для зберігання початкового положення серводвигуна
// Servo -----------------------------------------------------------------

// Buttons ---------------------------------------------------------------
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

// Кнопки керування меню
Button buttonReset = { 21, 0, false };
Button buttonRunTest = { 22, 0, false };
Button buttonOk = { 17, 0, false };  // todo: поки не використовується, можна використовувати її для встановлення заданої температури для нового алгоритму граничні межі
Button buttonRight = { 2, 0, false };
Button buttonLeft = { 15, 0, false };
Button buttonDown = { 13, 0, false };
Button buttonUp = { 12, 0, false };

// Змінні для відслідковування часу між перериваннями
unsigned long button_time = 0;
unsigned long last_button_time = 0;

// Функції переривань для кнопок
// дебаунс потрібен щоб не було при одному натисненні деілька спрацювань кнопки
void IRAM_ATTR isr_buttonReset() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonReset.numberKeyPresses++;
    buttonReset.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR isr_runTest() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonRunTest.numberKeyPresses++;
    buttonRunTest.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR isr_right() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonRight.numberKeyPresses++;
    buttonRight.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR isr_left() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonLeft.numberKeyPresses++;
    buttonLeft.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR isr_down() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonDown.numberKeyPresses++;
    buttonDown.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR isr_up() {
  button_time = millis();
  if (button_time - last_button_time > 250) {  // Дебаунс 250 мс
    buttonUp.numberKeyPresses++;
    buttonUp.pressed = true;
    last_button_time = button_time;
  }
}
// TFT Buttons ---------------------------------------------------------------


void setup() {
  // Налаштування кнопок
  Serial.begin(115200);


  // TFT Display -----------------------------------------------------------------
  tft.init();
  tft.setRotation(3);                  // обертання тексту на дисплеї
  show_loading_menu_display();         // Показуємо заставку при завантаженні
  clear_display();                     // Очищення екрану
  showActiveMenuItem(activeItemMenu);  // Показуємо активний елемент меню
  show_main_menu_display();            // Показуємо основне меню
  // TFT Display -----------------------------------------------------------------

  // TFT Buttons ---------------------------------------------------------------
  // ініціалізація переривань
  pinMode(buttonReset.PIN, INPUT_PULLUP);
  attachInterrupt(buttonReset.PIN, isr_buttonReset, FALLING);

  pinMode(buttonRunTest.PIN, INPUT_PULLUP);
  attachInterrupt(buttonRunTest.PIN, isr_runTest, FALLING);

  pinMode(buttonRight.PIN, INPUT_PULLUP);
  attachInterrupt(buttonRight.PIN, isr_right, FALLING);

  pinMode(buttonLeft.PIN, INPUT_PULLUP);
  attachInterrupt(buttonLeft.PIN, isr_left, FALLING);

  pinMode(buttonDown.PIN, INPUT_PULLUP);
  attachInterrupt(buttonDown.PIN, isr_down, FALLING);

  pinMode(buttonUp.PIN, INPUT_PULLUP);
  attachInterrupt(buttonUp.PIN, isr_up, FALLING);
  // TFT Buttons ---------------------------------------------------------------

  // Servo ---------------------------------------------------------------
  // Налаштування серводвигуна
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);  // стандартний сигнал 50 Гц
  myservo.attach(servoPin);    // підключаємо сервомотор на пін 13

  goToStartPositionServo();     // Повертаємо сервомотор у початкове положення, за замовчанням - це -10 С

  // todo: Цей фікс прибирає дрибіжжання сервомотора при старті (якщо стартова температура ще менша), треба розібратись чому не можна виставити в 0 градусів
  //myservo.write(10);  // повертаємо сервомотор у початкове положення (0 градусів)
  // Servo ---------------------------------------------------------------
}

void loop() {
  // Перевірка натискання кнопок
  // Кнопка Reset
  // Повертаємо сервомотор у початкове положення
  if (buttonReset.pressed) {
    Serial.printf("Кнопка Reset натискалась %u раз(и)\n", buttonReset.numberKeyPresses);
    buttonReset.pressed = false;
    goToStartPositionServo();  
  }

  // Кнопка RunTes
  // Запускаємо тест із заданими параметрами - вкл сервомотора та поступово рухаємо його
  if (buttonRunTest.pressed) {
    Serial.printf("Кнопка Run Test натискалась %u раз(и)\n", buttonRunTest.numberKeyPresses);
    buttonRunTest.pressed = false;
    runTestServo(startTemperature, deltaTemperature);  
  }

  // Кнопка RunRight
  // збільшення значення у пункті меню
  if (buttonRight.pressed) {
    Serial.printf("Кнопка Right натискалась %u раз(и)\n", buttonRight.numberKeyPresses);
    buttonRight.pressed = false;
    incrementMenuTemperature();
  }

  // Кнопка RunLeft
  // зменшення значення у пункті меню
  if (buttonLeft.pressed) {
    Serial.printf("Кнопка Left натискалась %u раз(и)\n", buttonLeft.numberKeyPresses);
    buttonLeft.pressed = false;
    decreaseMenuTemperature();
  }

  // Кнопка RunDown
  // перехід по меню вниз
  if (buttonDown.pressed) {
    Serial.printf("Кнопка Down натискалась %u раз(и)\n", buttonDown.numberKeyPresses);
    buttonDown.pressed = false;
    setActiveItemMenu(2);  
  }

  // Кнопка RunUp
  // перехід по меню вгору
  if (buttonUp.pressed) {
    Serial.printf("Кнопка Up натискалась %u раз(и)\n", buttonUp.numberKeyPresses);
    buttonUp.pressed = false;
    setActiveItemMenu(1);
  }
}

// -------------------------------------------------------------------------------
// Запуск тесту
// -------------------------------------------------------------------------------
void runTestServo(int startTemp, int deltaTemp) {
  int servoAngelStartTemp = calibration.getMinRotation();  // Стартовий кут повороту сервомотора сервомотора
  int servoTimeDeltaTemp = 1500;                            // Час між кроками сервомотора
  int testTime = 2;                                        // Час тесту (2хв, 3хв, 5хв, 15хв)
  int maxRotation = calibration.getMaxRotation();

  // Потрібно для дебагу
  Serial.print("maxRotation - ");
  Serial.println(maxRotation);
  Serial.print("servoAngelStartTemp - ");
  Serial.println(servoAngelStartTemp);
  // Потрібно для дебагу

  // меню початок тесту
  show_text_in_menu("Start test", "", true);
  delay(1000);

  // поки цей функціонал не працює повноцінно, стартова температура зараз -10 С
  // обираємо стартову позицію сервомотора взалежності від температури
  switch (startTemp) {
    case 0:  // 0 Celcies
      servoAngelStartTemp = 20;
      break;
    case -10:  // -10 Celcies
      servoAngelStartTemp = calibration.getMinRotation();
      break;
    default:
      Serial.println("default in switch startTemp");
  }

  // обираємо час проходження тесту в залежності від delta (5->15хв, 10->5хв, 20->3хв, 30->2хв)
  // Від значення у servoTimeDeltaTemp залежить тривалість тесту у хв
  // 1 хв -це приблизно зараз servoTimeDeltaTemp = 1500, залажить від калібровки яка ще недописана повноцінно
  switch (deltaTemp) {
    case 30:  // 30 Celcies/min (2хв)
      testTime = 2;
      servoTimeDeltaTemp = 1500;
      break;
    case 20:  // 20 Celcies/min (3хв)
      testTime = 3;
      servoTimeDeltaTemp = 2250;
      break;
    case 10:  // 10 Celcies/min (5хв)
      testTime = 5;
      servoTimeDeltaTemp = 3750;
      break;
    case 5:  // 5 Celcies/min (15хв)
      testTime = 15;
      servoTimeDeltaTemp = 11250;
      break;
    default:
      testTime = 2;
      servoTimeDeltaTemp = 1500;
  }

  //todo: меню старту тесту та відліку часу (додати пізніше)
  show_text_in_menu("Test in progress", "Time: " + String(testTime) + " min", true);
  Serial.println("servoTimeDeltaTemp - ");
  Serial.println(servoTimeDeltaTemp);

// todo:тут костиль який треба виправити, і інтегрувати сюдт каліброску
// pos = 60 це -10 С (тобто треба прокрутити сервомотор щоб було -10 С, якщо його не крутити то буде -17 С і мотор дрибіжить)
  for (pos = 60; pos <= maxRotation; pos += 1) {  // обертаємо сервомотор від 0 до 180 градусів
      Serial.print("pos - ");
      Serial.println(pos);
      myservo.write(pos);         // задаємо кут серводвигуна
      delay(servoTimeDeltaTemp);  // затримка для керування швидкістю повороту сервомотора
      // тут ідеально додати час тесту
  }

  // меню кінець тесту
  show_text_in_menu("End test!", "Return to main menu", true);
  // Затримка в дві секунди
  delay(2000);
  // переходимо в голвне меню
  redrawMenu();
}


// -------------------------------------------------------------------------------
// Повернення сервомотора у початкове положення
// -------------------------------------------------------------------------------
void goToStartPositionServo() {
  // Використання гетера для отримання minRotation
  int minRotation = calibration.getMinRotation();
  int rotation = calibration.mapTemperatureToRotation(minRotation);

  int currentRotation = minRotation;
  int targetRotation = minRotation;

  Serial.print("minRotation - ");
  Serial.println(minRotation);
  Serial.print("rotation - ");
  Serial.println(rotation);
  Serial.print("currentRotation - ");
  Serial.println(currentRotation);
  myservo.write(2);  // повертаємо сервомотор у початкове положення (0 градусів)
  delay(2000);
  myservo.write(rotation);  // повертаємо сервомотор у мін положення, що вказане у калібровкі (-10 С)

  Serial.println("Сервомотор досяг мінімальної позиції");
}

// -------------------------------------------------------------------------------
// Зменшення температури в меню
// -------------------------------------------------------------------------------
void decreaseMenuTemperature() {
  if (activeItemMenu == 1) {
    if (startTemperature == 0) {
      startTemperature = -10;
    }
    if (startTemperature == -10) {
      startTemperature = -10;
    }
  }
  if (activeItemMenu == 2) {
    if (deltaTemperature == 5) {
      deltaTemperature = 5;
    } else {
      deltaTemperature -= 5;
    }
  }

  redrawMenu();
}

// -------------------------------------------------------------------------------
// Збільшення температури в меню
// -------------------------------------------------------------------------------
void incrementMenuTemperature() {
  if (activeItemMenu == 1) {
    if (startTemperature == 0) {
      startTemperature = 0;
    }
    if (startTemperature == -10) {
      startTemperature = 0;
    }
  }
  if (activeItemMenu == 2) {
    if (deltaTemperature == 30) {
      deltaTemperature = 30;
    } else {
      deltaTemperature += 5;
    }
  }

  redrawMenu();
}

// -------------------------------------------------------------------------------
// Встановити активним пункт меню
// -------------------------------------------------------------------------------
void setActiveItemMenu(int numberActiveItem) {
  activeItemMenu = numberActiveItem;
  redrawMenu();
}

// -------------------------------------------------------------------------------
// Перемальовування меню
// -------------------------------------------------------------------------------
void redrawMenu() {
  clear_display();                     // Очищення екрану
  showActiveMenuItem(activeItemMenu);  // Показуємо активний елемент меню
  show_main_menu_display();            // Показуємо основне меню
}

// -------------------------------------------------------------------------------
// Стартова заставка при включенні
// -------------------------------------------------------------------------------
void show_loading_menu_display() {
  for (int i = 1; i <= 3; i++) {
    clear_display();              // Очищення екрану
    tft.setCursor(50, 50, 4);   // Встановлюємо курсор на екрані
    tft.setTextColor(TFT_WHITE);  // Встановлюємо білий колір тексту
    tft.print("Loading");         // Виводимо текст "Loading"

    // Додавання крапок до тексту
    for (int j = 0; j < i; j++) {
      tft.print(" .");
    }

    tft.setCursor(10, 200, 4);
    tft.print("Powered by Fire team!");

    // Затримка на 1 секунду
    delay(500);
  }
}

// -------------------------------------------------------------------------------
// Очищення екрану
// -------------------------------------------------------------------------------
void clear_display() {
  tft.fillScreen(TFT_BLACK);  // Заповнюємо екран чорним кольором
}

// -------------------------------------------------------------------------------
// Основне меню
// -------------------------------------------------------------------------------
void show_main_menu_display() {
  tft.setCursor(0, 4, 4);  // Встановлюємо курсор для виводу тексту
  tft.println("Menu:");    // Виводимо заголовок меню
  tft.println();

  tft.setTextColor(TFT_WHITE);  // Білий колір для тексту
  tft.print("   Start Temp:  ");
  tft.setTextColor(TFT_YELLOW);         // Жовтий колір для температури
  tft.print(String(startTemperature));  // Виводимо початкову температуру
  tft.println(" C");

  tft.setTextColor(TFT_WHITE);  // Білий колір для тексту
  tft.print("   Delta Temp:  ");
  tft.setTextColor(TFT_YELLOW);         // Жовтий колір для температури
  tft.print(String(deltaTemperature));  // Виводимо значення Delta Temperature
  tft.println(" C");

  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE);  // Білий колір для тексту
  tft.print("                              v2024.1");
}

// -------------------------------------------------------------------------------
// Виділення активного пункту меню
// -------------------------------------------------------------------------------
void showActiveMenuItem(int actieItem) {
  Serial.println("Active Menu Item " + String(actieItem));

  switch (actieItem) {
    case 1:
      tft.fillRect(0, 53, 320, 26, TFT_BLUE);
      break;
    case 2:
      tft.fillRect(0, 80, 320, 26, TFT_BLUE);
      break;
    case 3:
      tft.fillRect(0, 107, 320, 26, TFT_BLUE);
      break;
    case 4:
      tft.fillRect(0, 135, 320, 26, TFT_BLUE);
      break;
    default:
      Serial.println("Error: in main menu");
      tft.fillRect(0, 53, 320, 26, TFT_BLUE);
  }
}

// -------------------------------------------------------------------------------
// Меню старту тесту
// -------------------------------------------------------------------------------
void show_text_in_menu(String text_1, String text_2, boolean isLoading) {
  for (int i = 1; i <= 3; i++) {
    clear_display();  // Очищення екрану

    // Перша строка
    tft.setCursor(5, 50, 4);      // Встановлюємо курсор на екрані
    tft.setTextColor(TFT_WHITE);  // Встановлюємо білий колір тексту
    tft.print(text_1);            // Виводимо текст "Loading"

    // Анімація завантаження
    if (isLoading) {
      // Додавання крапок до тексту
      for (int j = 0; j < i; j++) {
        tft.print(" .");
      }
    }

    // Друга строка
    tft.setCursor(5, 90, 4);      // Встановлюємо курсор на екрані
    tft.setTextColor(TFT_WHITE);  // Встановлюємо білий колір тексту
    tft.print(text_2);            // Виводимо текст "Loading"

    // Затримка на пів секунду
    delay(500);
  }
}
