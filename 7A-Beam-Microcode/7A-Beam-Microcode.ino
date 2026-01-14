// per la connessione si usa ttyUSB0

// Definizione dei pin immaginando di avere l'USB di Arduino a sinistra
#define SHIFT_DATA  2 // riferito a nomenclatura GPIO, dunque pin D2 = 5
#define SHIFT_CLK   3 // riferito a nomenclatura GPIO, dunque pin D3 = 6
#define SHIFT_LATCH 4 // riferito a nomenclatura GPIO, dunque pin D4 = 7
#define EEPROM_D0 5   // D0 per la EEPROM, D5 del Nano
#define EEPROM_D7 12  // D7 per la EEPROM, D12 del Nano
#define WE        14  // /WE per la EEPROM, D13 del Nano
#define OE        15  // /OE per la EEPROM, D14 del Nano

// void initMicroCodeBlock(int block);
void setAddress(int address, bool outputEnable);
void eeprom_erase(byte value);
void eeprom_program();
void WriteOpcode(uint8_t opcode);
void writeEEPROM(int address, byte data);
byte readEEPROM(int address);
void printContents(int start, int lenght);
uint16_t calcCRC16_pre();
uint16_t calcCRC16_post();
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
  pinMode(SHIFT_DATA, OUTPUT); // da qui in poi setto la direzione "da Arduino verso HC595"
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WE, HIGH);               // WE = HI e così facendo Arduino mette automaticamente un pull-up
  digitalWrite(OE, HIGH);
  pinMode(WE, OUTPUT);                  // così, quando attivo il pin, questo è già attivo HI
  pinMode(OE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);         // per poter lampeggiare alla fine della programmazione
  Serial.println("\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("+++++          Microcode EEPROM programmer           +++++");
  Serial.println("+++++            for BEAM 8-bit Computer             +++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  // eeprom_erase(0x77);
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.print("EEPROM CRC-PRE:  $");
  crc_pre = calcCRC16_pre();
  Serial.println(crc_pre, HEX);
  // eeprom_program();
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.print("EEPROM CRC-POST: $");
  crc_post = calcCRC16_post();
  Serial.println(crc_post, HEX);
  // printContents(0x0000, 128);
  // printContents(0x1000, 128);
  // printContents(0x2000, 128);
  // printContents(0x3000, 128);
  printContents(0x2AAA, 1);
  printContents(0x5555, 1);
  Serial.println("Done!");
  // printArrayContents();
  uint32_t elapsedTime;
  elapsedTime = millis() / 1000;
  Serial.print("Elapsed time: ");
  Serial.print(elapsedTime);
  Serial.println(" seconds.");
}

// ************************************************************
// ****************** DEFINIZIONE PIN ROM *********************
// ************************************************************
//              ROM0    ROM1    ROM2    ROM3
//            +------++------++------++------+
// ROM 0      WWWWRRRR                                               WA  WB  WS  WR  WM  WD  WX  WY WPC  WO    RA  RB  RS  RH  RR  RL  RD  RX  RY RPC  RF
#define W3  0b10000000000000000000000000000000L // Write bit 3        0   1   0   0   1   0   1   0   1   0
#define W2  0b01000000000000000000000000000000L // Write bit 2        0   0   0   1   1   0   0   1   0   1
#define W1  0b00100000000000000000000000000000L // Write bit 1        0   0   1   1   1   0   0   0   1   1
#define W0  0b00010000000000000000000000000000L // Write bit 0        1   1   1   1   1   0   0   0   0   0    
#define R3  0b00001000000000000000000000000000L // Read bit 3                                                   0   0   0   0   0   0   1   1   1   1   1
#define R2  0b00000100000000000000000000000000L // Read bit 2                                                   0   0   1   1   1   1   0   0   0   1   1
#define R1  0b00000010000000000000000000000000L // Read bit 1                                                   0   1   0   0   1   1   0   0   1   0   1
#define R0  0b00000001000000000000000000000000L // Read bit 0                                                   1   0   0   1   0   1   0   1   0   1   1
// ROM 1              76543210
#define U1  0b00000000100000000000000000000000L // Unused
#define FS  0b00000000010000000000000000000000L // Sets flags
#define FV  0b00000000001000000000000000000000L // Flag Overflow
#define FZ  0b00000000000100000000000000000000L // Flag Zero
#define FN  0b00000000000010000000000000000000L // Flag Negative
#define FC  0b00000000000001000000000000000000L // Flag Carry
#define HR  0b00000000000000100000000000000000L // Registro H-Right
#define HL  0b00000000000000010000000000000000L // Registro H-Left
// ROM 2                      76543210
#define C1  0b00000000000000001000000000000000L
#define C0  0b00000000000000000100000000000000L
#define U2  0b00000000000000000010000000000000L // Unused
#define SUD 0b00000000000000000001000000000000L
#define SE  0b00000000000000000000100000000000L
#define U3  0b00000000000000000000010000000000L
#define CS  0b00000000000000000000001000000000L // /CC * /CS ==>> Passo ad ALU-Cin il flag C, invertendolo (la ALU ragiona con segnale Cin invertito)
#define CC  0b00000000000000000000000100000000L //  CC       ==>> Passo ad ALU-Cin il valore 1, cioé NO Carry; /CC * CS ==>> Passo ad ALU-Cin il valore 0, cioé YES Carry
// ROM 3                              76543210
#define WIR 0b00000000000000000000000010000000L // Write Instruction Register
#define LF  0b00000000000000000000000001000000L // ALU Force
#define N   0b00000000000000000000000000100000L // Fine istruzione
#define HLT 0b00000000000000000000000000010000L // Halt
#define DXY 0b00000000000000000000000000001000L // Se 1 espone X, se 0 espone Y
#define DZ  0b00000000000000000000000000000100L // Se HI allora X e Y = 0
#define JE  0b00000000000000000000000000000010L // Jump Enable per Relative Branch
#define PCI 0b00000000000000000000000000000001L // Program Counter Increment


// ************************************************************
// ************** DEFINIZIONE DEI SEGNALI ROM 0 ***************
// ************************************************************
#define WA  00|00|00|W0 // 0001
#define WB  00|00|W1|00 // 0002
#define WS  00|W2|00|00 // 0004
#define WR  00|W2|W1|00 // 0006
#define WM  00|W2|W1|W0 // 0007
#define WD  W3|00|00|00 // 0008
#define WX  W3|00|00|W0 // 0009
#define WY  W3|00|W1|00 // 000A
#define WPC W3|W2|00|W0 // 000D
#define WO  W3|W2|W1|00 // 000E

#define RA  00|00|00|R0 // 0001
#define RB  00|00|R1|00 // 0002
#define RS  00|R2|00|00 // 0004
#define RH  00|R2|00|R0 // 0005
#define RR  00|R2|R1|00 // 0006
#define RL  00|R2|R1|R0 // 0007
#define RD  R3|00|00|00 // 0008
#define RX  R3|00|00|R0 // 0009
#define RY  R3|00|R1|00 // 000A
#define RPC R3|R2|00|R0 // 000D
#define RF  R3|R2|R1|R0 // 000F

// ************************************************************
// ****************** DEFINIZIONE SHORTCUT ********************
// ************************************************************
#define F1  RPC|WM
#define F2  RR|WIR|PCI
#define WH  HR|HL
#define WAH WA|WH
#define RDX RD|DXY
#define RDY RD
#define RDZ RD|DZ
#define FNZ FS|FN|FZ
#define FNZC FS|FN|FZ|FC
#define NVZC FN|FV|FZ|FC
#define FNVZC FS|NVZC

const uint8_t NUM_STEPS = 16;
const uint8_t NUM_TEMPLATE_STEPS = 8;

typedef uint32_t microcode_t;
typedef microcode_t template_t[256][NUM_TEMPLATE_STEPS];
microcode_t code[NUM_STEPS]; // contiene un opcode composto da 4 byte (uno per ognuna delle 4 ROM) e 16 step = 64 byte

const template_t template0 PROGMEM = {
//<2>          <3>           <4>           <5>           <6>           <7>           <8>           <9>
// Sezione 0 -- Accumulatore, Implicito, Relativo, Immediato -- A, IP, REL, IM
{ HLT,          N,            0,            0,            0,            0,            0,            0 }, // 00 IP_HLT
{ RPC|WM,       RR|WM,        RR|WPC|N,     0,            0,            0,            0,            0 }, // 01 IN_JMP
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 02
{ CC|FC|RL|N,   0,            0,            0,            0,            0,            0,            0 }, // 03 IP_SEC *  -- CC mette HI in ALU-Cin dunque operazione è LLHH = -1 = signed 11111111
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 04
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 05
{ RPC|WM,       RR|WB,        RX|WH,    CS|C0|FNZC|LF|RL, PCI|N,        0,            0,            0 }, // 06 IM_CPX * - 
{ RPC|WM,       RR|WB,        RY|WH,    CS|C0|FNZC|LF|RL, PCI|N,        0,            0,            0 }, // 07 IM_CPY *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 08 IM_PLA - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 09 IM_PHA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 0a
{ FNZ|RA|WX|N,  0,            0,            0,            0,            0,            0,            0 }, // 0b IP_TAX - NZ
{ FNZ|RA|WY|N,  0,            0,            0,            0,            0,            0,            0 }, // 0c IP_TAY - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 0d IP_TSX - NZ ******* TO DO
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 0e
{ N,            0,            0,            0,            0,            0,            0,            0 }, // 0f IP_NOP
{ RA|WO|N,      0,            0,            0,            0,            0,            0,            0 }, // 10 IP_OUT
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 11 IP_RTS
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // 12 RE_BCC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 13 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 14
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 15
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 16
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 17 
{ NVZC,         0,            0,            0,            0,            0,            0,            0 }, // 18 IP_PLP - NVZC dal bus
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 19 IP_PHP
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1a 
{ FNZ|RX|WAH|N, 0,            0,            0,            0,            0,            0,            0 }, // 1b IP_TXA - NZ
{ FNZ|RY|WAH|N, 0,            0,            0,            0,            0,            0,            0 }, // 1c IP_TYA - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1d IP_TXS - NZ ******* TO DO
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1f

// Sezione 1 -- Immediato IM
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 20
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 21
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 22
{ CS|FC|RL|N,   0,            0,            0,            0,            0,            0,            0 }, // 23 IP_CLC * - CS mette LO in ALU-Cin dunque operazione è LLHH = 0 = signed 00000000
{ RPC|WM,       RR|FNZ|WX|PCI|N,    0,      0,            0,            0,            0,            0 }, // 24 IM_LDX - NZ
{ RPC|WM,       RR|FNZ|WY|PCI|N,    0,      0,            0,            0,            0,            0 }, // 25 IM_LDY - NZ
{ RPC|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0,            0,            0 }, // 26 IM_SBC * - NVZC
{ RPC|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,     0,  0,            0,            0,            0 }, // 27 IM_CMP * - NZC
{ RPC|WM,       RR|FNZ|WAH|PCI|N,   0,      0,            0,            0,            0,            0 }, // 28 IM_LDA - NZ
{ RPC|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0,            0,            0 }, // 29 IM_ADC * - NVZC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2a
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2d
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2f
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 30
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 31
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // 32 RE_BCS
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 33 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 34
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 35
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0,            0 }, // 36 IM_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 37 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 38
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 39
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3a 
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0,            0 }, // 3b IM_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3d
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0,            0 }, // 3e IM_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3f

