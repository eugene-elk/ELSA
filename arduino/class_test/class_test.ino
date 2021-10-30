// Программа для тестирования класса Нота

short finger_notes[14][11] = {
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
};

class note {
  public:
    String name;
    short fingerings[11] = {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    short fingerings_size;

    note(String _name, short* _fingerings, int _fin_len) {
      name = _name;
      fingerings_size = _fin_len / sizeof(_fingerings[0]);
      for (short i = 0; i < fingerings_size; i++) {
        fingerings[i] = _fingerings[i];
      }
    }

    note() { // По умолчанию создаём ноту До (C)
      name = "C";
      fingerings_size = 11;
    }

    void show_info() {
      Serial.println(name);
      for (short i = 0; i < fingerings_size; i++){
        Serial.println(fingerings[i]);
        delay(50);
      }
    }
};

note flex;
note flex2("D", finger_notes[1], sizeof(finger_notes[1]));

note notes[14] = {
    note("C",   finger_notes[0], sizeof(finger_notes[0])), 
    note("C#",  finger_notes[1], sizeof(finger_notes[1])), 
    note("D",   finger_notes[2], sizeof(finger_notes[2])),  
    note("D#",  finger_notes[3], sizeof(finger_notes[3])),  
    note("E",   finger_notes[4], sizeof(finger_notes[4])),  
    note("F",   finger_notes[5], sizeof(finger_notes[5])),  
    note("F#",  finger_notes[6], sizeof(finger_notes[6])),  
    note("G",   finger_notes[7], sizeof(finger_notes[7])),  
    note("G#",  finger_notes[8], sizeof(finger_notes[8])),  
    note("A",   finger_notes[9], sizeof(finger_notes[9])),  
    note("A#",  finger_notes[10], sizeof(finger_notes[10])),  
    note("B",   finger_notes[11], sizeof(finger_notes[11])),  
    note("C_2", finger_notes[12], sizeof(finger_notes[12])),  
    note("C#_2",finger_notes[13], sizeof(finger_notes[13]))  
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  flex.show_info();
  flex2.show_info();
  notes[4].show_info();
  notes[12].show_info();
}

void loop() 
{   
   delay(1000);
}
