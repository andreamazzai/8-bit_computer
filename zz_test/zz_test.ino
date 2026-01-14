#include "Arduino.h"
// per la connessione si usa ttyUSB0

// Definizione dei pin immaginando di avere l'USB di Arduino a sinistra
#define SHIFT_DATA  2 // riferito a nomenclatura GPIO, dunque pin D2 = 5
#define SHIFT_CLK   3 // riferito a nomenclatura GPIO, dunque pin D3 = 6
#define SHIFT_LATCH 4 // riferito a nomenclatura GPIO, dunque pin D4 = 7
#define EEPROM_D0 5   // D0 per la EEPROM, D5 del Nano
#define EEPROM_D7 12  // D7 per la EEPROM, D12 del Nano
#define WE 14         // /WE per la EEPROM, A0/D14 del Nano
#define OE 15         // /OE per la EEPROM, A1/D15 del Nano
#define CE 16         // /CE per la EEPROM, A2/D16 del Nano

// void initMicroCodeBlock(int block);
void setAddress(int address, bool outputEnable);
void setDataBusMode(byte mode);
void strobeWE();
void enableOutput();
void disableOutput();
void enableChip();
void disableChip();
void writeOpcode(uint8_t opcode);
void writeEEPROM(int address, byte data);
byte readEEPROM(int address);
byte readDataBus();
void eeprom_erase(byte value);
void eeprom_program();
void eepromSmallWrite(byte value);
void buildInstruction(uint8_t opcode);
void printStep(uint8_t step, uint16_t address, uint8_t rom);
void printInstruction(uint8_t rom, uint16_t opcode, uint8_t step);
void printContents(int start, int lenght);
void printOpcodeContents(uint8_t opcode);
bool waitForWriteCycleEnd(byte lastValue, byte* b1Ptr);
void unlock();
void lock();
uint16_t calcCRC16_pre();
uint16_t calcCRC16_post();
uint16_t calculate_crc(uint8_t data, uint16_t crc, uint16_t polynomial);
uint16_t crc = 0xFFFF, crc_pre, crc_post;

// ************************************************************
// ******************* CODICE PRINCIPALE **********************
// ************************************************************
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }

  pinMode(LED_BUILTIN, OUTPUT); // per poter lampeggiare alla fine della programmazione
  Serial.println("\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("+++++          Microcode EEPROM programmer           +++++");
  Serial.println("+++++            for BEAM 8-bit Computer             +++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("\n+++++++++++++++++++++++++++++");
  // first();
  second();
  Serial.println("\nFatto!");
}

void first()
{
  for (uint8_t opcode = 0; opcode < 256; opcode++)
  {
      Serial.print(".");
      Serial.print(opcode);
    delayMicroseconds(300);
  }
}

void second()
{
  byte opcode = 255;
  do
  {
    Serial.print(".");
    Serial.print(opcode);
    opcode--;
  } while (opcode != 55);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(800);
}