
#include <SPI.h>

/*  Const for Voltage
 *  
 *  5 Volt  = 0.454V => 800
 *  9 Volt  = 0.818V => 837
 *  12 Volt = 1V     => 1024
 *  
 *  Address for this SLAVE (Header)
 *  0x60 = write Volt 1, 0x62 = write Volt 2, 0x61 = read Amp 1, 0x63 = read Amp 2
*/

#define volt_5       474
#define volt_9       812
#define volt_12      1024

#define CS_Charger_1   10        //  Arduino UNO = 10, Atmega328 pin = 14
#define CS_Charger_2    9        //  Arduino UNO = 9, Atmega328 pin = 13
#define VOLT_1         A0        //  Atmega328 pin = 23
#define CAP_1          A1        //  Atmega328 pin = 24
#define VOLT_2         A2        //  Atmega328 pin = 25
#define CAP_2          A3        //  Atmega328 pin = 26

uint8_t VoltOut_1_reg = 0;
uint8_t VoltOut_2_reg = 0;           // 0=0 Volt, 1=5 Volt, 2=9 Volt, 3=12 Volt
uint16_t AmpOut_1_reg;
uint16_t AmpOut_2_reg;            // I = 1 A max
uint8_t Pot_1_reg;
uint8_t Pot_2_reg;
float Ampere;
uint8_t tempSerial[5];
uint8_t SerialCount = 0;

boolean Timer_bool;

//*********************************************
// Sample Rate Volt Out detector
ISR(TIMER2_OVF_vect){
  Timer_bool = true;
  TCNT2 = 0x9C;          // divider for 0.01S sample, 8mHz = 0xB1, 16mHz = 0x9C
}

//*********************************************

void setup() {

  pinMode( VOLT_1, INPUT);
  digitalWrite( VOLT_1, HIGH);
  pinMode( CAP_1, INPUT);
  digitalWrite( CAP_1, HIGH);
  pinMode( VOLT_2, INPUT);
  digitalWrite( VOLT_2, HIGH);
  pinMode( CAP_2, INPUT);
  digitalWrite( CAP_1, HIGH);

  pinMode( CS_Charger_1, OUTPUT);
  digitalWrite( CS_Charger_1, HIGH);
  pinMode( CS_Charger_2, OUTPUT);
  digitalWrite( CS_Charger_2, HIGH);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  Serial.begin(9600);
  // ** Timer init **
  TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
  TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);  // Prescale 1024 (Timer/Counter started)

  
}

//*******************************************************************************
void loop() {
uint16_t tempVoltCn;
uint16_t tempVoltIn;
  
  if (Timer_bool){
    analogReference(INTERNAL);
    Timer_bool = false;
    if (VoltOut_1_reg != 0){
      tempVoltCn = VoltStep(VoltOut_1_reg);
      tempVoltIn = analogRead(VOLT_1);
        if (tempVoltIn < tempVoltCn){
          if (Pot_1_reg < 0x7F){Pot_1_reg++;}
          Charger_Write(1, Pot_1_reg);
        }else if (tempVoltIn > tempVoltCn){
          if (Pot_1_reg > 0){Pot_1_reg--;}
          Charger_Write(1, Pot_1_reg);
        }
      
    }else{Charger_Write(1, 0);}
    if (VoltOut_2_reg != 0){
      tempVoltCn = VoltStep(VoltOut_2_reg);
      tempVoltIn = analogRead(VOLT_2);
        if (tempVoltIn < tempVoltCn){
          if (Pot_2_reg < 0x7F){Pot_2_reg++;}
          Charger_Write(2, Pot_2_reg);
        }else if (tempVoltIn > tempVoltCn){
          if (Pot_2_reg > 0){Pot_2_reg--;}
          Charger_Write(2, Pot_2_reg);
        }
    }else{Charger_Write(2, 0);}

    analogReference(DEFAULT);
    AmpOut_1_reg = analogRead( CAP_1);
    Ampere = AmpOut_1_reg * (5.0 / 1023.0);
    Ampere = (Ampere / 50) * 10000;
    AmpOut_1_reg = Ampere;
    
    AmpOut_2_reg = analogRead( CAP_2);
    Ampere = AmpOut_2_reg * (5.0 / 1023.0);
    Ampere = (Ampere / 50) * 10000;             // 50 is the Constant of OpAmp
    AmpOut_2_reg = Ampere;
    
  }

  if (Serial.available()>0){
    uint8_t temp = Serial.read();
    if (temp == '\n'){
      SerialProcess();
      SerialCount= 0;
    }else if (temp != '\r'){
      tempSerial[SerialCount] = temp;
      SerialCount++;
    }
  }
}

void SerialProcess(){
uint16_t temp;
  
  switch (tempSerial[0]){
    case 0x60:
      VoltOut_1_reg = tempSerial[1];
      break;
    case 0x62:
      VoltOut_2_reg = tempSerial[1];
      break;
    case 0x61:
      Serial.write(0x61);
      temp = (AmpOut_1_reg >> 8) & 0x00FF;
      Serial.write(temp);
      temp = AmpOut_1_reg & 0x00FF;
      Serial.write(temp);
      Serial.write('\r\n');
      break;
    case 0x63:
      Serial.write(0x63);
      temp = (AmpOut_2_reg >> 8) & 0x00FF;
      Serial.write(temp);
      temp = AmpOut_2_reg & 0x00FF;
      Serial.write(temp);
      Serial.write('\r\n');
      break;
    default:
      break;
  }
}





//*******************************************************************************

void Charger_Write(uint8_t Addr, uint8_t Value){

if (Addr==1){digitalWrite(CS_Charger_1,LOW);}
else if (Addr==2){digitalWrite(CS_Charger_2,LOW);}
  SPI.transfer(0x01);
  SPI.transfer(Value);
if (Addr==1){digitalWrite(CS_Charger_1,HIGH);}
else if (Addr==2){digitalWrite(CS_Charger_2,HIGH);}
}


//*******************************************************************************
uint16_t VoltStep(uint16_t Vconst){
  
  uint16_t temp;
  switch (Vconst){
    case 1:
      temp = volt_5;
      break;
    case 2:
      temp = volt_9;
      break;
    case 3:
      temp = volt_12;
      break;
    default:
      break;
  }
  return temp;
}

//*******************************************************************************

//*******************************************************************************

//*******************************************************************************
