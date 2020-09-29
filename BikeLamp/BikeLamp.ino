// Контроллер поворотников и габаритных огней для велосипеда на ATTiny85 (8 MHz)
// Дата последней модификации: 2015-08-29
// Подключение:
// 6, 7 - входы переключателя/выходы индикации (PB1, PB2)
// 2, 3 - выходы поворотников (PB3, PB4)
// 5 - выход габ.огней (PB0 - ШИМ)
// Для управления портами переключателя/индикации используется прямое обращение
// к регистрам для устранения мерцания св.диода из-за неодновременного переключения

// состояния поворотников (активный сигнал 0)
#define OFF 3
#define TURN1 2
#define TURN2 1

// порты
const int switch1 = 1;  // переключатель/индикация
const int switch2 = 2;
const int turn1 = 3;    // поворотники
const int turn2 = 4;
const int light = 0;    // габариты (PWM)

byte turnState = OFF;   // текущее состояние
byte lightState = 0;

const int turnFlashCycle = 450;    // длительность полупериода мигания поворотников в мс
long turnFlashTime;                // момент прошлого изменения сигнала поворотников
int turnFlashState = 0;            // текущее состояние мигающего сигнала

const int lightOnCycle = 2000;       // длительность периода для включения габ.огней в мс
const int lightOnClicksNeeded = 3;   // кол-во щелчков, требуемое для включения габ.огней
long lightOnFirstClickTime;          // момент первого щелчка
int linghOnClicks = 0;               // счетчик кол-ва щелчков

const int lightPWM = 50;    // уровень PWM для габ.огней
byte flashState = 0;         // текущее состояние дин.индикации

void setup() {
  
  // инициализируем порты
  pinMode(turn1, OUTPUT);
  pinMode(turn2, OUTPUT);
  pinMode(light, OUTPUT);
  // входы от переключателя тоже в режим выходов
  pinMode(switch1, OUTPUT); 
  pinMode(switch2, OUTPUT);
  // и высокий уровень на них
  digitalWrite(switch1, 1);
  digitalWrite(switch2, 1);

  // Serial.begin(115200);
}


void loop() {
  
  // опрос входов
  // входы от переключателя временно в режим входов
  // pinMode(switch1, INPUT);
  // pinMode(switch2, INPUT);
  noInterrupts();
  DDRB &= ~(_BV(switch1) | _BV(switch2));

  // задержка для зарядки паразитной емкости провода к переключателю
  delayMicroseconds(1);
  
  // считываем состояние переключателя
  int switchState = ((PINB >> switch1) | (PINB >> (switch2 - 1))) & OFF;
  
  // входы от переключателя в режим выходов
  // pinMode(switch1, OUTPUT);
  // pinMode(switch2, OUTPUT);
  DDRB |= _BV(switch1) | _BV(switch2);
  interrupts();

  if (switchState != turnState) {
    // состояние переключателя изменилось
    turnState = switchState;
    if (turnState != OFF) {
      // включено положение 1 или 2
      turnFlashTime = millis();  // запоминаем момент включения для мигания
      turnFlashState = 1;        // включаем мигающий сигнал

      if (linghOnClicks == 0)
        // переключений еще не было
        lightOnFirstClickTime = millis();  // запоминаем момент первого переключения
      linghOnClicks++;    // считаем переключения
    }
    else
      // нейтральное положение
      turnFlashState = 0;
  }

  // обработка мигания
  if (turnState != OFF && (millis() - turnFlashTime > turnFlashCycle)) {
    // поворотник включен, и прошло время полупериода мигания
    turnFlashState = !turnFlashState;  // переключаем состояние
    turnFlashTime = millis();          // запоминаем момент переключения
  }
  
  // обработка включения габаритов
  if (millis() - lightOnFirstClickTime < lightOnCycle) {
    // период включения еще не истек
    if (linghOnClicks == lightOnClicksNeeded) {
      // сделано нужное кол-во переключений
      lightState = !lightState;    // переключаем состояние габаритов
      linghOnClicks = 0;           // сбрасываем счетчик
    }
  }
  else
    // период истек
    linghOnClicks = 0;     // сбрасываем счетчик
    
  // обработка дин.индикации включенных габаритов
  flashState++;
  if (flashState == 4) flashState = 0;

  // индикация на переключателе
  if (turnState == TURN1) {
    // поворотник 1
    // digitalWrite(switch2, turnFlashState);
    // digitalWrite(switch1, 0);
    PORTB = (PORTB & ~(_BV(switch1) | _BV(switch2))) | (turnFlashState << switch2);
  }
  else if (turnState == TURN2) {
      // поворотник 2
      // digitalWrite(switch1, turnFlashState);
      // digitalWrite(switch2, 0);
      PORTB = (PORTB & ~(_BV(switch1) | _BV(switch2))) | (turnFlashState << switch1);
    }
    else if (lightState) {
        // габариты
        PORTB = (PORTB & ~(_BV(switch1) | _BV(switch2))) | 
          (((flashState == 0)?0:1) << switch1) | (((flashState == 2)?0:1) << switch2);
      }
      else {
        // все выключено
        PORTB |= _BV(switch1) | _BV(switch2);
      }
    
  // выход на поворотник 1
  if (turnState == TURN1) digitalWrite(turn1, turnFlashState);
  else digitalWrite(turn1, 0);
  
  // выход на поворотник 2
  if (turnState == TURN2) digitalWrite(turn2, turnFlashState);
  else digitalWrite(turn2, 0);

  // выход PWM на габаритные огни
  if (lightState) analogWrite(light, lightPWM);
  else analogWrite(light, 0);

  //Serial.println();
    
  delay(5);
}

 