// Sezione 2 - Assoluto -- AB
{ RPC|WM,       RR|WH,        CS|FNZ|RL|WR, RA|WH|PCI|N,  0,            0,            0,            0 }, // 40 AB_INC * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 41 AB_JSR
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 42
{ CS|FS|FV|RL|N,    0,        0,            0,            0,            0,            0,            0 }, // 43 IP_CLV *
{ RPC|WM,       RR|WM,        RR|FNZ|WX|PCI|N,    0,      0,            0,            0,            0 }, // 44 AB_LDX - NZ
{ RPC|WM,       RR|WM,        RR|FNZ|WY|PCI|N,    0,      0,            0,            0,            0 }, // 45 AB_LDY - NZ
{ RPC|WM,       RR|WM,        RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0,            0 }, // 46 AB_SBC * - NVZC // devo settare Cin = HI cioè 0 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WM,        RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,     0,  0,            0,            0 }, // 47 AB_CMP * - NZC
{ RPC|WM,       RR|WM,        RR|FNZ|WAH|PCI|N,   0,      0,            0,            0,            0 }, // 48 AB_LDA
{ RPC|WM,       RR|WM,        RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0,            0 }, // 49 AB_ADC *  // devo settare Cin = LO cioè 1 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WM,        RA|WR|PCI|N,  0,            0,            0,            0,            0 }, // 4a AB_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4c AB_ASL *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4d AB_LSR
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4e
{ RPC|WM,       RR|WH,        CC|FNZ|RL|WR, RA|WH|PCI|N,  0,            0,            0,            0 }, // 4f AB_DEC *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 50
{ RPC|WM,       RR|WPC|N,     0,            0,            0,            0,            0,            0 }, // 51 AB_JMP
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // 52 RE_BEQ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 53 
{ RPC|WM,       RR|WM,        RX|WR|PCI|N,  0,            0,            0,            0,            0 }, // 54 AB_STX
{ RPC|WM,       RR|WM,        RY|WR|PCI|N,  0,            0,            0,            0,            0 }, // 55 AB_STY
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0 }, // 56 AB_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 57 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 58
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 59
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5a 
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0 }, // 5b AB_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5c AB_ROL
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5d AB_ROR
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0,            0 }, // 5e AB_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5f

