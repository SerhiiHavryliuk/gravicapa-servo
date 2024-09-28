/*
Serhii 
1) Кнопки з використанням переривань
  - Приклад коду для відслідковування натисненя кнопоки використовуючи переривання (в любий момент часу)
  - Кнопки підключаємо до загального мінуса (-) та до пінів stm32

    code example
    https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/

2) Налаштування TFT Display 
  - Встановити бібліотеку TFT_eSPI https://github.com/Bodmer/TFT_eSPI/
  робочий приклад https://blog.egorvakh.com/ru/posts/how-to-connect-lcd-ili9486-ili9341-to-esp32)
  
  - Налаштування та підключення пінів екрану (може бути два варіанти)
  файл для зміни пінів Setup42_ILI9341_ESP32.h
  тут C:\Users\s.havryliuk\Documents\Arduino\libraries\TFT_eSPI\User_Setups
  #define ILI9341_DRIVER
  #define TFT_MISO 19  // 12 19 (leave TFT SDO disconnected if other SPI devices share MISO)
  #define TFT_MOSI 23  // 13 23
  #define TFT_SCLK 18  // 14 18
  #define TFT_CS   15  // 25 15 Chip select control pin
  #define TFT_DC   2   // 27 2 Data Command control pin
  #define TFT_RST  4   // 26 4 Reset pin (could connect to RST pin)

  VCC -> 3.3V (Vcc)
  Gnd -> Gnd
  LED -> 3.3V (Vcc) - підсвітка екрану
*/

#include <ESP32Servo.h>

// TFT Display -----------------------------------------------------------------
#define TEXT "Test hello!"  // Text that will be printed on screen in any font

#include "Free_Fonts.h"  // Include the header file attached to this sketch

#include "SPI.h"
#include "TFT_eSPI.h"

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

// TFT Display -----------------------------------------------------------------
unsigned long drawTime = 0;
int activeItemMenu = 1;    // Активний пункт меню
int startTemperature = 0;  // Стартова температура
int deltaTemperature = 5;  // Наростання температури за 1 хв
// TFT Display -----------------------------------------------------------------


// Servo -----------------------------------------------------------------
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;  // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPin = 13;
// Servo -----------------------------------------------------------------


// TFT Buttons ---------------------------------------------------------------
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

// Кнопки для керування
Button buttonReset = { 32, 0, false };
Button buttonRunTest = { 33, 0, false };
Button buttonRight = { 25, 0, false };
Button buttonLeft = { 26, 0, false };
Button buttonDown = { 27, 0, false };
Button buttonUp = { 14, 0, false };

// Змінні для відслідковування часу між перериваннями
unsigned long button_time = 0;
unsigned long last_button_time = 0;

// Функції переривань для кнопок
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
  // TFT Display -----------------------------------------------------------------
  tft.begin();
  tft.setRotation(1);
  show_loading_menu_display();         // Показуємо заставку при завантаженні
  clear_display();                     // Очищення екрану
  showActiveMenuItem(activeItemMenu);  // Показуємо активний елемент меню
  show_main_menu_display();            // Показуємо основне меню
  // TFT Display -----------------------------------------------------------------

  // Налаштування кнопок
  Serial.begin(115200);

  // TFT Buttons ---------------------------------------------------------------
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
  myservo.setPeriodHertz(50);           // стандартний сигнал 50 Гц
  myservo.attach(servoPin, 500, 3200);  // підключаємо сервомотор на пін 13
  goToStartPositionServo();             // Повертаємо сервомотор у початкове положення
  // Servo ---------------------------------------------------------------
}

void loop() {
  // Перевірка натискання кнопок
  // Кнопка Reset
  if (buttonReset.pressed) {
    Serial.printf("Кнопка Reset натискалась %u раз(и)\n", buttonReset.numberKeyPresses);
    buttonReset.pressed = false;
    goToStartPositionServo();  // Повертаємо сервомотор у початкове положення
  }

  // Кнопка RunTes
  if (buttonRunTest.pressed) {
    Serial.printf("Кнопка Run Test натискалась %u раз(и)\n", buttonRunTest.numberKeyPresses);
    buttonRunTest.pressed = false;
    runTestServo(startTemperature, deltaTemperature);  // Запускаємо тест - вкл сервомотора
  }

  // Кнопка RunRight
  if (buttonRight.pressed) {
    Serial.printf("Кнопка Right натискалась %u раз(и)\n", buttonRight.numberKeyPresses);
    buttonRight.pressed = false;
    incrementMenuTemperature();
  }

  // Кнопка RunLeft
  if (buttonLeft.pressed) {
    Serial.printf("Кнопка Left натискалась %u раз(и)\n", buttonLeft.numberKeyPresses);
    buttonLeft.pressed = false;
    decreaseMenuTemperature();
  }

  // Кнопка RunDown
  if (buttonDown.pressed) {
    Serial.printf("Кнопка Down натискалась %u раз(и)\n", buttonDown.numberKeyPresses);
    buttonDown.pressed = false;
    setActiveItemMenu(2);  // Встановлюємо активний пункт в меню
  }

  // Кнопка RunUp
  if (buttonUp.pressed) {
    Serial.printf("Кнопка Up натискалась %u раз(и)\n", buttonUp.numberKeyPresses);
    buttonUp.pressed = false;
    setActiveItemMenu(1);  // Встановлюємо активний пункт в меню
  }
}

