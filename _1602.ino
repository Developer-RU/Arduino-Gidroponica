#include <wiring.c>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define relay 10  // реле
#define pin_A 12  // пин1 энкодера
#define pin_B 11  // пин2 энкодера
#define button 5  // кнопка энкодера

LiquidCrystal lcd(7, 6, 8, 4, 3, 2);

int ON_state, OFF_state, ON_state_current, OFF_state_current;
uint8_t state_position = 0;
uint8_t seconds = 1; // (было 60)
uint8_t fadeAmountON = 1; // изменение ON на секунду
uint8_t fadeAmountOFF = 60; // изменение OFF на минуту

unsigned long currentTime;
unsigned long loopTime = 0;
unsigned long loopTime2 = 0;
unsigned long loopTime3 = 0;
unsigned long loopTime4 = 0;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev = 0;
unsigned long settings_hide = 0;

uint8_t st = 0;
uint8_t buttonState = 0;
uint8_t _pwm = 0;
uint8_t relay_pwm = 1;

bool buttonBlock = 0;
bool relaystate = false;
bool tt = false;


void setup() {
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  if (EEPROM.read(7) == 13) {
    if (EEPROM.read(1) == 1) {
      ON_state_current = ON_state = EEPROM.read(0) + (EEPROM.read(2) * 255);
    } else if (EEPROM.read(1) == 0) {
      ON_state_current = ON_state = EEPROM.read(2);
    }
    if (EEPROM.read(4) == 1) {
      OFF_state_current = OFF_state = EEPROM.read(3) + (EEPROM.read(5) * 255);
    } else if (EEPROM.read(4) == 0) {
      OFF_state_current = OFF_state = EEPROM.read(5);
    }
    relay_pwm = EEPROM.read(6);
  } else {
    ON_state_current = ON_state = 5;
    OFF_state_current = OFF_state = 60;
    relay_pwm = 10;

    EEPROM.write(0, 0); EEPROM.write(1, 0); EEPROM.write(2, 5);
    EEPROM.write(3, 0); EEPROM.write(4, 0); EEPROM.write(5, 60);
    EEPROM.write(6, 10);
    EEPROM.write(7, 13);
  }

  lcd.begin(16, 2);

  currentTime = millis();
  loopTime = currentTime;
  lcd_update();
}

void loop()
{
  // читаем кнопку
  buttonState = digitalRead(button);
  if (buttonState == LOW) {
    if (!buttonBlock) {
      switch (st) {
        case 1: st = st + 1;
          if (ON_state > 255) {
            EEPROM.write(0, ON_state % 255);
            EEPROM.write(1, 1);
            EEPROM.write(2, round(ON_state / 255));
          } else {
            EEPROM.write(0, 0);
            EEPROM.write(1, 0);
            EEPROM.write(2, ON_state);
          }
          if (OFF_state > 255) {
            EEPROM.write(3, OFF_state % 255);
            EEPROM.write(4, 1);
            EEPROM.write(5, round(OFF_state / 255));
          } else {
            EEPROM.write(3, 0);
            EEPROM.write(4, 0);
            EEPROM.write(5, OFF_state);
          }
          break;
        case 2: st = 3; seconds = 1; // (было 60)
          if (ON_state > 255) {
            EEPROM.write(0, ON_state % 255);
            EEPROM.write(1, 1);
            EEPROM.write(2, round(ON_state / 255));
          } else {
            EEPROM.write(0, 0);
            EEPROM.write(1, 0);
            EEPROM.write(2, ON_state);
          }
          if (OFF_state > 255) {
            EEPROM.write(3, OFF_state % 255);
            EEPROM.write(4, 1);
            EEPROM.write(5, round(OFF_state / 255));
          } else {
            EEPROM.write(3, 0);
            EEPROM.write(4, 0);
            EEPROM.write(5, OFF_state);
          }
          ON_state_current = ON_state;
          OFF_state_current = OFF_state;
          break;
        case 3: st = 0;
          EEPROM.write(6, relay_pwm);
          break;
        default: st = st + 1;
          break;
      }
      buttonBlock = true;
      lcd_update();
      delay(5);
    }
  } else {
    buttonBlock = false;
  }

  // заходит каждые 2мс
  currentTime = millis();

  if (currentTime >= (loopTime + 1)) {
    encoder_A = digitalRead(pin_A);    // Read encoder pins
    encoder_B = digitalRead(pin_B);
    if ((!encoder_A) && (encoder_A_prev)) {
      // УВЕЛИЧЕНИЕ
      if (encoder_B) {
        switch (st) {
          case 1: // редактор ON
            if (ON_state + fadeAmountON <= 3600) ON_state += fadeAmountON; //макс. интервал
            lcd_update();
            break;
          case 2: // редактор OFF
            if (OFF_state + fadeAmountOFF <= 3600) OFF_state += fadeAmountOFF; //макс. интервал
            lcd_update();
            break;
          case 3: // редактор PWM
            if (relay_pwm < 250) relay_pwm++;
            lcd_update();
            break;
          default: // начальный экран, редактор текущего режима
            // какой текущий режим?
            if (state_position == 1) {
              if (ON_state_current + fadeAmountON <= 3600) ON_state_current += fadeAmountON; //макс. интервал
            }
            if (state_position == 0) {
              if (OFF_state_current + fadeAmountOFF <= 3600) OFF_state_current += fadeAmountOFF; //макс. интервал
            }
            lcd_update();
            break;
        }
      } else {
        // УМЕНЬШЕНИЕ
        switch (st) {
          case 1: // редактор ON
            if (ON_state - fadeAmountON > 0) ON_state -= fadeAmountON;
            lcd_update();
            break;
          case 2: // редактор OFF
            if (OFF_state - fadeAmountOFF > 0) OFF_state -= fadeAmountOFF;
            lcd_update();
            break;
          case 3: // редактор PWM
            if (relay_pwm > 1) relay_pwm--;
            lcd_update();
            break;
          default: // начальный экран, редактор текущего режима
            // какой текущий режим?
            if (state_position == 1) {
              if (ON_state_current - fadeAmountON >= 1) ON_state_current -= fadeAmountON;
              if (ON_state_current <= 0 ) ON_state_current = 1;
            }
            if (state_position == 0) {
              if (OFF_state_current - fadeAmountOFF >= 1) OFF_state_current -= fadeAmountOFF;
              if (OFF_state_current <= 0 ) OFF_state_current = 1;
            }
            seconds = 1; // (было 60)
            lcd_update();
            break;
        }
      }
      loopTime4 = settings_hide;
    }
    encoder_A_prev = encoder_A;     // Store value of A for next time
    loopTime = currentTime;  // Updates loopTime
  }

  // Второе прерывание - обратный отсчет, заходит каждую 0,5 сек, (я переделал, было 1000)
  if (currentTime >= (loopTime2 + 500)) {
    loopTime2 = currentTime;
    // какой текущий режим?
    if (seconds - 1 >= 0) {
      seconds -= 1;
    } else {
      seconds = 1; // (было 60)
      if (state_position == 1) { // ON
        if (ON_state_current - 1 >= 1) {
          ON_state_current -= 1;
        } else {
          OFF_state_current = OFF_state;
          ON_state_current = ON_state;
          state_position = 0;
          relaystate = false;
          if (tt)
          {
            tt = false;

            timer0_millis = 0;
            currentTime = millis();
            loopTime = loopTime2 = loopTime3 = loopTime4 = 0;
          }
        }
      } else { // OFF
        if (state_position == 0) {
          if (OFF_state_current - 1 >= 1) {
            OFF_state_current -= 1;
          } else {
            OFF_state_current = OFF_state;
            ON_state_current = ON_state;
            state_position = 1;
            relaystate = true;
            tt = true;

          }
        }
      }
      lcd_update();
    }
  }

  // Третье прерывание для ШИМ каждые 50мс (relay_pwm)
  if (currentTime >= (loopTime3 + relay_pwm)) {
    if (relaystate) {
      if (_pwm < 255) {
        _pwm++;
        analogWrite(relay, _pwm);
        loopTime3 = currentTime;
      }
    } else {
      _pwm = 0;
      analogWrite(relay, 0);
    }
  }
}