// Sezione 3 - Indexed Addressing: Absolute,X
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,        CS|FNZ|RL|WR, RA|WH|PCI|N,  0,            0 }, // 60 AX_INC * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 61
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 62
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 63
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 64
{ RPC|WM,       RR|WD,        RDX|WM,       RR|FNZ|WY|PCI|N,    0,      0,            0,            0 }, // 65 AX_LDY - NZ
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0 }, // 66 AX_SBC * - NVZC // devo settare Cin = HI cioè 0 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,     0,  0,            0 }, // 67 AX_CMP * - NZC
{ RPC|WM,       RR|WD,        RDX|WM,       FNZ|RR|WAH|PCI|N,   0,      0,            0,            0 }, // 68 AX_LDA
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0 }, // 69 AX_ADC * - NZVC // devo settare Cin = LO cioè 1 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WD,        RDX|WM,       RA|WR|PCI|N,  0,            0,            0,            0 }, // 6a AX_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6c AX_ASL *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6d AX_LSR
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6e
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,        CC|FNZ|RL|WR, RA|WH|PCI|N,  0,            0 }, // 6f AX_DEC *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 70
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 71
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // 72 RE_BNE    // con i branch devo calcolare dove saltare
// salvo X in B, leggo PC e metto in MAR, leggo RAM e metto in D,
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 73 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 74
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 75
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 76 AX_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 77 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 78
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 79
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7a 
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 7b AX_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7c AX_ROL
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7d AX_ROR
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 7e AX_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7f

