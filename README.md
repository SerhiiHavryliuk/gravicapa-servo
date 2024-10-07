# gravicapa-servo
/*
Serhii 
1) Кнопки з використанням переривань
  - Приклад коду для відслідковування натисненя кнопоки використовуючи переривання (в любий момент часу)
  - Кнопки підключаємо до загального мінуса (-) та до пінів stm32

    code example
    https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/

2) Налаштування TFT Display 
  - Встановити бібліотеку TFT_eSPI https://github.com/Bodmer/TFT_eSPI/
  робочий приклад є в exaples бібліотеки
  C:\Users\s.havryliuk\Documents\Arduino\libraries\TFT_eSPI\examples\160 x 128\TFT_Print_Test
  
  - Налаштування та підключення пінів екрану 
  файл для зміни пінів User_Setup_Select.h
  тут C:\Users\s.havryliuk\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
  розкоментувати строку
  #include <User_Setups/Setup25_TTGO_T_Display.h>
 
*/
