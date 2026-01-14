// per la connessione si usa ttyUSB0
// Definizione dei pin immaginando di avere l'USB di Arduino a sinistra
#define SHIFT_DATA 2  // riferito a nomenclatura GPIO, dunque pin D2 = 5
#define SHIFT_CLK 3   // riferito a nomenclatura GPIO, dunque pin D3 = 6
#define SHIFT_LATCH 4 // riferito a nomenclatura GPIO, dunque pin D4 = 7
#define EEPROM_D0 5   // D0 per la EEPROM, D5 del Nano
#define EEPROM_D7 12  // D7 per la EEPROM, D12 del Nano
#define WRITE_EN 13   // /W£ per la EEPROM, D13 del Nano

void setAddress(int address, bool outputEnable);
void eeprom_erase(byte value);
byte readEEPROM(int address);
void writeEEPROM(int address, byte data);
void printContents(int start, int lenght);
void eeprom_program();
void WriteOpcode(uint8_t opcode);
int16_t power(int base, int esponente);

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
  pinMode(SHIFT_DATA, OUTPUT); // da qui in poi setto la direzione "da Arduino verso HC595"
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, /*LOW*/ HIGH); // WE = HI e così facendo Arduino mette automaticamente un pull-up
  pinMode(WRITE_EN, OUTPUT);            // così quando attivo il pin, questo è già attivo HI
  pinMode(LED_BUILTIN, OUTPUT);         // per poter lampeggiare alla fine della programmazione
  Serial.println("\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("+++++    Seven Segment Display EEPROM programmer     +++++");
  Serial.println("+++++            for BEAM 8-bit Computer             +++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  // eeprom_erase(0x77);
  eeprom_program();
  printContents(0x00, 256);
  printContents(0x100, 256);
  printContents(0x200, 256);
}

// ************************************************************
// ********************* EEPROM ERASE *************************
// ************************************************************
void eeprom_erase(byte value)
{
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.println("Erasing EEPROM");
  for (int address = 0; address <= 4096; address += 1)
  {
    writeEEPROM(address, value);
    if (address % 64 == 0)
    {
      // Serial.print(".");
      Serial.print("Address ");
      Serial.println(address, HEX);
    }
  }
  Serial.println(" Done!");
}

// ************************************************************
// ******************** READ FROM EEPROM **********************
// ************************************************************
byte readEEPROM(int address)
{
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable, cioè in realtà setto /OE basso */ true);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1)
  {
    //    Serial.print(digitalRead(pin)); // ************************* //
    data = (data << 1) + digitalRead(pin);
  }
  //  Serial.print("\t");
  //  Serial.println(data);
  return data;
}

// ************************************************************
// *********************** SET ADDRESS ************************
// ************************************************************
// Uso del 595: prima metto il dato in D2 (SHIFT_DATA) e poi pulso D3 (SHIFT_CLK) per mandarlo
void setAddress(int address, bool outputEnable)
{ // se false = 0 = /OE abilitato
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00L : 0x80));
  // Utilizzo indirizzi da A0 a A9 inclusi; A8 e A9 sono settati dal FF per selezionare la cifra da accendere
  // Da A0 a A7 arrivano invece da D0 / D7 del bus dati, dunque un numero da 0 a 255 (0x00 a 0xFF)
  // Se outputEnable /OE = 0, attivo l'output, dunque leggo dalla EEPROM
  // Se outputEnable /OE = 1, disattivo l'output, dunque scrivo sulla EEPROM
  // Poiché uso "(outputEnable ? 0x00L: 0x80)", sto dicendo che se nella chiamate passo un "outputEnable" true,
  // setto a zero outputEnable /OE e dunque attivo la lettura; se passo un "outputEnable" false, il MSb è HI e dunque attivo la scrittura
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  // Quanto sotto sblocca il 595
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