// Sezione 4 - Indexed Addressing: Absolute,Y
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 80
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 81
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 82
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 83
{ RPC|WM,       RR|WD,        RDY|WM,       RR|FNZ|WX|PCI|N,    0,      0,            0,            0 }, // 84 AY_LDX
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 85
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0 }, // 86 AY_SBC * - NVZC
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,     0,  0,            0 }, // 87 AY_CMP * - NZC
{ RPC|WM,       RR|WD,        RDY|WM,       FNZ|RR|WAH|PCI|N,   0,      0,            0,            0 }, // 88 AY_LDA - NZ
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N,   0,  0,            0 }, // 89 AY_ADC * - NVZC
{ RPC|WM,       RR|WD,        RDY|WM,       RA|WR|PCI|N,  0,            0,            0,            0 }, // 8a AY_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8d
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8f
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 90
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 91
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDY|WB,       RH|WX,        RA|WH,    RB|JE }, // 92 RE_BMI
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 93 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 94
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 95
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 96 AY_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 97 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 98
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 99
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9a 
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 9b AY_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9d
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,   0,      0,            0 }, // 9e AY_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9f

// Sezione 5 - Pre-Indexed Indirect, "(Zero-Page,X)"
{ RX|WH,        CS|FNZ|RL|WX, RA|WH|N,      0,            0,            0,            0,            0 }, // a0 IP_INX * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a5
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N, 0,    0 }, // a6 IX_SBC * - NVZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,   0,    0 }, // a7 IX_CMP * - NZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        FNZ|RR|WAH|PCI|N,   0,      0,            0 }, // a8 IX_LDA - NZ
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N, 0,    0 }, // a9 IX_ADC * - NVZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RA|WR|PCI|N,  0,            0,            0 }, // aa IX_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ab
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ac
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ad
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ae
{ RX|WH,        CC|FNZ|RL|WX, RA|WH|N,      0,            0,            0,            0,            0 }, // af IP_DEX * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b1
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // b2 RE_BPL
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b3 
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RY|WR|PCI|N,  0,            0,            0 }, // b4 IX_STY
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b5
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    FNZ|RL|WAH|PCI|N, 0,            0 }, // b6 IX_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b7 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ba 
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    FNZ|RL|WAH|PCI|N, 0,            0 }, // bb IX_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bd
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    FNZ|RL|WAH|PCI|N, 0,            0 }, // be IX_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bf

