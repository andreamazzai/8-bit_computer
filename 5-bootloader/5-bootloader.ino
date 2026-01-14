#define MEMORY_PROGRAMMING_MODE A1  // Connected to the switch that selects Run or Program Mode
#define WRITE_TO_MEMORY A2          // Connected to the button that writes to Memory (pin 14 of the '157)
#define RESET_COMPUTER A3           // Connected to the reset button
#define START_STOP_CLOCK A0         // Connected to pin 6 on the 3rd 555 and it can stop the clock
#define MEMORY_ADDRESS_PIN_0 10     // This and the next 3 are connected to the '157 of the MAR register, same pins of the dip switch.
#define MEMORY_ADDRESS_PIN_1 11
#define MEMORY_ADDRESS_PIN_2 12
#define MEMORY_ADDRESS_PIN_3 13
#define MEMORY_PIN_0 2              // This and the next 7 are connected to the '157s of the RAM register, same pins of the dip switch.
#define MEMORY_PIN_1 3
#define MEMORY_PIN_2 4
#define MEMORY_PIN_3 5
#define MEMORY_PIN_4 6
#define MEMORY_PIN_5 7
#define MEMORY_PIN_6 8
#define MEMORY_PIN_7 9

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
  { 0, 0, 0, 0,    0, 1, 1, 0,    0, 0, 0, 1 }, // $0   LDI #1
  { 0, 0, 0, 1,    0, 1, 0, 1,    1, 1, 1, 0 }, // $1   STA $14
  { 0, 0, 1, 0,    0, 1, 1, 0,    0, 0, 0, 0 }, // $2   LDI #0
  { 0, 0, 1, 1,    0, 0, 1, 0,    1, 1, 1, 0 }, // $3   ADD $14
  { 0, 1, 0, 0,    0, 1, 0, 1,    1, 1, 1, 1 }, // $4   STA $15
  { 0, 1, 0, 1,    0, 1, 1, 1,    0, 0, 0, 0 }, // $5   JC  $0
  { 0, 1, 1, 0,    1, 1, 1, 0,    0, 0, 0, 0 }, // $6   OUT
  { 0, 1, 1, 1,    0, 0, 0, 1,    1, 1, 1, 0 }, // $7   LDA $14
  { 1, 0, 0, 0,    0, 1, 0, 1,    1, 1, 0, 1 }, // $8   STA $13
  { 1, 0, 0, 1,    0, 0, 0, 1,    1, 1, 1, 1 }, // $9   LDA $15
  { 1, 0, 1, 0,    0, 1, 0, 1,    1, 1, 1, 0 }, // $10  STA $14
  { 1, 0, 1, 1,    0, 0, 0, 1,    1, 1, 0, 1 }, // $11  LDA $13
  { 1, 1, 0, 0,    0, 1, 0, 0,    0, 0, 1, 1 }, // $12  JMP $3
  { 1, 1, 0, 1,    0, 0, 0, 0,    0, 0, 0, 0 }, // $13
  { 1, 1, 1, 0,    0, 0, 0, 0,    0, 0, 0, 0 }, // $14
  { 1, 1, 1, 1,    0, 0, 0, 0,    0, 0, 0, 0 }, // $15
  //  { 1, 1, 1, 1,    1, 0, 1, 0,    1, 0, 1, 0 }, // $15 PATTERN for test only
};