// ************************************************************
// ******************** WRITE TO EEPROM ***********************
// ************************************************************
void writeEEPROM(int address, byte data)
{
  // Prima setto l'indirizzo ma soprattutto disabilito l'output della EEPROM e poi abilito l'output di Arduino, così non causo cortocircuiti
  // Credo che la EEPROM non abbia resistenze in uscita e dunque se ho un pin in output e Arduino in input faccio un cortocircuito
  // Se outputEnable = false posso scrivere sulla EEPROM
  setAddress(address, false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
    pinMode(pin, OUTPUT);
  }
  /*
      Serial.print("\n+++++++++++++++++++++++++++++\n+++++++++++++++++++++++++++++\n\nIndirizzo da scrivere = ");
      Serial.println(address);
      Serial.print("MSB (>> 8) = ");
      Serial.println(address >> 8);
      Serial.print("LSB (&& 255) = ");
      int address_lsb = address & 255;
      Serial.println(address_lsb);
      Serial.print("Valore 'data' in HEX = ");
      Serial.println(data, HEX);
      Serial.print("Valore 'data' in BIN = ");
      Serial.println(data, BIN);
      Serial.println("** Ora inizia il loop **");
    */
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
    digitalWrite(pin, data & 1); // scrivo il LSb
    /*
      Serial.print("Sul PIN ");
      Serial.print(pin);
      Serial.print(" scrivo ");
      Serial.println(data & 1);
    */
    data = data >> 1; // e shift a dx di una posizione così al prossimo loop prendo il bit successivo
  }
  //  Serial.println("** Loop completo **");
  //  Serial.println("** Ora Strobe WE **");
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1); // modificato 23.12.2023 era 1
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

// ************************************************************
// ***************** PRINT EEPROM CONTENTS ********************
// ************************************************************
void printContents(int start, int lenght)
{
  Serial.println("\n\n+++++++++++++++++++++++++++++");
  Serial.println("+++++++++++++++++++++++++++++");
  Serial.print("EEPROM Content $");
  Serial.print(start, HEX);
  Serial.print("- $");
  Serial.println(start + lenght - 1, HEX);
  for (int base = start, end = start + lenght; base < end; base += 16)
  {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1)
    {
      data[offset] = readEEPROM(base + offset);
    }
    char buf[80];
    sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
}

const int8_t digit = 17;
const int8_t segment = 8;

// ************************************************************
// *** 17 cifre  : 0-9 a-z .
// *** 8 segmenti: A B C D E F G DP
// *** totale    : 136 byte
// ************************************************************
// ***********************       A          *******************
// ***********************    ╔═════╗       *******************
// ***********************  F ║     ║ B     *******************
// ***********************    ╠═ G ═╣       *******************
// ***********************  E ║     ║ C     *******************
// ***********************    ╚═════╝ . DP  *******************
// ***********************       D          *******************
// ************************************************************

const int8_t segments[digit][segment] = {
    // A B  C  D  E  F  G  DP
    //<2^0>   <2^1>   <2^2>   <2^3>   <2^4>   <2^5>   <2^6>   <2^7>
    {1, 1, 1, 1, 1, 1, 0, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1, 0}, // 2
    {1, 1, 1, 1, 0, 0, 1, 0}, // 3
    {0, 1, 1, 0, 0, 1, 1, 0}, // 4
    {1, 0, 1, 1, 0, 1, 1, 0}, // 5
    {1, 0, 1, 1, 1, 1, 1, 0}, // 6
    {1, 1, 1, 0, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1, 0}, // 8
    {1, 1, 1, 1, 0, 1, 1, 0}, // 9
    {1, 1, 1, 0, 1, 1, 1, 0}, // a
    {0, 0, 1, 1, 1, 1, 1, 0}, // b
    {1, 0, 0, 1, 1, 1, 0, 0}, // c
    {0, 1, 1, 1, 1, 0, 1, 0}, // d
    {1, 0, 0, 1, 1, 1, 1, 0}, // e
    {1, 0, 0, 0, 1, 1, 1, 0}, // f
    {0, 0, 0, 0, 0, 0, 0, 1}, // .
};

// ************************************************************
// ************************************************************
// ******************** EEPROM PROGRAM ************************
// ************************************************************
// ************************************************************
// Per ogni valore presente sul bus (bus_value), ogni cifra dovrà mostrare un numero composto da centinaia, decine, unità
// Il bus_value va da 0 a 255, dunque per ogni valore c'è una combinazione unica di centinaia, decine, unità
// Numero   100    10     1
//    000     0     0     0
//    001     0     0     1
//    018     0     1     8
//    096     0     9     6
//    123     1     2     3
//    216     2     1     6
//
// ogni cifra dunque occupa 256 byte
//   0x00 a  0xff = unità
//  0x100 a 0x1ff = decine
//  0x200 a 0x2ff = centinaia
//

void eeprom_program()
{
  Serial.println("\n\n+++++++++++++++++++++++++++++");
  Serial.println("+++++++++++++++++++++++++++++");
  Serial.println("Programming EEPROM");
  for (int16_t bus_value = 0; bus_value < 256; bus_value++)
  {
    writeDigit(bus_value);
  }
  Serial.println("\nDone!");
}

