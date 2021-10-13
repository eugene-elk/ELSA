#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "/home/tlab/Documents/ELSA/arduino/settings.h"

// Создаем сервоконтроллер
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Текущее состояние для автомата состояний
short state = 0;

// Буфер для хранения текущей расшифровываемой ноты
byte buffer[50];

// Флаги
bool read_seq = false;
bool read_meta = false;

uint16_t sequence[1024][3];
uint16_t seq_length = 0; 
unsigned tempo = 0;
uint16_t total_seq_length = 0;

// что-то для задержек
uint16_t semiq1 = 125;
uint16_t semiq2 = 200;

void setup()
{
    // Установка скорости соединения (бод)
    Serial.begin(115200);
    
    pwm.begin();
    // Частота обновления серв - 60 Гц
    pwm.setPWMFreq(60); 
    
    // Pump HIGH - off, LOW - on
    // Valve HIGH - off, LOW - on
    pinMode(PUMP, OUTPUT); 
    pinMode(VALVE, OUTPUT);  

    // Объявление
    pinMode(DOWNBORDER, INPUT_PULLUP);
    pinMode(UPBORDER, INPUT_PULLUP);
    pinMode(ARMSMOTORDOWN, OUTPUT);
    pinMode(ARMSMOTORUP, OUTPUT);
    pinMode(LCD, OUTPUT);

    delay(100);
    
    digitalWrite(PUMP, HIGH);
    digitalWrite(VALVE, HIGH);
    digitalWrite(ARMSMOTORUP, LOW);
    digitalWrite(ARMSMOTORDOWN, LOW);

    // Выставляем голову в центр
    pwm.setPWM(HEAD_SERVO, 0, get_pulse(HEAD_FRONT));
    delay(100);
    // Берём ноту C (До), в этой ноте все пальцы закрыты
    pick_note(0);
    delay(100);
    
    // raise_arms();
}

// Взятие ноты
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
        //Выставляем по физическому номеру сервы её в нужное положение
        pwm.setPWM(servos[i], 0, p);
    }
}

// Пересчитывает значение сервы от 0 до 100 в значения от SERVOMIN до SERVOMAX
int get_pulse(int angle)
{
    return map(angle, 0, 100, SERVOMIN, SERVOMAX);
}

// Функция опускания рук
void lower_arms()
{
    // Пока не нажат концевик - двигаем руки вниз
    while (digitalRead(DOWNBORDER) == 0)
    {
        digitalWrite(ARMSMOTORDOWN, HIGH);
        digitalWrite(ARMSMOTORUP, LOW);
    }
    // Останавливаем руки
    digitalWrite(ARMSMOTORDOWN, LOW);
    digitalWrite(ARMSMOTORUP, LOW);
}

// Функция поднятия рук
void raise_arms()
{
    // Пока не нажат концевик - двигаем руки вверх
    while (digitalRead(UPBORDER) == 0)
    {
        digitalWrite(ARMSMOTORDOWN, LOW);
        digitalWrite(ARMSMOTORUP, HIGH);
    }
    // Останавливаем руки
    digitalWrite(ARMSMOTORDOWN, LOW);
    digitalWrite(ARMSMOTORUP, LOW);
}

