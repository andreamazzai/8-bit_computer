    LDY #$00
    LDY #$80
    DEY     
    LDA #$01
    TYA     
    INA     
    CPY #$50
    CPY #$7F
    CPY #$80
    SEC     
    CLC     
    LDA #$10
    ADC #$10
    ADC #$60
    ADC #$80
    LDX #$12
    LAB1:
    TXA     
    DEX     
    CPX #$10
    BNE LAB1 

    LDX #$FF
    TXS
    LDA #$70 
    CLC      
    CLV      
    SEC      
    SBC #$10 
    SBC #$60 
    PHP      
    CLC      
    LDA #$70 
    PLP      

    LDA #$70
    SBC #$78
    LDA #$A0
    SBC #$40

    SEC     
    LDA #$A0
    SBC #$40
    LDX #$20
    LAB2:
    DEX     
    TXA     
    CPX #$10
    BNE LAB2 
    LDX #$30
    LAB3:
    DEX     
    TXA     
    CPX #$28
    BNE LAB3 
    CLC     
    CLV     

    SEC       
    LDA #$7F  
    ASL A     
    ASL A     
    LDA #$FF  
    ASL A     
    LSR A     
    LDA #$FF  
    STA $C0   
    ASL $C0   
    LDA $C0   
    LDA #$7E  
    STA $C1   
    ASL $C1   
    LDA $C1   
    LDA #$7E  
    STA $C1   
    LDX #$11
    ASL $B0,X 
    ASL $B0,X 
    ASL $B0,X 
    ASL $B0,X 
    ASL $B0,X 
    LDA $C1   

    CLC     
    LDA #$FF
    ROL A   
    CLC     
    LDA #$7E
    ROL A   
    ROR A   
    HLT

LDA #$80
TAY
DEY
CPY #$50
CPY #$7F
CPY #$80
SEC
CLC
LDA #$10
ADC #$10
ADC #$60
ADC #$80
LDX #$12
LAB1:
TXA
DEX
CPX #$10
BNE LAB1
LDA #$70
CLC
CLV
SEC
SBC #$10
SBC #$60
SEC
LDA #$70
SBC #$78
LDA #$A0
SBC #$40

LDA #$FF
ROL A
CLC
LDA #$7E
ROL A
ROR A