// Sezione 6 - Post-Indexed Indirect, "(Zero-Page),Y"
{ RY|WH,        CS|FNZ|RL|WY, RA|WH|N,      0,            0,            0,            0,            0 }, // c0 IP_INY * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c5
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|N, 0,    0 }, // c6 IY_SBC * - NVZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|N,   0,    0 }, // c7 IY_CMP * - NZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       FNZ|RR|WAH|PCI|N,   0,      0,            0 }, // c8 IY_LDA - NZ
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|N, 0,    0 }, // c9 IY_ADC * - NVZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RA|WR|PCI|N,  0,            0,            0 }, // ca IY_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cb
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cd
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ce
{ RY|WH,        CC|FNZ|RL|WY, RA|WH|N,      0,            0,            0,            0,            0 }, // cf IP_DEY * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d1
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // d2 RE_BVC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d3 
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RX|WR|PCI|N,  0,            0,            0 }, // d4 IY_STX
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d5
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,           0 }, // d6 IY_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d7 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // da 
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,           0 }, // db IY_AND * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // dc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // dd
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|N,           0 }, // de IY_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // df

// Sezione 7
{ CS|FNZ|RL|WAH|N,    0,      0,            0,            0,            0,            0,            0 }, // e0 AA_INA *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e5
{ RPC|WM,       RR|WM,        RR|WB,        RX|WH,        CS|C0|FNZC|RL,  RA|WH|PCI|N,    0,        0 }, // e6 AB_CPX * - NZC
{ RPC|WM,       RR|WM,        RR|WB,        RY|WH,        CS|C0|FNZC|RL,  RA|WH|PCI|N,    0,        0 }, // e7 AB_CPY * - NZC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ea
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // eb
{ C1|FS|FC|RH,      HL|CC,    FNZ|RH|WA|N,  0,            0,            0,            0,            0 }, // ec AA_ASL  C <= xxxxxxxx <= 0 - non uso ALU
{ C0|C1|FS|FC|RH,   HR|CC,    FNZ|RH|WA|N,  0,            0,            0,            0,            0 }, // ed AA_LSR  0 => xxxxxxxx => C - non uso ALU
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ee
{ CC|FNZ|RL|WAH|N,  0,        0,            0,            0,            0,            0,            0 }, // ef AA_DEA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f1
{ RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // f2 RE_BVS
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f3 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f5
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f6
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f7 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // fa 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // fb AB_BIT *
{ C1|HL,        FNZC|RH|WA|N, 0,            0,            0,            0,            0,            0 }, // fc AA_ROL
{ C0|C1|HR,     FNZC|RH|WA|N, 0,            0,            0,            0,            0,            0 }, // fd AA_ROR
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // fe 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ff
};

// ************************************************************
// *********************                  *********************
// ********************   EEPROM PROGRAM   ********************
// *********************                  *********************
// ************************************************************
void eeprom_program()
{
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.println("Programming EEPROM");
  for (uint16_t opcode = 0; opcode < 256; opcode++)
  {
    buildInstruction(opcode);
    // printOpcodeContents(opcode);
    char buf[80];
    if ((opcode) % 16 == 0)
    {
       Serial.print("Opcode: 0x");
    }
    sprintf(buf, "%02X ", opcode);
    Serial.print(buf);
    if ((opcode + 1) % 16 == 0)
    {
      Serial.println("");
    }
    WriteOpcode(opcode);
  }
  Serial.println("Done!");
}

// ************************************************************
// *********************                 **********************
// ********************   CALCOLO CRC16   *********************
// *********************                 **********************
// ************************************************************

// CALCOLO CRC16 PRE-PROGRAMMAZIONE
/* Il calcolo del CRC prevede la ricezione dei dati in sequenza (da 0x0000 a 0x3FFF). Non posso calcolare il CRC
durante la programmazione della EEPROM, perché in quel momento genero un opcode completo e ne scrivo i 4 segmenti
da 16 byte "frazionandoli" sulla EEPROM corrispondente (1a, 2a, 3a, 4a).
Faccio dunque un ciclo: per ogni ROM genero le 256 istruzioni in sequenza; ricavo solo i 16 byte che mi interessano
in quel momento (1a EEPROM, 2a EEPROM etc) e li utilizzo per calcolare il checksum. */
uint16_t calcCRC16_pre(void)
{
  crc = 0xFFFF;
  uint16_t polynomial = 0x1021;
  for (uint8_t rom = 0; rom < 4; rom++)
  {
    for (uint16_t opcode = 0; opcode < 256; opcode++)
    {
      buildInstruction(opcode);
      for (uint8_t step = 0; step < NUM_STEPS; step++)
      {
        // printInstruction(rom, opcode, step); // solo per Debug
        crc = calculate_crc(((code[step]) >> (24 - 8 * rom)) & 0xFF, crc, polynomial);
      }
    }
  }
  return crc;
}