// ************************************************************
// *************** PREPARA E SCRIVE SEGMENTI ******************
// ************************************************************
void writeDigit(uint8_t bus_value)
{
  // Sono 4 cifre, mappate $0-$0FF, $100-$1FF, $200-$2FF, $300-$3FF
  int8_t units = 0;    // unità
  int8_t tens = 0;     // decine
  int8_t hundreds = 0; // centinaia
  // Per ogni cifra, devo scrivere in EEPROM il valore che quella cifra deve assumere per ognuna delle 256 combinazioni di bus_value
  // 1) divido bus_value per 100 e ottengo il numero da scrivere nelle centinaia
  // 2) divido il resto per 10 e ottengo il numero da scrivere nelle decine
  // 3) il resto è il numero da scrivere nelle unità
  //
  // esempio 154      ==> 154 / 100 =  1        ==> scrivo 1 nelle centinaia
  //                  ==> 154 % 100 = 54
  //                  ==>  54 /  10 =  5        ==> scrivo 5 nelle decine
  //                  ==>  54 %  10 =  4        ==> scrivo 4 nelle unità
  //
  // esempio  85      ==>  85 / 100 =  0        ==> scrivo 0 nelle centinaia
  //                  ==>  85 % 100 = 85
  //                  ==>  85 /  10 =  8        ==> scrivo 8 nelle decine
  //                  ==>  85 %  10 =  5        ==> scrivo 5 nelle unità
  // In realtà non devo scrivere il numero, ma il valore che ricavo dalla tabella per ogni digit, ad esempio:
  // per la cifra 3 devo avere 2^0 + 2^1 + 2^2 + 2^3 + 2^6 = 1 + 2 + 4 + 8 + 64 = 79
  // per la cifra 7 devo avere 2^0 + 2^1 + 2^2             = 1 + 2 + 4          = 7
  for (int i = 0; i <= 7; i++)
  {
    hundreds += segments[bus_value / 100][i] * power(2, i);
    tens += segments[(bus_value % 100) / 10][i] * power(2, i);
    units += segments[(bus_value % 100) % 10][i] * power(2, i);
  }
  writeEEPROM(bus_value + 0x000, units);
  writeEEPROM(bus_value + 0x100, tens);
  writeEEPROM(bus_value + 0x200, hundreds);
  // Serial.println(" - ");
  // Serial.print("bus_value: ");
  // Serial.print(bus_value);
  // Serial.print(" - ");
  // Serial.print(hundreds);
  // Serial.print("/");
  // Serial.print(tens);
  // Serial.print("/");
  // Serial.print(units);
  // Serial.print(" /// ");
  if (bus_value % 4 == 0)
  {
    Serial.print(".");
  }
}

// ************************************************************
// ************************* POWER ****************************
// ************************************************************
int16_t power(int base, int esponente)
{
  int risultato = 1;
  for (int x = 0; x < esponente; x++)
  {
    risultato *= base;
  }
  return risultato;
}

/*
  // ********************************************
  // ******** EEPROM PROGRAM -- MASTER -- *******
  // ********************************************
  Serial.print("\nProgramming EEPROM ");
    for (int address = 0; address < 4096; address += 1) {
      int flags       = (address & 0b1100000000) >> 8;
      int byte_sel    = (address & 0b0010000000) >> 7;
      int instruction = (address & 0b0001111000) >> 3;
      int step        = (address & 0b0000000111);
      if (byte_sel) {
        writeEEPROM(address, microcode[instruction][step]);
      } else {
        writeEEPROM(address, microcode[instruction][step] >> 8);
      }
      if (address % 64 == 0) {
        Serial.print(".");
      }
    }
  Serial.println(" Done!");
*/

void testWriteRead()
{
  // 11.12.2023 Questo codice funziona e scrive da 0 a FF nelle celle da 0 a FF e ripete per tutti i blocchi da 256
  // Dunque il fatto che altrove io ottenga garbage significa che sto sbagliando qualcosa
  for (int counter = 0; counter < 256; counter += 1)
  {
    int address = 256 * 1 + counter;
    writeEEPROM(address, counter);
    if (address % 16 == 0)
    {
      Serial.print("\naddress / instruction = ");
      Serial.print(address, HEX);
    }
    // if (address % 64 == 0) {
    //   Serial.print(".");
    // }
    // }
  }
  Serial.println("\nDone!");
  // printContents(0, 512);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(800);
}