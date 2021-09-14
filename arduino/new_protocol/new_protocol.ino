#include <Adafruit_PWMServoDriver.h>

// Создаем сервоконтроллер
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Максимальный и минимальный углы поворота сервы
#define SERVOMIN 80
#define SERVOMAX 600

// Порт клапана
#define VALVE 13 //HIGH -- off, LOW -- on

// Порт компрессора
#define PUMP 17 //HIGH -- off, LOW -- on

// Номер сервы головы
const short HEAD_SERVO = 0; 

// Положение сервы, при котором голова смотрит вперёд
const short HEAD_FRONT = 47;

// Буфер для хранения текущей расшифровываемой ноты
byte buffer[4];

// Флаги
bool read_seq = false; 
bool read_meta = false;

// Массив для хранения последовательности нот произведения
// sequence[x][0] - нота
// sequence[x][1] - velocity (?)
// sequence[x][2] - длина
uint16_t sequence[1024][3];

// Размер расшифрованной части последовательности и её полный размер
uint16_t seq_length = 0; 
uint16_t total_seq_length = 0;

// Предыдущая и текущая ноты, нужны для контроля переходов в C# и D#
// Начинаем с С (номер ноль)
short previous_note = 0;
short current_note = 0;

// Даёт положения пальцев для выбранной ноты
short fingerings[22][11] = {
//   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1},  //C
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0},  //C#
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1},  //D
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 0, 1},  //D#
    {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1},  //E
    {1, 2, 1, 1, 1, 1, 0, 0, 0, 1, 1},  //F
    {1, 2, 1, 1, 1, 0, 1, 1, 1, 1, 1},  //F#
    {1, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1},  //G
    {1, 2, 1, 1, 0, 1, 1, 0, 0, 1, 1},  //G#
    {1, 2, 1, 0, 1, 1, 1, 0, 1, 1, 1},  //A
    {1, 2, 1, 0, 1, 0, 0, 0, 0, 1, 1},  //A#
    {1, 2, 0, 1, 0, 1, 0, 1, 0, 1, 1},  //B
    {1, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1},  //C
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},  //C#
    // дальше надо сильнее дуть, пока мы не можем
    {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1},  //to D# 1       14
    {1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 1},  //to D# 2       15
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1},  //to C# 1       16
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 0},  //to C# 2       17
    {1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 1},  //from D# 1     18
    {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1},  //from D# 2     19
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 0},  //from C# 1     20
    {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1}}; //from C# 2     21
    
// Положение серв пальцев при открытии и закрытии ноты
short positions[11][3] = {
// open, closed
    {0, 0},         //head
    {5, 20, 40},    //1
    {25, 50},       //2
    {60, 20},       //3
    {20, 55},       //4
    {70, 20},       //5
    {60, 20},       //6
    {35, 85},       //7
    {68, 20},       //8
    {70, 5},       //9
    {50, 10}};      //10

// Соответствие виртуальных и физических номеров серв
// Виртуальный - номер в массиве
// Физический - номер порта на Ардуино
//                0, 1, 2, 3, 4,  5,  6,  7,  8,  9, 10
short servos[] = {0, 1, 2, 3, 4, 13, 15, 12, 10, 14, 11};

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
    digitalWrite(PUMP, HIGH);
    digitalWrite(VALVE, LOW);
    
    // Выставляем голову в центр
    pwm.setPWM(HEAD_SERVO, 0, get_pulse(HEAD_FRONT));
    delay(100);
    // Берём ноту C (До), в этой ноте все пальцы закрыты
    pick_note(0);
    delay(100);
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

void loop()
{
    // 
    if (not read_seq && Serial.available() > 3)
    {   
        Serial.readBytes(buffer, 4);
        Serial.write(1);
        
        // При первом вхождении - читаем метаданные - длину последовательности
        if (not read_meta)
        {
            // Восстанавливаем число из двух байтов, первое домножаем на 256, прибавляем второе
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
                // Последовательность прочитана
                read_seq = true;  
                // Открываем клапан и включаем компрессор
                digitalWrite(VALVE, LOW);    
                digitalWrite(PUMP, LOW); 
                // Время на раздувание
                delay(4000);
            }
        }  
    }

    // Если есть прочитанная последовательность - начинаем играть
    if (read_seq)
    {
        for (int i = 0; i < seq_length; ++i)
        {
            // Задержка
            delay(sequence[i][2]);

            /*
            // Обновляем предыдущую и текущую ноту
            previous_note = current_note;
            current_note = sequence[i][0];

            // Переход в D#
            if (current_note == 3) {
                pick_note(14);
                delay(200);
                pick_note(15);
                delay(200); 
            }
            // Переход в С#
            if (current_note == 1) {
                pick_note(16);
                delay(200);
                pick_note(17);
                delay(200);
            }
            // Переход из D#
            if (previous_note == 3) {
                pick_note(18);
                delay(200);
                pick_note(19);
                delay(200); 
            }
            // Переход из С#
            if (previous_note == 1) {
                pick_note(20);
                delay(200);
                pick_note(21);
                delay(200); 
            }
            */

            // Взятие ноты
            pick_note(sequence[i][0]);
            
            // Действия с клапаном и компрессором (?)
            if (sequence[i][1] > 0)
            {
                // Открыли клапан
                digitalWrite(VALVE, HIGH);   
                // Включили компрессор 
                digitalWrite(PUMP, LOW);
            }
            else 
            {
                // Закрыли клапан
                digitalWrite(VALVE, LOW);
            }             
        }
    }
}