// CALCOLO CRC16 POST-PROGRAMMAZIONE
/* Nella lettura di una EEPROM precedentemente programmata leggo tutti i byte in sequenza, dunque è sufficiente
leggere il contenuto da 0x0000 a 0x3FFF per calcolare il checksum. */
uint16_t calcCRC16_post(void)
{
  crc = 0xFFFF;
  uint16_t polynomial = 0x1021;
  byte data;
  for (int address = 0x0000; address <= 0x3FFF; address++)
  {
    data = readEEPROM(address);
    crc = calculate_crc(data, crc, polynomial);
  }
  return crc;
}

// CALCOLO MATEMATICO CRC16
uint16_t calculate_crc(uint8_t data, uint16_t crc, uint16_t polynomial)
{
  crc ^= data;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (crc & 0x0001)
    {
      crc >>= 1;
      crc ^= polynomial;
    }
    else
    {
      crc >>= 1;
    }
  }
  return crc;
}



//
// ************************************************************
// ************** COPIA BLOCCO MICROCODE IN RAM ***************
// ************************************************************
void buildInstruction(uint8_t opcode)
{
  // Ogni istruzione = 16 step: i primi 2 uguali per tutti; 8 presi dal template; 6 che - per ora - non utilizzo, dunque a 0.
  code[0] = F1; // Fetch instruction from memory (RPC, WM - Read Program Counter, Write Memory Address Register)
  code[1] = F2; // Opcode into IR (sets ALU mode and S bits) (RR, WIR, PCI - Read RAM, Write Instruction Register, Program Counter Increment)
  // Copia parte del template dalla memoria Flash alla memoria RAM
  // Gli step presenti nel template sono 8 e ognuno di essi sono 4 byte, uno per ogni ROM
  // Ogni step è composto da 4 byte che vanno allo stesso indirizzo su ognuna delle EEPROM; è una vista delle 4 ROM "affiancate"
  // In ogni ROM 0x000-0x00F è 1° opcode, 0x010-0x01f è 2° opcode... 0xFF0-FFF è 256° opcode
  memcpy_P(code + 2, template0[opcode], NUM_TEMPLATE_STEPS * 4);
  code[10] = code[11] = code[12] = code[13] = code[14] = code[15] = 0;
}

// ************************************************************
// **************** PREPARA E SCRIVE OPCODE *******************
// ************************************************************
void WriteOpcode(uint8_t opcode)
{
  // Sono 4 ROM, mappate $0-$0FFF, $1000-$1FFF, $2000-$2FFF, $3000-$3FFF
  // Ogni opcode è lungo 16 step = 16 byte per ogni ROM, dunque 256 * 16 = 4096 byte = $1000
  for (uint8_t rom = 0; rom < 4; rom++)
  {
    // Serial.print(" - ROM: ");
    // Serial.print(rom);
    // Serial.print(" - ");
    for (uint8_t step = 0; step < NUM_STEPS; step++) // ciclo fra i 16 step di ogni opcode e dunque li scrivo consecutivamente su ogni EEPROM
    {
      uint16_t address;
      address = 0x1000 * rom;
      address += opcode * NUM_STEPS;
      address += step;
      // printStep(step, address, rom);
      writeEEPROM(address, ((code[step]) >> (24 - 8 * rom)) & 0xFF);
      // if (step % 16 == 0)
      // {
      //   Serial.print(".");
      // }
    }
  }
  // if ((opcode - 1) % 2 == 0)
  // {
  //   Serial.println("");
  // }
  // else
  // {
  //   Serial.print(" - ");
  // }
}

// ************************************************************
// ******************** WRITE TO EEPROM ***********************
// ************************************************************
void writeEEPROM(int address, byte data)
{
  // Setto indirizzo e disabilito output EEPROM; solo poi poi abilito output di Arduino, così non causo cortocircuiti.
  // La EEPROM non ha resistenze in uscita; se EEPROM ha output attivo e Arduino è in input, si genera un cortocircuito.
  setAddress(address, /*outputEnable */ false);
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
  digitalWrite(WE, LOW);
  delayMicroseconds(1);
  digitalWrite(WE, HIGH);
  delay(10);
}