void loop() 
{
    // Странный автомат состояний
    // Состояние 0 - кручение головой
    if (state == 0 && Serial.available() > 0)
    {
        // Ожидаем команду старта состояния с компьютера
        Serial.read();
        
        //lower_arms();
        
        // Вращение головой, влево и обратно вправо
        for (int i = HEAD_FRONT; i > HEAD_LEFT; --i)
        {
            pwm.setPWM(HEAD_SERVO, 0, get_pulse(i));
            delay(50);
        }
        delay(1500);
        for (int i = HEAD_LEFT; i <= HEAD_FRONT; ++i)
        {
            pwm.setPWM(HEAD_SERVO, 0, get_pulse(i));
            delay(50);
        }
        
        // Закрыли клапан, включили компрессор, раздуваемся (?)
        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, LOW);    

        //raise_arms();
    
        // Переход на следующее состояние
        state = 1;

        // Отправляем на компьютер сигнал о завершении состояния
        Serial.write(1);
    }

    // Состояние 1 - Проигрыш из трех нот
    else if (state == 1 && Serial.available() > 0)
    {
        // Ожидаем команду старта состояния с компьютера
        Serial.read();

        delay(500);

        pick_note(0);
        // Открыли  клапан
        digitalWrite(VALVE, HIGH);
        delay(1000);
        
        pick_note(4);
        delay(1000);

        pick_note(7);
        delay(1000);

        // Закрыли клапан, выключили компрессор
        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, HIGH);

        // Переход на следующее состояние
        state = 3;

        // Отправляем на компьютер сигнал о завершении состояния
        Serial.write(1);
    }

    // Это состояние не используется, зачем оно?
    else if (state == 2 && Serial.available() > 0)
    {   
        Serial.read();

        delay(500);

        pick_note(10);
        digitalWrite(VALVE, HIGH);
        digitalWrite(PUMP, LOW);
        delay(1000);
        
        pick_note(2);
        delay(1000);

        pick_note(5);
        delay(1000);

        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, HIGH);
        pick_note(3);

        Serial.write(1);

        state = 3;
    }

    // Состояние 3 - Поворот головы в сторону гитариста
    else if (state == 3 && Serial.available() > 0)
    {
        // Ожидаем команду старта состояния с компьютера
        Serial.read();

        // Поворачиваем голову в сторону гитариста
        for (int i = HEAD_FRONT; i > HEAD_LEFT; --i)
        {
            pwm.setPWM(HEAD_SERVO, 0, get_pulse(i));
            delay(50);
        }

        // хз что это
        digitalWrite(LCD, HIGH);

        // Закрыли клапан, выключили компрессор
        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, HIGH);

        // Переход на следующее состояние
        state = 4;

        // Отправляем на компьютер сигнал о завершении состояния
        Serial.write(1);
    }

    // Состояние 4 - 
	else if (state == 4 && Serial.available() > 1) 
    {
        // хз что это
        digitalWrite(LCD, LOW);

        // Нота и длительность
		unsigned note = Serial.read();
        unsigned dur = Serial.read();

        // Отправляем на компьютер подтверждение принятия ноты
        Serial.write(note);

        // Если ноты кончились - заканчиваем 
        if (dur == 0) 
        {
            // Поворачиваем голову, готовимся играть
            for (int i = HEAD_LEFT; i <= HEAD_FRONT; ++i)
            {
                pwm.setPWM(HEAD_SERVO, 0, get_pulse(i));
                delay(50);
            }
            // Закрыли клапан, включили компрессор, раздуваемся
            digitalWrite(VALVE, LOW);
            digitalWrite(PUMP, LOW);
            delay(1000);
            
            // Переходим на следующее состояние
            state = 5;
        }
        else 
        {
            // Сохраняем сгенерированнуе ноты
            sequence[seq_length][0] = note;
            sequence[seq_length][1] = dur;
            seq_length++;         
        }
	}

    // Состояние 5 
    else if (state == 5 && Serial.available() > 0)
    {
        // Ждем команду с компьютера
        Serial.read();

        // 
        short passed = 0;

        // 
        for (int i = 0; i < seq_length; ++i)
        {
            if (sequence[i][0] == 12)
            {
                // Закрыли клапан, выключили компрессор
                digitalWrite(VALVE, LOW);
                digitalWrite(PUMP, HIGH);
                delay(uint16_t(sequence[i][1]*semiq2*1));
            }
            else 
            {
                
                pick_note(sequence[i][0]);
                digitalWrite(PUMP, LOW);
                digitalWrite(VALVE, HIGH);
                delay(uint16_t(sequence[i][1]*semiq2*0.95));
                digitalWrite(VALVE, LOW);
                delay(uint16_t(sequence[i][1]*semiq2*0.05));
            }
        }

        // Закрыли клапан, выключили компрессор
        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, HIGH);


        seq_length = 0;

        // Переход на следующее состояние
        state = 6;
        
        // Отправляем на компьютер сигнал о завершении состояния
        Serial.write(1);
    }

    // Состояние 6 - Расшифровка полученной с компьютера последовательности
    else if(state == 6 && Serial.available() > 3) 
    {

        Serial.readBytes(buffer, 4);
        Serial.write(1);

        if (not read_meta)
        {
            total_seq_length = ((unsigned) buffer[2]) << 8 | ((unsigned) buffer[3]);
            read_meta = true;
        }

        else 
        {
            // Расшифровываем ноту
            unsigned note = (unsigned) buffer[0];
            unsigned velocity = (unsigned) buffer[1];
             // Восстанавливаем число из двух байтов, первое домножаем на 256, прибавляем второе
            uint16_t msg_time = ((unsigned) buffer[2] << 8) | (unsigned) buffer[3]; 

             // Записываем расшифрованную ноту в последовательность
            sequence[seq_length][0] = note;
            sequence[seq_length][1] = velocity;
            sequence[seq_length][2] = msg_time;

            // Увеличиваем длину расшифрованной последовательности
            seq_length++;

            // Когда размер расшифрованного достиг полного размера последовательности, заканчиваем чтение, готовимся к игре
            if (seq_length == total_seq_length) 
            {
                // Открываем клапан и включаем компрессор
                digitalWrite(PUMP, LOW);
                digitalWrite(VALVE, LOW);
                // Время на раздувание
                delay(1000);
                
                // Переход к игре
                state = 7;
            }  
        }  
	}

    // Состояние 7 - Игра
    else if (state == 7 && Serial.available() > 0)
    {   
        Serial.read();
        for (int i = 0; i < seq_length; ++i)
        {
            // Задержка перед нотой
            delay(sequence[i][2]);
            // Взятие ноты
            pick_note(sequence[i][0]);
            
            if (sequence[i][1] > 0)
            {
                // Открыли клапан
                digitalWrite(VALVE, HIGH);    
                // Начали дуть
                digitalWrite(PUMP, LOW);
            }
            else 
            {
                // Закрыли клапан
                digitalWrite(VALVE, LOW);
            } 
        }
        // Закрыли клапан, начали дуть
        digitalWrite(VALVE, LOW);
        digitalWrite(PUMP, HIGH);

        // 
        state = 8;

        // Опускаем руки
        lower_arms();

        // Сообщаем на компьютер о завершении состояния
        Serial.write(1);
    }   
}