// -------------------------------------------------------------------------------
// Запуск тесту
// -------------------------------------------------------------------------------
void runTestServo(int startTemp, int deltaTemp) {
  int servoAngelStartTemp = 0;   // Стартовий кут повороту сервомотора сервомотора
  int servoTimeDeltaTemp = 100;  // Час між кроками сервомотора
  int testTime = 2;              // Час тесту (2хв, 3хв, 5хв, 15хв)

  // меню початок тесту
  // меню встановлення стартової температури startTemp + лоадінг
  show_text_in_menu("Start test", "", true);
  delay(1000);
  show_text_in_menu("Set start temperature", "", true);

  // обираємо стартову позицію сервомотора взалежності від температури
  switch (startTemp) {
    case 0:  // 0 Celcies
      servoAngelStartTemp = 10;
      myservo.write(servoAngelStartTemp);
      break;
    case -10:  // -10 Celcies
      servoAngelStartTemp = 0;
      myservo.write(servoAngelStartTemp);
      break;
    default:
      myservo.write(servoAngelStartTemp);
  }

  // обираємо час проходження тесту в залежності від delta (5->15хв, 10->5хв, 20->3хв, 30->2хв)
  switch (deltaTemp) {
    case 30:  // 30 Celcies/min (2хв)
      testTime = 2;
      servoTimeDeltaTemp = 700;
      myservo.write(servoTimeDeltaTemp);
      break;
    case 20:  // 20 Celcies/min (3хв)
      testTime = 3;
      servoTimeDeltaTemp = 1050;
      myservo.write(servoTimeDeltaTemp);
      break;
    case 10:  // 10 Celcies/min (5хв)
      testTime = 5;
      servoTimeDeltaTemp = 1750;
      myservo.write(servoTimeDeltaTemp);
      break;
    case 5:  // 5 Celcies/min (15хв)
      testTime = 15;
      servoTimeDeltaTemp = 5250;
      myservo.write(servoTimeDeltaTemp);
      break;
    default:
      myservo.write(servoTimeDeltaTemp);
  }

  // Затримка в дві секунди
  delay(2000);

  // меню старту тесту та відліку часу
  show_text_in_menu("Test in progress", "Time: " + String(testTime) + " min", true);
  Serial.println("servoTimeDeltaTemp - ");
  Serial.println(servoTimeDeltaTemp);


  // запускаємо сервомотор з визначеними параметрами
  // стартова позиція сервомотору в залежності віт стартової температури
  // обираємо час проходження тесту в залежності від delta
  for (pos = servoAngelStartTemp; pos <= 180; pos += 1) {  // обертаємо сервомотор від 0 до 180 градусів
    myservo.write(pos);                                    // задаємо кут серводвигуна
    delay(servoTimeDeltaTemp);                             // затримка для керування швидкістю повороту сервомотора
  }

  // меню кінець тесту
  show_text_in_menu("End test!", "Return to main menu", true);
  // Затримка в дві секунди
  delay(2000);
  // переходимо в голвне меню
  redrawMenu();
}

// Видалити
// void runTestServo() {
//   for (pos = 0; pos <= 180; pos += 1) { // обертаємо сервомотор від 0 до 180 градусів
//     myservo.write(pos);    // задаємо кут серводвигуна
//     delay(50);             // затримка 50 мс для плавного руху
//   }
// }

// -------------------------------------------------------------------------------
// Повернення сервомотора у початкове положення
// -------------------------------------------------------------------------------
void goToStartPositionServo() {
  myservo.write(0);  // повертаємо сервомотор у початкове положення (0 градусів)
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
    tft.setCursor(100, 100, 4);   // Встановлюємо курсор на екрані
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
  tft.setCursor(0, 4, 4);          // Встановлюємо курсор для виводу тексту
  tft.println("Menu gravicapa:");  // Виводимо заголовок меню
  tft.println();

  tft.setTextColor(TFT_WHITE);  // Білий колір для тексту
  tft.print("   Start Temperature: ");
  tft.setTextColor(TFT_YELLOW);           // Жовтий колір для температури
  tft.println(String(startTemperature));  // Виводимо початкову температуру

  tft.setTextColor(TFT_WHITE);  // Білий колір для тексту
  tft.print("   Delta Temperature: ");
  tft.setTextColor(TFT_YELLOW);           // Жовтий колір для температури
  tft.println(String(deltaTemperature));  // Виводимо значення Delta Temperature
}

// -------------------------------------------------------------------------------
// Виділення активного пункту меню
// -------------------------------------------------------------------------------
void showActiveMenuItem(int actieItem) {
  Serial.println("Servo " + String(actieItem));

  switch (actieItem) {
    case 1:
      tft.fillRect(0, 55, 320, 26, TFT_BLUE);
      break;
    case 2:
      tft.fillRect(0, 82, 320, 26, TFT_BLUE);
      break;
    case 3:
      tft.fillRect(0, 109, 320, 26, TFT_BLUE);
      break;
    case 4:
      tft.fillRect(0, 136, 320, 26, TFT_BLUE);
      break;
    default:
      Serial.println("Error: in main menu");
      tft.fillRect(0, 55, 320, 27, TFT_BLUE);
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

    // Затримка на 1 секунду
    delay(500);
  }
}
