// ************************************************************
// ************************************************************
// ****** PROGRAMMAZIONE MICROCODE EEPROM SAP BEN EATER *******
// ************************************************************
// ************************************************************
// Set di istruzioni base, non comprende i flag e dunque la gestione dei salti condizionali

#define SHIFT_DATA  2 // riferito a nomenclatura GPIO, dunque pin D2 = 20
#define SHIFT_CLK   3 // riferito a nomenclatura GPIO, dunque pin D3 = 21
#define SHIFT_LATCH 4 // riferito a nomenclatura GPIO, dunque pin D4 = 22
#define EEPROM_D0 5   // D0 (pin 9 su 28C16) della EEPROM, 23/D5 del nano
#define EEPROM_D7 12  // D12 (pin 17 su 28C16) della EEPROM, 30/D12 del nano
#define WRITE_EN 13   // 21 della EEPROM, 1 / D13 del Nano

// prima metto il dato in D2 (SHIFT_DATA) e poi pulso D3 (SHIFT_CLK) per mandarlo

void setAddress(int address, bool outputEnable) { // se false = 0 = /OE abilitato
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  // Qui vedrò sullo scope il SER e il SRCLK
  // L'indirizzo arriva in due byte. Quanto sopra gestisce la parte alta dell'indirizzo (che
  // sono i bit da A8 a A10, e poi aggiunge il bit 15, che controlla /OE
  // se il pin /OE = 0, attivo l'output, dunque leggo dalla EEPROM
  // se il pin /OE = 1, disattivo l'output, dunque scrivo sulla EEPROM
  // poiché uso "(outputEnable ? 0x00 : 0x80)", sto dicendo che se nella routine passo un "outputEnable" true,
  // setto a zero /OE e dunque attivo la lettura; se passo un "outputEnable" false, il MSb è HI e dunque attivo la scrittura
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address );
  // Quanto sopra gestisce la parte bassa dell'indirizzo (da A0 ad A7)
  // Quanto sotto "triggera" il 595 per mettere in output quanto ha finora ricevuto in input
  // Qui vedrò sullo scope il RCLK
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}


// ********************************************
// ************ READ FROM EEPROM **************
// ********************************************
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable, cioè in realtà setto /OE basso */ true);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    //    Serial.print(digitalRead(pin)); // ************************* //
    data = (data << 1) + digitalRead(pin);
  }
  //  Serial.print("\t");
  //  Serial.println(data);
  return data;
}


// ********************************************
// ************ WRITE TO EEPROM ***************
// ********************************************
void writeEEPROM(int address, byte data) {
  //  prima setto l'indirizzo ma soprattutto disabilito l'output della EEPROM e poi abilito l'output di Arduino, così non causo cortocircuiti
  //  credo che la EEPROM non abbia resistenze in uscita e dunque se ho un pin in output e Arduino in input faccio un cortocircuito
  setAddress(address, /*outputEnable */ false /* cioè porto a uno /OE*. cosi posso scrivere sulla EEPROM*/);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
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

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1); // scrivo il LSb
    /*
      Serial.print("Sul PIN ");
      Serial.print(pin);
      Serial.print(" scrivo ");
      Serial.println(data & 1);
    */
    data = data >> 1;         // e shift a dx di una posizione così al prossimo loop prendo il bit successivo
  }

  //  Serial.println("** Loop completo **");
  //  Serial.println("** Ora Strobe WE **");
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}


// ********************************************
// ********* PRINT EEPROM CONTENTS ************
// ********************************************
void printContents() {
  for (int base = 0; base <= 255; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
}


#define HLT 0b1000000000000000
#define MI  0b0100000000000000
#define RI  0b0010000000000000
#define RO  0b0001000000000000
#define IO  0b0000100000000000
#define II  0b0000010000000000
#define AO  0b0000001000000000
#define AI  0b0000000100000000
#define EO  0b0000000010000000
#define FI  0b0000000001000000
#define BI  0b0000000000100000
#define SU  0b0000000000010000
#define OI  0b0000000000001000
#define CE  0b0000000000000100
#define CO  0b0000000000000010
#define J   0b0000000000000001


uint16_t data[] = {
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 0000 - NOP
  CO|MI, RO|II|CE,  IO|MI,  RO|AI,  0,            0,  0,  0, // 0001 - LDA
  CO|MI, RO|II|CE,  IO|MI,  RO|BI,  EO|AI|FI,     0,  0,  0, // 0010 - ADD
  CO|MI, RO|II|CE,  IO|MI,  RO|BI,  EO|AI|SU|FI,  0,  0,  0, // 0011 - SUB
  CO|MI, RO|II|CE,  IO|J,   0,      0,            0,  0,  0, // 0100 - JMP
  CO|MI, RO|II|CE,  IO|MI,  AO|RI,  0,            0,  0,  0, // 0101 - STA
  CO|MI, RO|II|CE,  IO,AI,  0,      0,            0,  0,  0, // 0110 - LDI
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 0111
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1000
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1001
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1010
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1011
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1100
  CO|MI, RO|II|CE,  0,      0,      0,            0,  0,  0, // 1101
  CO|MI, RO|II|CE,  AO|OI,  0,      0,            0,  0,  0, // 1110 - OUT
  CO|MI, RO|II|CE,  HLT,    0,      0,            0,  0,  0, // 1111 - HLT
};


// ********************************************
// *********** CODICE PRINCIPALE **************
// ********************************************
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(300);
  pinMode(SHIFT_DATA, OUTPUT); // da qui in poi setto la direzione "da Arduino verso HC595"
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, /*LOW*/ HIGH); // WE = HI e così facendo Arduino mette automaticamente un pull-up
  pinMode(WRITE_EN, OUTPUT); // così quando attivo il pin, questo è già attivo HI


  // ********************************************
  // ************* EEPROM ERASE *****************
  // ********************************************
  Serial.print("\n+++++++++++++++++++++++++++++\n+++++++++++++++++++++++++++++\nErasing EEPROM ");
  for (int address = 0; address <= 2048; address += 1) {
    writeEEPROM(address, 0x00 /*0xAA*/);

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" Done!");


  // ROM program
  Serial.print("\nProgramming EEPROM ");
  for (int address = 0; address < sizeof(data) / sizeof(data[0]); address += 1) {
    writeEEPROM(address, data[address] >> 8); // faccio shift a destra così prendo i primi 8 bit della word a 16 bit
    writeEEPROM(address +128, data[address]); // scrivo gli 8 bit LSb all'indirizzo 128 in poi così faccio le due EEPROM uguali e poi cambio l'indirizzo
    // del bit A7 a HI sulla breadboard in modo da partire dal byte 128 per la seconda EEPROM
    if (address % 8 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" Done!");

  printContents();
}

void loop() {
  // put your main code here, to run repeatedly:

}
