# 1 "/home/pi/8-bit_computer/5-bootloader/5-bootloader.ino"
# 18 "/home/pi/8-bit_computer/5-bootloader/5-bootloader.ino"
const int MEMORY_ADDRESSES = 16;
const int MEMORY_ADDRESSES_PINS = 4;
const int MEMORY_PINS = 8;
const int LEDS = 27; // array for KITT supercar LEDs effect
const int PINS = 8;

/*
   Fibonacci program.
   References: - https://youtu.be/a73ZXDJtU48?t=263
   Restart from scratch when Carry is set (calculation > 255)
*/

// Best option as of 13.08.2022. Sequence start at 1; end at 233.
int FIBONACCI_PROGRAM_4[MEMORY_ADDRESSES /*4 digits*/][MEMORY_ADDRESSES_PINS /*4 digits*/ + MEMORY_PINS /*4 digits*/] = {
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1 }, // $0   LDI #1
  { 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0 }, // $1   STA $14
  { 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, // $2   LDI #0
  { 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0 }, // $3   ADD $14
  { 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1 }, // $4   STA $15
  { 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0 }, // $5   JC  $0
  { 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0 }, // $6   OUT
  { 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0 }, // $7   LDA $14
  { 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1 }, // $8   STA $13
  { 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1 }, // $9   LDA $15
  { 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0 }, // $10  STA $14
  { 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1 }, // $11  LDA $13
  { 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1 }, // $12  JMP $3
  { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // $13
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // $14
  { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // $15
  //  { 1, 1, 1, 1,    1, 0, 1, 0,    1, 0, 1, 0 }, // $15 PATTERN for test only
};

int COUNTER_PROGRAM[MEMORY_ADDRESSES][MEMORY_ADDRESSES_PINS + MEMORY_PINS] = {
  { 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1 }, // $0   ADD $15
  { 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 }, // $1   OUT
  { 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0 }, // $2   JC  $4
  { 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0 }, // $3   JMP $0
  { 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 }, // $4   SUB $15
  { 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 }, // $5   OUT
  { 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // $6   JZ  $0
  { 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0 }, // $7   JMP $4
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // $8
  { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // $9
  { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // $10
  { 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // $11
  { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // $12
  { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // $13
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // $14
  { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 }, // $15   #$01
};


/*
   Set Arduino pins for computer Program Mode
*/
void set_for_programming() {
  digitalWrite(A1 /* Connected to the switch that selects Run or Program Mode*/, 0x1); // Program pin          (HI for Program Mode, LO for Run mode)
  digitalWrite(A0 /* Connected to pin 6 on the 3rd 555 and it can stop the clock*/, 0x1); // Clock pin            on/off (clock active LO)
  digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x1); // Write to Memory pin  (active LO) (push button)
  digitalWrite(A3 /* Connected to the reset button*/, 0x1); // Reset pin            (active LO)

  pinMode(A1 /* Connected to the switch that selects Run or Program Mode*/, 0x1);
  pinMode(A0 /* Connected to pin 6 on the 3rd 555 and it can stop the clock*/, 0x1);
  pinMode(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x1);
  pinMode(A3 /* Connected to the reset button*/, 0x1);

  pinMode(2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/, 0x1);
  pinMode(3, 0x1);
  pinMode(4, 0x1);
  pinMode(5, 0x1);
  pinMode(6, 0x1);
  pinMode(7, 0x1);
  pinMode(8, 0x1);
  pinMode(9, 0x1);
  pinMode(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, 0x1);
  pinMode(11, 0x1);
  pinMode(12, 0x1);
  pinMode(13, 0x1);
  digitalWrite(A1 /* Connected to the switch that selects Run or Program Mode*/, 0x0);
}

/*
   Set Arduino pins for computer Run Mode
*/
void post_programming() {
  digitalWrite(A1 /* Connected to the switch that selects Run or Program Mode*/, 0x1);
  //~ delay(10);
  digitalWrite(A0 /* Connected to pin 6 on the 3rd 555 and it can stop the clock*/, 0x0);
  digitalWrite(A3 /* Connected to the reset button*/, 0x1);
  digitalWrite(A3 /* Connected to the reset button*/, 0x0);
  delay(10);
  // All pins in input mode, so they get into High Impedance (hidden from the computer)
  pinMode(A1 /* Connected to the switch that selects Run or Program Mode*/, 0x0);
  pinMode(A0 /* Connected to pin 6 on the 3rd 555 and it can stop the clock*/, 0x0);
  pinMode(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x0);
  pinMode(A3 /* Connected to the reset button*/, 0x0);
  pinMode(2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/, 0x0);
  pinMode(3, 0x0);
  pinMode(4, 0x0);
  pinMode(5, 0x0);
  pinMode(6, 0x0);
  pinMode(7, 0x0);
  pinMode(8, 0x0);
  pinMode(9, 0x0);
  pinMode(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, 0x0);
  pinMode(11, 0x0);
  pinMode(12, 0x0);
  pinMode(13, 0x0);
}

/*
   Write a program to memory.
*/
void writeProgram(int program[MEMORY_ADDRESSES][MEMORY_ADDRESSES_PINS + MEMORY_PINS]) {
  for (int command = 0; command < MEMORY_ADDRESSES; command += 1) {
    digitalWrite(13, program[command][0]);
    digitalWrite(12, program[command][1]);
    digitalWrite(11, program[command][2]);
    digitalWrite(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, program[command][3]);
    digitalWrite(9, program[command][4]);
    digitalWrite(8, program[command][5]);
    digitalWrite(7, program[command][6]);
    digitalWrite(6, program[command][7]);
    digitalWrite(5, program[command][8]);
    digitalWrite(4, program[command][9]);
    digitalWrite(3, program[command][10]);
    digitalWrite(2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/, program[command][11]);
    digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x0);
    delayMicroseconds(10);
    digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x1);
    delay(150);
  }
}


////////////////////////////////////////////////////////////
////////////// KITT supercar LEDs effect ///////////////////
////////////////////////////////////////////////////////////

int KITT[LEDS][PINS] = {

  { 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 0, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 1, 1, 1, 1 },
  { 0, 0, 0, 1, 1, 1, 1, 1 },
  { 0, 0, 1, 1, 1, 1, 1, 0 },
  { 0, 1, 1, 1, 1, 1, 0, 0 },
  { 1, 1, 1, 1, 1, 0, 0, 0 },
  { 1, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 0, 0, 0, 0, 0 },
  { 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 0 },
  { 0, 0, 0, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
};
/*
  { 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 1, 0, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  //~ { 0, 0, 0, 0, 0, 0, 0, 1 },
  //~ { 0, 0, 0, 0, 0, 0, 0, 0 },
};
*/
/*
   LEDs moving as KITT supercar; shown at end of programming
*/
void writeKitt(int led_array[LEDS][PINS]) {
  //~ Save content of memory address $15 because it is used for LED effect
  digitalWrite(13, 1);
  digitalWrite(12, 1);
  digitalWrite(11, 1);
  digitalWrite(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, 1);
  for (int pin = 9; pin >= 2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/; pin -= 1) {
    pinMode(pin, 0x0);
    led_array[27][pin] = digitalRead(pin); // this is where I save the value of location $15
    pinMode(pin, 0x1);
//    Serial.println("\n+++++++++++++++++++++++++++++");
//    Serial.print("Numero pin: ");
//    Serial.print(pin);
//    Serial.print("; ");
//    Serial.print("Valore: ");
//    Serial.println(led_array[16][pin]);
  }

  //~ Actual LEDs effect
  for (int times = 1; times <= 3; times += 1) {
    for (int led_sequence = 0; led_sequence < LEDS; led_sequence += 1) {
      digitalWrite(13, 1);
      digitalWrite(12, 1);
      digitalWrite(11, 1);
      digitalWrite(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, 1);
      digitalWrite(9, led_array[led_sequence][0]);
      digitalWrite(8, led_array[led_sequence][1]);
      digitalWrite(7, led_array[led_sequence][2]);
      digitalWrite(6, led_array[led_sequence][3]);
      digitalWrite(5, led_array[led_sequence][4]);
      digitalWrite(4, led_array[led_sequence][5]);
      digitalWrite(3, led_array[led_sequence][6]);
      digitalWrite(2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/, led_array[led_sequence][7]);
      digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x0);
      delayMicroseconds(100);
      digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x1);
      delay(45);
    }
  }
  //~ Restore content of memory address $15
  digitalWrite(13, 1);
  digitalWrite(12, 1);
  digitalWrite(11, 1);
  digitalWrite(10 /* This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.*/, 1);
  for (int pin = 9; pin >= 2 /* This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.*/; pin -= 1) {
    digitalWrite(pin, led_array[27][pin]);
    digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x0);
    delayMicroseconds(10);
    digitalWrite(A2 /* Connected to the button that writes to Memory (pin 14 of the '157)*/, 0x1);
    delay(10);
  }
  //~ delay(500);
}

void setup() {
  Serial.begin(115200);
  delay(300);
}

void loop() {
  set_for_programming();
  writeProgram(FIBONACCI_PROGRAM_4);
  writeKitt(KITT);
  post_programming();
  delay(60000);

  set_for_programming();
  writeProgram(COUNTER_PROGRAM);
  writeKitt(KITT);
  post_programming();
  delay(360000);
}