void lcd_update()
{
  lcd.clear();
  switch (st) {

    case 1: // редактор ON
      lcd.setCursor(0, 0); lcd.print("Select value ON ");
      lcd.setCursor(0, 1); lcd.print("ON:");
      lcd.setCursor(4, 1); lcd.print(ON_state);
      lcd.setCursor(8, 1); lcd.print("OFF:");
      lcd.setCursor(13, 1); lcd.print(round(OFF_state / 60));

      settings_hide = currentTime;

      if (settings_hide >= (loopTime4 + 10000)) {
        loopTime4 = settings_hide;
        st = 0;
      }

      break;

    case 2: // редактор OFF
      lcd.setCursor(0, 0); lcd.print("Select value OFF ");
      lcd.setCursor(0, 1); lcd.print("ON:");
      lcd.setCursor(4, 1); lcd.print(ON_state);
      lcd.setCursor(8, 1); lcd.print("OFF:");
      lcd.setCursor(13, 1); lcd.print(round(OFF_state / 60));

      settings_hide = currentTime;

      if (settings_hide >= (loopTime4 + 10000)) {
        loopTime4 = settings_hide;
        st = 0;
      }

      break;

    case 3: // редактор OFF
      lcd.setCursor(0, 0); lcd.print("Select PWM");
      lcd.setCursor(0, 1); lcd.print("VALUE:");
      lcd.setCursor(9, 1); lcd.print(relay_pwm);

      settings_hide = currentTime;

      if (settings_hide >= (loopTime4 + 10000)) {
        loopTime4 = settings_hide;
        st = 0;
      }

      break;

    default: // начальный экран, редактор текущего режима

      if (state_position == 1) {
        lcd.setCursor(0, 0); lcd.print("Active ON");
        lcd.setCursor(11, 0); lcd.print(doublesimbols(round(ON_state_current / 60)));
        lcd.setCursor(13, 0); lcd.print(":");
        lcd.setCursor(14, 0); lcd.print(doublesimbols(ON_state_current % 60));
        lcd.setCursor(0, 1); lcd.print("ON:");
        lcd.setCursor(4, 1); lcd.print(ON_state);
        lcd.setCursor(8, 1); lcd.print("OFF:");
        lcd.setCursor(13, 1); lcd.print(round(OFF_state / 60));
      }

      if (state_position == 0) {
        lcd.setCursor(0, 0); lcd.print("Active OFF");
        lcd.setCursor(11, 0); lcd.print(doublesimbols(round(OFF_state_current / 60)));
        lcd.setCursor(13, 0); lcd.print(":");
        lcd.setCursor(14, 0); lcd.print(doublesimbols(OFF_state_current % 60));
        lcd.setCursor(0, 1); lcd.print("ON:");
        lcd.setCursor(4, 1); lcd.print(ON_state);
        lcd.setCursor(8, 1); lcd.print("OFF:");
        lcd.setCursor(13, 1); lcd.print(round(OFF_state / 60));
      }

      break;
  }
}

String doublesimbols(int i) {
  String res;
  if ((String(i)).length() == 2) {
    res = String(i);
  } else {
    res = "0" + String(i);
  }
  return res;
}
