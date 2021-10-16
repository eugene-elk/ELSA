# ELSA
ELSA, The Robot Playing Flute

Работает под управлением Arduino и компьютера, связь по USB через Serial.

<code>![ELSA](https://sun9-78.userapi.com/impf/c844720/v844720010/1f8977/D4m9H9WjYKg.jpg?size=600x400&quality=96&sign=819ebeb6bf7d0f5a9ba6cc82c2322873&type=album "ELSA")
</code>

## Программы ##

### Тестирование нот ###
Arduino: 
        
        fingers_test.ino  

Python: 

        ard_test.py - старая программа для тестирования
        
        finger_test.py - имеет сохранённые настройки, нет необходимости зававать конкретное значение пальца

### Проигрывание midi-файла ###
Arduino: 

        old_protocol.ino - работает, ноты в массиве fingerings

        new_protocol.ino - для работы с классом note

Python: 
        
        play_midi.py

### Импровизация ###
Arduino: 
        
        main.ino  

Python: 
        
        main.py


## TODO ##
* ~~Единый файл с настройками нот и пальцев~~
* ~~Удобная тестовая программа с возможностью открывать и закрывать пальцы по сохраненным значениям, а также брать сохранённые ноты~~
* Общий json для настроек на Arduino и в тестовых программах
* Избавиться от delay, использовать millis
* Объект "нота" вместо массива fingerings
* Отладить движения пальцами для C# и D#
