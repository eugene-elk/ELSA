#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

// TODO: сделать define для открытия и закрытия клапана и компрессора
// TODO: новая программа для тестирования - сразу вписывать ноту
// TODO: файл с конфигурацией нот и серв на все программы

// Создаем сервоконтроллер
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
    // Установка скорости соединения (бод)
    Serial.begin(115200);
    
    pwm.begin();
    // Частота обновления серв - 60 Гц
    pwm.setPWMFreq(60); 
    
    //Pump HIGH - off, LOW - on
    //Valve HIGH - off, LOW - on
    pinMode(PUMP, OUTPUT); 
    pinMode(VALVE, OUTPUT);  
    delay(100);
    digitalWrite(VALVE, HIGH);
    digitalWrite(PUMP, LOW);
    // Выставляем голову в центр
    pwm.setPWM(HEAD_SERVO, 0, get_pulse(50));
    // Берём ноту C (До), в этой ноте все пальцы закрыты
    pick_note(0);
}

// Взятие ноты, в этой программе используется только в setup
void pick_note(uint8_t note)
{
    // Перебираем все 11 подключенных серв
    for (int i = 0; i < 11; ++i)
    {
        // Если наткнулись на серву головы - пропускаем
        if (i == HEAD_SERVO) 
            continue;
        // Записываем в переменную p нужную позицию для сервы i по ноте note
        int p = get_pulse(positions[i][fingerings[note][i]]);
        // Выставляем по физическому номеру сервы её в нужное положение
        pwm.setPWM(servos[i], 0, p);
    }
}

// Пересчитывает значение сервы от 0 до 100 в значения от SERVOMIN до SERVOMAX
int get_pulse(int angle)
{
    return map(angle, 0, 100, SERVOMIN, SERVOMAX);
}

void loop() {
    // При наличии данных с компа - выставляем пальцы в нужные позиции
	if(Serial.available() > 1) {
        // Получаем номер сервы и угол, на который она должна повернуть
		unsigned s = Serial.read();
		unsigned angle = Serial.read();

        // Выставляем каждую серву в нужное значение
        pwm.setPWM(servos[s], 0, get_pulse(angle));
	}
}