int COUNTER_PROGRAM[MEMORY_ADDRESSES][MEMORY_ADDRESSES_PINS + MEMORY_PINS] = {
  { 0, 0, 0, 0,    0, 0, 1, 0,    1, 1, 1, 1 }, // $0   ADD $15
  { 0, 0, 0, 1,    1, 1, 1, 0,    0, 0, 0, 0 }, // $1   OUT
  { 0, 0, 1, 0,    0, 1, 1, 1,    0, 1, 0, 0 }, // $2   JC  $4
  { 0, 0, 1, 1,    0, 1, 0, 0,    0, 0, 0, 0 }, // $3   JMP $0
  { 0, 1, 0, 0,    0, 0, 1, 1,    1, 1, 1, 1 }, // $4   SUB $15
  { 0, 1, 0, 1,    1, 1, 1, 0,    0, 0, 0, 0 }, // $5   OUT
  { 0, 1, 1, 0,    1, 0, 0, 0,    0, 0, 0, 0 }, // $6   JZ  $0
  { 0, 1, 1, 1,    0, 1, 0, 0,    0, 1, 0, 0 }, // $7   JMP $4
  { 1, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0 }, // $8
  { 1, 0, 0, 1,    0, 0, 0, 0,    0, 0, 0, 0 }, // $9
  { 1, 0, 1, 0,    0, 0, 0, 0,    0, 0, 0, 0 }, // $10
  { 1, 0, 1, 1,    0, 0, 0, 0,    0, 0, 0, 0 }, // $11
  { 1, 1, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0 }, // $12
  { 1, 1, 0, 1,    0, 0, 0, 0,    0, 0, 0, 0 }, // $13
  { 1, 1, 1, 0,    0, 0, 0, 0,    0, 0, 0, 0 }, // $14
  { 1, 1, 1, 1,    0, 0, 0, 0,    0, 0, 0, 1 }, // $15   #$01
};


/*
   Set Arduino pins for computer Program Mode
*/
void set_for_programming() {
  digitalWrite(MEMORY_PROGRAMMING_MODE, HIGH);    // Program pin          (HI for Program Mode, LO for Run mode)
  digitalWrite(START_STOP_CLOCK, HIGH);           // Clock pin            on/off (clock active LO)
  digitalWrite(WRITE_TO_MEMORY, HIGH);            // Write to Memory pin  (active LO) (push button)
  digitalWrite(RESET_COMPUTER, HIGH);             // Reset pin            (active LO)

  pinMode(MEMORY_PROGRAMMING_MODE, OUTPUT);
  pinMode(START_STOP_CLOCK, OUTPUT);
  pinMode(WRITE_TO_MEMORY, OUTPUT);
  pinMode(RESET_COMPUTER, OUTPUT);

  pinMode(MEMORY_PIN_0, OUTPUT);
  pinMode(MEMORY_PIN_1, OUTPUT);
  pinMode(MEMORY_PIN_2, OUTPUT);
  pinMode(MEMORY_PIN_3, OUTPUT);
  pinMode(MEMORY_PIN_4, OUTPUT);
  pinMode(MEMORY_PIN_5, OUTPUT);
  pinMode(MEMORY_PIN_6, OUTPUT);
  pinMode(MEMORY_PIN_7, OUTPUT);
  pinMode(MEMORY_ADDRESS_PIN_0, OUTPUT);
  pinMode(MEMORY_ADDRESS_PIN_1, OUTPUT);
  pinMode(MEMORY_ADDRESS_PIN_2, OUTPUT);
  pinMode(MEMORY_ADDRESS_PIN_3, OUTPUT);
  digitalWrite(MEMORY_PROGRAMMING_MODE, LOW);
}

/*
   Set Arduino pins for computer Run Mode
*/
void post_programming() {
  digitalWrite(MEMORY_PROGRAMMING_MODE, HIGH);
  //~ delay(10);
  digitalWrite(START_STOP_CLOCK, LOW);
  digitalWrite(RESET_COMPUTER, HIGH);
  digitalWrite(RESET_COMPUTER, LOW);
  delay(10);
  // All pins in input mode, so they get into High Impedance (hidden from the computer)
  pinMode(MEMORY_PROGRAMMING_MODE, INPUT);
  pinMode(START_STOP_CLOCK, INPUT);
  pinMode(WRITE_TO_MEMORY, INPUT);
  pinMode(RESET_COMPUTER, INPUT);
  pinMode(MEMORY_PIN_0, INPUT);
  pinMode(MEMORY_PIN_1, INPUT);
  pinMode(MEMORY_PIN_2, INPUT);
  pinMode(MEMORY_PIN_3, INPUT);
  pinMode(MEMORY_PIN_4, INPUT);
  pinMode(MEMORY_PIN_5, INPUT);
  pinMode(MEMORY_PIN_6, INPUT);
  pinMode(MEMORY_PIN_7, INPUT);
  pinMode(MEMORY_ADDRESS_PIN_0, INPUT);
  pinMode(MEMORY_ADDRESS_PIN_1, INPUT);
  pinMode(MEMORY_ADDRESS_PIN_2, INPUT);
  pinMode(MEMORY_ADDRESS_PIN_3, INPUT);
}

