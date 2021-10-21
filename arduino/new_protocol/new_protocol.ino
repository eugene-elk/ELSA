#include <Adafruit_PWMServoDriver.h>

// Максимальный и минимальный углы поворота сервы
#define SERVOMIN 80
#define SERVOMAX 600

// Концевики на движение рук вверх и вниз
#define UPBORDER 9
#define DOWNBORDER 5 

// Моторы на поднятие и опускание рук
#define ARMSMOTORUP 50
#define ARMSMOTORDOWN 48

// Порт клапана
#define VALVE 13 //HIGH -- off, LOW -- on

// Порт компрессора
#define PUMP 17 //HIGH -- off, LOW -- on

// хз что это
#define LCD 3

// Номер сервы головы
const short HEAD_SERVO = 0; 

// Положения сервы головы
const short HEAD_FRONT = 47;
const short HEAD_LEFT = 30;

class note {

    note() // По умолчанию создаём ноту До (C)
    {
        name = "C";
        fingerings = {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    }

    public:
        String name;
        short[11] fingerings;

        note(String _name, short[11] _fingerings) {
            name = _name;
            fingerings = _fingerings;
        }
}

// Даёт положения пальцев для выбранной ноты
note fingerings[14] = {
    note("C",   {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1}), 
    note("C#",  {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0}), 
    note("D",   {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1}),  
    note("D#",  {1, 2, 1, 1, 1, 1, 1, 1, 0, 0, 1}),  
    note("E",   {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1}),  
    note("F",   {1, 2, 1, 1, 1, 1, 0, 0, 0, 1, 1}),  
    note("F#",  {1, 2, 1, 1, 1, 0, 1, 1, 1, 1, 1}),  
    note("G",   {1, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1}),  
    note("G#",  {1, 2, 1, 1, 0, 1, 1, 0, 0, 1, 1}),  
    note("A",   {1, 2, 1, 0, 1, 1, 1, 0, 1, 1, 1}),  
    note("A#",  {1, 2, 1, 0, 1, 0, 0, 0, 0, 1, 1}),  
    note("B",   {1, 2, 0, 1, 0, 1, 0, 1, 0, 1, 1}),  
    note("C_2", {1, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1}),  
    note("C#_2",{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1})
}

    
// Положение серв пальцев при открытии и закрытии ноты
short positions[11][3] = {
// open, closed
    {0, 0},     //head
    {5, 20, 40},   //1
    {25, 50},   //2
    {60, 20},   //3
    {20, 55},   //4
    {70, 20},   //5
    {60, 20},   //6
    {40, 65},   //7
    {70, 20},   //8
    {30, 10},   //9
    {22, 5}};  //10

// Соответствие виртуальных и физических номеров серв
// Виртуальный - номер в массиве
// Физический - номер порта на Ардуино
//                0, 1, 2, 3, 4,  5,  6,  7,  8,  9, 10
short servos[] = {0, 1, 2, 3, 4, 13, 15, 12, 10, 14, 11};

//---------------------------------------------------------------------------------------------------------------------------------------------------

// Создаем сервоконтроллер
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

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

// Взятие ноты через объект ноты
void pick_note(note note)
{
    for (int i = 0; i < 11; i++) 
    {
        if (i == HEAD_SERVO) 
            continue;
        
        int p = get_pulse(note.fingerings[i]);

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

            // Обновляем предыдущую и текущую ноту
            previous_note = current_note;
            current_note = sequence[i][0];

            // Переход в D#
            if (current_note == 3) {
                note to_D_dies_1("to D# 1", {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1});
                note to_D_dies_2("to D# 2", {1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 1});
                pick_note(to_D_dies_1);
                delay(20);
                pick_note(to_D_dies_2);
                delay(20); 
            }
            // Переход в С#
            if (current_note == 1) {
                note to_C_dies_1("to C# 1", {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1});
                note to_C_dies_2("to C# 2", {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 0});
                pick_note(to_C_dies_1);
                delay(20);
                pick_note(to_C_dies_2);
                delay(20); 
            }
            // Переход из D#
            if (previous_note == 3) {
                note from_D_dies_1("from D# 1", {1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 1});
                note from_D_dies_2("from D# 2", {1, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1});
                pick_note(from_D_dies_1);
                delay(20);
                pick_note(from_D_dies_2);
                delay(20); 
            }
            // Переход из С#
            if (previous_note == 1) {
                note from_C_dies_1("from C# 1", {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 0});
                note from_C_dies_2("from C# 2", {1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1});
                pick_note(from_C_dies_1);
                delay(20);
                pick_note(from_C_dies_2);
                delay(20); 
            }

            // Взятие ноты
            pick_note(fingerings[sequence[i][0]]);
            
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