// ************************************************************
// *********************** SET ADDRESS ************************
// ************************************************************
// prima metto il dato in D2 (SHIFT_DATA) e poi pulso D3 (SHIFT_CLK) per mandarlo
void setAddress(int address, bool outputEnable)
{
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8)); // se false = 0 = /OE abilitato
  // Qui vedrò sullo scope il SER e il SRCLK
  // L'indirizzo arriva in due byte. Quanto sopra gestisce la parte alta dell'indirizzo (che
  // sono i bit da A8 a A10, e poi aggiunge il bit 15, che controlla /OE
  // se il pin /OE = 0, attivo l'output, dunque leggo dalla EEPROM
  // se il pin /OE = 1, disattivo l'output, dunque scrivo sulla EEPROM
  // poiché uso "(outputEnable ? 0x00L: 0x80)", sto dicendo che se nella routine passo un "outputEnable" true,
  // setto a zero /OE e dunque attivo la lettura; se passo un "outputEnable" false, il MSb è HI e dunque attivo la scrittura
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  // Quanto sopra gestisce la parte bassa dell'indirizzo (da A0 ad A7)
  // Quanto sotto sblocca il 595
  // Qui vedrò sullo scope il RCLK
  digitalWrite(OE, outputEnable ? LOW : HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
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
  Serial.println("Done!");
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
  setAddress(address, /*outputEnable*/ true);
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
  // *********************** PRINT STEP *************************
  // ************************************************************
  void printStep(uint8_t step, uint16_t address, uint8_t rom)
  {
    Serial.print("Step: ");
    Serial.print(step, HEX);
    Serial.print(" - Address: 0x");
    Serial.print(address, HEX);
    Serial.print(" - Value: ");
    Serial.println(((code[step]) >> (24 - 8 * rom)) & 0xFF, HEX);
    }

// ************************************************************
// ***************** PRINT EEPROM CONTENTS ********************
// ************************************************************
void printContents(int start, int lenght)
{
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.print("EEPROM Content $");
  Serial.print(start, HEX);
  Serial.print(" - $");
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

  // ************************************************************
  // ********************** PRINT OPCODES ***********************
  // ************************************************************
  void printOpcodeContents(uint8_t opcode)
  {
    // stampa il contenuto dell'array temporaneo creato in RAM per l'opcode
    // sono 4 ROM e 16 step
    Serial.println("_________");
    for (int step = 0; step < NUM_STEPS; step += 4)
    {
      // Serial.print(code[step], HEX);
      // Serial.print(":");
      // for (int shift = 24; shift >= 0; shift -= 8)
      // {
      Serial.print("Opcode 0x");
      Serial.print(opcode, HEX);
      Serial.print(" - Step: ");
      Serial.print(step, HEX);
      Serial.print("/");
      Serial.print(step + 3, HEX);
      Serial.print(":  ");
      Serial.print(((code[step]) >> 24) & 0xFF, HEX); // ROM 0
      Serial.print(":");
      Serial.print(((code[step]) >> 16) & 0xFF, HEX); // ROM 1
      Serial.print(":");
      Serial.print(((code[step]) >> 8) & 0xFF, HEX); // ROM 2
      Serial.print(":");
      Serial.print(((code[step]) >> 0) & 0xFF, HEX); // ROM 3
      Serial.print(" : ");
      //
      Serial.print(((code[step + 1]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 0) & 0xFF, HEX);
      Serial.print(" : ");
      //
      Serial.print(((code[step + 2]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 0) & 0xFF, HEX);
      //
      Serial.print(" : ");
      Serial.print(((code[step + 3]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 0) & 0xFF, HEX);
      Serial.println("");
      // uint32_t data[4];
      // {
      //   data[col + 0] = (code[col + 0]);
      //   data[col + 1] = (code[col + 1]);
      //   data[col + 2] = (code[col + 2]);
      //   data[col + 3] = (code[col + 3]);
      // }
      // char buf[80];
      // sprintf(buf, "Opcode %02x:  %04lx %04lx %04lx %04lx",
      //         opcode, data[0], data[1], data[2], data[3]);
      // Serial.println(buf);
      // }
    }
  }



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

  // ********************************************
  // ********** TEST LETTURA DA ARRAY ***********
  // ********************************************
  // Questo non funziona, restituisce spazzatura
  /*     uint32_t val[16];
      for (int row = 0; row < 4; row += 1) {
        for (int col = 0; col < 16; col += 4) {
          val[col] = my_ram_microcode_template[row][col];
        }
        char buf[80];
        sprintf(buf, "%03x:  %02lx %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                row, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
                val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
        Serial.println(buf);
      } */

  /* void read_RAM()
  {
    // Qui voglio provare a leggere il contenuto della RAM copiata e stamparlo in sequenza, per vedere se quanto
    // ho copiato dalla Flash con initMicroCodeBlock sia o meno corretto
    uint32_t data;
    // per 256 elementi ognuno lungo 4 byte
    //
    for (int row = 0 + 16 * block; row < 4 + 16 * block; row += 1)
    {
      Serial.print(row, HEX);
      Serial.print(":  ");
      for (int col = 0; col < 16; col += 4)
      {
        data = my_ram_microcode_template[row][col];
        Serial.print(data, HEX);
        Serial.print(" / ");
        byte value1, value2, value3, value4;
        char buf[60];
        sprintf(buf, "%02lx:%02lx:%02lx:%02lx", (data & 0xFF000000) >> 24, (data & 0x00FF0000) >> 16, (data & 0x0000FF00) >> 8, (data & 0x000000FF));
        Serial.print(buf);
        value1 = data >> 24;
        value2 = (data & 0x00FF0000) >> 16;
        value3 = (data & 0x0000FF00) >> 8;
        value4 = (data & 0x000000FF);
        sprintf(buf, " - %02x:%02x:%02x:%02x", value1, value2, value3, value4);
        Serial.print(buf);
        Serial.print(" - Totale = ");
        Serial.println(value1 + value2 + value3 + value4);
        // sprintf(buf, "%03x:  %02x %02x %02x %02x", row, data >> 24, data & 0x00FF000 > 16, data & 0x0000FF000 > 8, data & 0x00000FF);
        // Serial.print("    ");
        //           for (int cnt = 0; cnt < 16; cnt += 1) {
        //           data = current_microcode_block[i][j];
        //           Serial.print(data >> 24, HEX);
        //           Serial.print(F(":"));
        //         }
      }
      Serial.println("");
    }
  } */

  /*
    // ********************************************
    // ************ EEPROM PROGRAM ****************
    // ********************************************
    void eeprom_program() {
    Serial.print("\nProgramming EEPROM ");
    // ogni block = 16 righe * 16 colonne * elemento uint32_t = 1024 Byte
    // ogni sezione sono 32 righe dunque due blocchi = 2K
    // 8 sezioni = 16 KB divisi in 4 parti, 0-FFF / 1000-1FFF / 2000-2FFF / 3000-3FFF
      for (int block = 0; block < 4; block += 1) {
        Serial.print("\nBlock = ");
        Serial.print(block);
        Serial.print(" - Starting element = ");
        Serial.print(block * 256);
        Serial.print(" - Ending element = ");
        Serial.print((block + 1) * 256);
        initMicroCodeBlock(block);
        for (int address = 256 * block; address < 256 * (block + 1); address += 1) {
          int byte_sel    = (address & 0b11000000000000) >> 12; // selezione della ROM
          int instruction = (address & 0b00111111110000) >> 4;
          int step        = (address & 0b00000000001111);
          if (address % 16 == 0) {
            Serial.print("\naddress / instruction = ");
            Serial.print(address, HEX);
            Serial.print(" / ");
            Serial.print(current_microcode_block[instruction][step] >> 24, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x00FF0000) >> 16, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x0000FF00) >>  8, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x000000FF),  HEX);
          }
          // switch (byte_sel) {
            // case 0:
              writeEEPROM(address, current_microcode_block[instruction][step] >> 24);
              // if (address % 64 == 0) {
              //   Serial.print(".");
              //   break;
              // }
            // case 1:
              writeEEPROM(address + 4096, current_microcode_block[instruction][step] >> 16);
              // if (address % 64 == 0) {
              //   Serial.print(":");
              //   break;
              // }
            // case 2:
              writeEEPROM(address + 4096 * 2, current_microcode_block[instruction][step] >> 8);
              //   if (address % 64 == 0) {
              //     Serial.print(",");
              //     break;
              // }
            // case 3:
              writeEEPROM(address + 4096 * 3, current_microcode_block[instruction][step]);
              // if (address % 64 == 0) {
              //   Serial.print(";");
              //   break;
              // }
          // if (address % 64 == 0) {
          //   Serial.print(".");
          // }
          // }
        }
      Serial.println("\nDone!");
      }
    }
   */

  void loop()
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
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