/*
   Write a program to memory.
*/
void writeProgram(int program[MEMORY_ADDRESSES][MEMORY_ADDRESSES_PINS + MEMORY_PINS]) {
  for (int command = 0; command < MEMORY_ADDRESSES; command += 1) {
    digitalWrite(MEMORY_ADDRESS_PIN_3,  program[command][0]);
    digitalWrite(MEMORY_ADDRESS_PIN_2,  program[command][1]);
    digitalWrite(MEMORY_ADDRESS_PIN_1,  program[command][2]);
    digitalWrite(MEMORY_ADDRESS_PIN_0,  program[command][3]);
    digitalWrite(MEMORY_PIN_7,          program[command][4]);
    digitalWrite(MEMORY_PIN_6,          program[command][5]);
    digitalWrite(MEMORY_PIN_5,          program[command][6]);
    digitalWrite(MEMORY_PIN_4,          program[command][7]);
    digitalWrite(MEMORY_PIN_3,          program[command][8]);
    digitalWrite(MEMORY_PIN_2,          program[command][9]);
    digitalWrite(MEMORY_PIN_1,          program[command][10]);
    digitalWrite(MEMORY_PIN_0,          program[command][11]);
    digitalWrite(WRITE_TO_MEMORY, LOW);
    delayMicroseconds(10);
    digitalWrite(WRITE_TO_MEMORY, HIGH);
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
  digitalWrite(MEMORY_ADDRESS_PIN_3, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_2, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_1, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_0, 1);
  for (int pin = MEMORY_PIN_7; pin >= MEMORY_PIN_0; pin -= 1) {
    pinMode(pin, INPUT);
    led_array[27][pin] = digitalRead(pin); // this is where I save the value of location $15
    pinMode(pin, OUTPUT);
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
      digitalWrite(MEMORY_ADDRESS_PIN_3, 1);
      digitalWrite(MEMORY_ADDRESS_PIN_2, 1);
      digitalWrite(MEMORY_ADDRESS_PIN_1, 1);
      digitalWrite(MEMORY_ADDRESS_PIN_0, 1);
      digitalWrite(MEMORY_PIN_7, led_array[led_sequence][0]);
      digitalWrite(MEMORY_PIN_6, led_array[led_sequence][1]);
      digitalWrite(MEMORY_PIN_5, led_array[led_sequence][2]);
      digitalWrite(MEMORY_PIN_4, led_array[led_sequence][3]);
      digitalWrite(MEMORY_PIN_3, led_array[led_sequence][4]);
      digitalWrite(MEMORY_PIN_2, led_array[led_sequence][5]);
      digitalWrite(MEMORY_PIN_1, led_array[led_sequence][6]);
      digitalWrite(MEMORY_PIN_0, led_array[led_sequence][7]);
      digitalWrite(WRITE_TO_MEMORY, LOW);
      delayMicroseconds(100);
      digitalWrite(WRITE_TO_MEMORY, HIGH);
      delay(45);
    }
  }
  //~ Restore content of memory address $15
  digitalWrite(MEMORY_ADDRESS_PIN_3, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_2, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_1, 1);
  digitalWrite(MEMORY_ADDRESS_PIN_0, 1);
  for (int pin = MEMORY_PIN_7; pin >= MEMORY_PIN_0; pin -= 1) {
    digitalWrite(pin, led_array[27][pin]);
    digitalWrite(WRITE_TO_MEMORY, LOW);
    delayMicroseconds(10);
    digitalWrite(WRITE_TO_MEMORY, HIGH);
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
