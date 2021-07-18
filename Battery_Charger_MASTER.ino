#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);


 #define Key_UP     A2        //  Atmega328 pin = 25
 #define Key_DWN    A3        //  Atmega328 pin = 26
 #define Key_LEFT   A0        //  Atmega328 pin = 23
 #define Key_RIGHT  A1        //  Atmega328 pin = 24
 #define Relay_1     9        //  Atmega328 pin = 13
 #define Relay_2    10        //  Atmega328 pin = 14

boolean Key_UP_togle = false;
boolean Key_DWN_togle = false;
boolean Key_LEFT_togle = false;
boolean Key_RIGHT_togle = false;

boolean Slot_1_togle = false;
boolean Slot_2_togle = false;
boolean oneS_togle = false;

//uint8_t MENU_PAGE = 0;
uint8_t SubMENU_0 = 0;
uint8_t SubMENU_1 = 0;
uint8_t SubMENU_2 = 0;
uint8_t Cursor = 1;


uint8_t tempVolt_reg_1 = 0;
uint8_t tempVolt_reg_2 = 0;
uint8_t tempDelay_reg_1 = 0;
uint8_t tempDelay_reg_2 = 0;

uint8_t Volt_reg_1 = 0;
uint8_t Volt_reg_2 = 0;
uint16_t Amp_reg_1 = 0;
uint16_t Amp_reg_2 = 0;
uint8_t Delay_reg_1 = 0;
uint8_t Delay_reg_2 = 0;
uint16_t Timer_count_1 = 0;
uint16_t Timer_count_2 = 0;
uint8_t Counter_Divider = 0;
//uint8_t Counter_Divider_1S = 0;

uint16_t tempSerial[5];
uint8_t SerialCount = 0;


//*******************************************************************
// 1 Second Interupt

ISR(TIMER2_OVF_vect){
  Counter_Divider++;
  if (Counter_Divider == 64){     // 32 for 8mHz and 64 for 16mHz
    Counter_Divider  = 0;
    oneS_togle = true;
  }
  // ** Show Amp **********
}



//*******************************************************************

void setup() {
  pinMode( Key_UP, INPUT);
  digitalWrite( Key_UP, HIGH);
  pinMode( Key_DWN, INPUT);
  digitalWrite( Key_DWN, HIGH);
  pinMode( Key_LEFT, INPUT);
  digitalWrite( Key_LEFT, HIGH);
  pinMode( Key_RIGHT, INPUT);
  digitalWrite( Key_RIGHT, HIGH);

  pinMode( Relay_1, OUTPUT);
  digitalWrite( Relay_1, LOW);
  pinMode( Relay_2, OUTPUT);
  digitalWrite( Relay_2, LOW);


  Serial.begin(9600);

    // TIMER 2 for delay *************************************************
    
    TIMSK2 = 0x01;        //Timer2 INT Reg: Timer2 Overflow Interrupt Enable
    TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);  // Prescale 1024 (Timer/Counter started)

  lcd.init();
  lcd.init();
  lcd.backlight();
  Wire.begin(4);
  lcd.begin(20,4);
  lcd_MENU_Show(0,0);
  lcd_Cursor_Show();
}

//*******************************************************************

void loop() {
uint16_t temp;
uint16_t temp2;
  
  if (digitalRead(Key_LEFT) == 0){
    if(Key_LEFT_togle == false){
      Key_LEFT_togle = true;
      KeyButtonProcess(0);
    }
  }else{Key_LEFT_togle = false;}

  if (digitalRead(Key_RIGHT) == 0){
    if(Key_RIGHT_togle == false){
      Key_RIGHT_togle = true;
      KeyButtonProcess(1);
    }
  }else{Key_RIGHT_togle = false;}

  if (digitalRead(Key_UP) == 0){
    if (Key_UP_togle == false) {
      Key_UP_togle = true;
      KeyButtonProcess(2);
    }
  }else{Key_UP_togle = false;}

  if (digitalRead(Key_DWN) == 0){
    if(Key_DWN_togle == false){
      Key_DWN_togle = true;
      KeyButtonProcess(3);
    }
  }else{Key_DWN_togle = false;}

  if (oneS_togle == true){
    oneS_togle = false;
    
  //** Show the Value ****************************
    if ((digitalRead(Relay_1))&&(SubMENU_1 == 1)&&(SubMENU_2 == 3)){
      Serial.write(0x61);
      Serial.write('\r\n');
      Timer_count_1++;
      Show_time_left(Delay_reg_1);
      Show_Amp(Amp_reg_1);
    }
    if ((digitalRead(Relay_2))&&(SubMENU_1 == 2)&&(SubMENU_2 == 3)){
      Serial.write(0x63);
      Serial.write('\r\n');
      Timer_count_2++;
      Show_time_left(Delay_reg_2);
      Show_Amp(Amp_reg_2);
      }
  //** Compare Delay ****************************
    if (Delay_reg_1 != 0){                 // if Delay is FINISH
      if ((Delay_reg_1*60) == Timer_count_1){  
        Delay_reg_1 = 0;
        Volt_reg_1 = 0;
        Timer_count_1 =0;
        digitalWrite( Relay_1, LOW);
        Reset_lcd();
      }
    }
    if (Delay_reg_2 != 0){                 // if Delay is FINISH
      if ((Delay_reg_2*60) == Timer_count_2){  
        Delay_reg_2 = 0;
        Volt_reg_2 = 0;
        Timer_count_2 =0;
        digitalWrite( Relay_2, LOW);
        Reset_lcd();
      }
    }
    
  } // end of 1S

    // ****************************************  Read Power Amp from ATmega SLAVE ****************************************
    if (Serial.available()!=0) {
      temp = Serial.read();
      if (temp == '\n'){
        switch (tempSerial[0]){
          case 0x61:
            temp = (tempSerial[1] << 8) & 0xFF00;
            Amp_reg_1 = temp; 
            temp = tempSerial[2] & 0x00ff;
            Amp_reg_1 = Amp_reg_1 | temp;
            break;
          case 0x63:
            temp = (tempSerial[1] << 8) & 0xFF00;
            Amp_reg_2 = temp; 
            temp = tempSerial[2] & 0x00ff;
            Amp_reg_2 = Amp_reg_2 | temp;
            break;
          default:
            break;
        }
        //SerialProcess();
        SerialCount= 0;
      }else if (temp != '\r'){
        tempSerial[SerialCount] = temp;
        SerialCount++;
      }
    }
  
}

//**********************




//**********************
void Reset_lcd(){
  SubMENU_0 = 0;
  SubMENU_1 = 0;
  SubMENU_2 = 0;
  Cursor = 1;
  lcd_MENU_Show(0,0);
  lcd_Cursor_Show();
}


//*******************************************************************

void lcd_MENU_Show(uint8_t MENU_PAGE, uint8_t subMENU){
  
  lcd.clear();
  
  switch (MENU_PAGE){
    case 0:
        lcd.setCursor(0,0);
        lcd.print("________MENU________");
        lcd.setCursor(3,1);
        lcd.print("1.Charging Phone");
        lcd.setCursor(3,2);
        lcd.print("2.Maintenance");
        break;
    case 1:
        if (subMENU == 1){
          lcd.setCursor(0,0);
          lcd.print("__SELECT PHONE SLOT_");
          lcd.setCursor(3,1);
          lcd.print("Slot 1");
          lcd.setCursor(3,2);
          lcd.print("Slot 2");
        }else if (subMENU == 2){
          lcd.setCursor(1,1);
          lcd.print("look at the charge");
          lcd.setCursor(1,2);
          lcd.print("controller for more");
          lcd.setCursor(6,3);
          lcd.print("information");
          
        }
        break;
    case 2:
        if (subMENU == 1){
          lcd.setCursor(0,0);
          lcd.print("_______SLOT 1_______");
          lcd.setCursor(3,1);
          lcd.print("Set Voltage");
          lcd.setCursor(3,2);
          lcd.print("Set Timer");
          lcd.setCursor(3,3);
          lcd.print("Data");
        }else if (subMENU == 2){
          lcd.setCursor(0,0);
          lcd.print("_______SLOT 2_______");
          lcd.setCursor(3,1);
          lcd.print("Set Voltage");
          lcd.setCursor(3,2);
          lcd.print("Set Timer");
          lcd.setCursor(3,3);
          lcd.print("Data");
        }
        break;
    case 3:
        if (subMENU == 1){                      // Voltage Setting
          lcd.setCursor(1,0);
          lcd.print("SET VOLTAGE SLOT ");
          lcd.setCursor(19,0);
          lcd.print(SubMENU_1);
          lcd.setCursor(3,1);
          lcd.print("Cancel");
          lcd.setCursor(11,2);
          lcd.print("<= ");
          lcdprintTempVoltValue();
          lcd.setCursor(16,2);
          lcd.print("V =>");
          lcd.setCursor(3,3);
          lcd.print("Ok");
        }else if (subMENU == 2){                 // Delay Setting
          lcd.setCursor(2,0);
          lcd.print("SET TIMER SLOT ");
          lcd.setCursor(18,0);
          lcd.print(SubMENU_1);
          lcd.setCursor(3,1);
          lcd.print("Cancel");
          lcd.setCursor(3,3);
          lcd.print("Ok");
          
          lcd.setCursor(9,2);
          lcd.print("<= ");
          lcdprintTempTimerValue();
          lcd.setCursor(14,2);
          lcd.print("Mnt =>");

        }else if (subMENU == 3){                 // DATA
          lcd.setCursor(2,0);
          lcd.print("DATA INFORMATION");
          lcd.setCursor(0,1);
          lcd.print("Voltage: ");
          lcd.setCursor(0,2);
          lcd.print("Ampere: ");
          lcd.setCursor(0,3);
          lcd.print("Time Left: ");
          lcd.setCursor(17,1);
          lcdprintVoltValue();
          lcd.setCursor(19,1);
          lcd.print('V');

          lcd.setCursor(17,2);
          if (SubMENU_1 == 1){lcd.print(Amp_reg_1);}
          else if (SubMENU_1 == 2){lcd.print(Amp_reg_2);}
          lcd.setCursor(19,2);
          lcd.print('A');
          
          if (SubMENU_1 == 1){Show_time_left(Delay_reg_1);}
          else if (SubMENU_1 == 2){Show_time_left(Delay_reg_2);}

        }
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        break;
    default:
        break;
    
  }
  
}


//*******************************************************************

void lcd_Cursor_Show(){

  switch (Cursor){
    case 0:
        //lcd.setCursor(0,0);
        //lcd.print("=>");
        lcd.setCursor(0,1);
        lcd.print("  ");
        lcd.setCursor(0,2);
        lcd.print("  ");
        lcd.setCursor(0,3);
        lcd.print("  ");
        break;
    case 1:
        //lcd.setCursor(0,0);
        //lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("=>");
        lcd.setCursor(0,2);
        lcd.print("  ");
        lcd.setCursor(0,3);
        lcd.print("  ");
        break;
    case 2:
        //lcd.setCursor(0,0);
        //lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("  ");
        lcd.setCursor(0,2);
        lcd.print("=>");
        lcd.setCursor(0,3);
        lcd.print("  ");
        break;
    case 3:
        //lcd.setCursor(0,0);
        //lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("  ");
        lcd.setCursor(0,2);
        lcd.print("  ");
        lcd.setCursor(0,3);
        lcd.print("=>");
        break;
    default:
        break;
    
  }
  
}

//*******************************************************************

void KeyButtonProcess(byte nKey){
  if (SubMENU_2 != 0){
    //************************* Page 3 *******************************
    switch (nKey){
      case 0:                               // KEY LEFT
          if (SubMENU_2 == 1){Decrement_Value(false);}
          else if (SubMENU_2 == 2){Decrement_Value(true);}
          else if (SubMENU_2 == 3){
            Cursor = SubMENU_2;
            SubMENU_2 = 0;
            lcd_MENU_Show(2,SubMENU_1);
            lcd_Cursor_Show();
          }
          break;
      case 1:                               // KEY RIGHT
          if (SubMENU_2 == 1){Increment_Value(false);}
          else if (SubMENU_2 == 2){Increment_Value(true);}
          break;
      case 2:                               // KEY UP
          if (SubMENU_2 != 3){
            if (Cursor > 1){
              Cursor--;
              lcd_Cursor_Show();
            }
          }
          break;
      case 3:                               // KEY DWN
          if (SubMENU_2 != 3){
            if (Cursor < 3){
              Cursor++;
              lcd_Cursor_Show();
            }
          }
          break;
      default:
          break;
    }
    
  }else if (SubMENU_1 != 0){
    //************************* Page 2 *******************************
    switch (nKey){
      case 0:                               // KEY LEFT
          Cursor = SubMENU_1;
          SubMENU_1 = 0;
          lcd_MENU_Show(1,SubMENU_0);
          lcd_Cursor_Show();
          break;
      case 1:                               // KEY RIGHT
          SubMENU_2 = Cursor;
          lcd_MENU_Show(3, SubMENU_2);
          tempVolt_reg_1 = Volt_reg_1;      // Data Temp init
          tempVolt_reg_2 = Volt_reg_2;
          tempDelay_reg_1 = Delay_reg_1;
          tempDelay_reg_2 = Delay_reg_2;
          if (Cursor != 3){
           Cursor = 1;
           lcd_Cursor_Show();
          }
          break;
      case 2:                               // KEY UP
          if (Cursor > 1){
            Cursor--;
          }
          lcd_Cursor_Show();
          break;
      case 3:                               // KEY DWN
          if (Cursor < 3){
            Cursor++;
          }
          lcd_Cursor_Show();
          break;
      default:
          break;
    }
    
  }else if (SubMENU_0 != 0){
    //************************* Page 1 *******************************
    switch (nKey){
      case 0:                               // KEY LEFT
          Cursor = SubMENU_0;
          SubMENU_0 = 0;
          lcd_MENU_Show(0,0);
          lcd_Cursor_Show();
          break;
      case 1:                               // KEY RIGHT
        if (SubMENU_0 == 1){
          SubMENU_1 = Cursor;
          lcd_MENU_Show(2, SubMENU_1);
          Cursor = 1;                       // reset cursor
          lcd_Cursor_Show();
        }
          break;
      case 2:                               // KEY UP
        if (SubMENU_0 == 1){
          if (Cursor != 1) {
            Cursor = 1;
            lcd_Cursor_Show();
          }
        }
          break;
      case 3:                               // KEY DWN
        if (SubMENU_0 == 1){
          if (Cursor != 2) {
            Cursor = 2;
            lcd_Cursor_Show();
          }
        }
          break;
      default:
          break;
    }
    
  }else {
    //********************* Page 0 ***********************************
    switch (nKey){
      case 0:                               // KEY LEFT
          break;
      case 1:                               // KEY RIGHT
          SubMENU_0 = Cursor;
          lcd_MENU_Show(1, SubMENU_0);
          if (SubMENU_0 == 1){
            Cursor = 1;
            lcd_Cursor_Show();
          }
          break;
      case 2:                               // KEY UP
          if (Cursor != 1) {
            Cursor = 1;
            lcd_Cursor_Show();
          }
          break;
      case 3:                               // KEY DWN
          if (Cursor != 2) {
            Cursor = 2;
            lcd_Cursor_Show();
          }
          
          break;
      default:
          break;
    }
  }
}

//*******************************************************************
void Show_Amp(uint16_t Value){
  uint8_t temp;

  if (Value >= 100){
    temp = Value / 100;
    Value = Value - (temp*100);
    lcd.setCursor(15,2);
    lcd.print(temp);
  }else{
    lcd.setCursor(15,2);
    lcd.print('0');
  }
  
  if (Value >= 10){
    temp = Value / 10;
    Value = Value - (temp*10);
    lcd.setCursor(16,2);
    lcd.print(temp);
 }else{
    lcd.setCursor(16,2);
    lcd.print('0');
  }

    lcd.setCursor(17,2);
    lcd.print(temp);
    lcd.setCursor(18,2);
    lcd.print('m');
  
}

//*******************************************************************


void Show_time_left(uint8_t timer){
  uint8_t temp;
  uint8_t nPost = 15;
  uint8_t xCount;

  if (SubMENU_1 == 1){
    xCount = Timer_count_1;
  }else if (SubMENU_1 == 2){
    xCount = Timer_count_2;
  }
  
  uint16_t nTime = (timer*60) - xCount;
  if (nTime >= 600){
    temp = nTime / 600;
    nTime = nTime - (temp*600);
    lcd.setCursor(nPost,3);
    lcd.print(temp);
  } else {
    lcd.setCursor(nPost,3);
    lcd.print('0');
  }
  nPost++;
  if (nTime >= 60){
    temp = nTime / 60;
    nTime = nTime - (temp*60);
    lcd.setCursor(nPost,3);
    lcd.print(temp);
  } else {
    lcd.setCursor(nPost,3);
    lcd.print('0');
  }
  nPost++;
  lcd.setCursor(nPost,3);
  lcd.print(':');
  nPost++;
  if (nTime >= 10){
    temp = nTime / 10;
    nTime = nTime - (temp*10);
    lcd.setCursor(nPost,3);
    lcd.print(temp);
  } else {
    lcd.setCursor(nPost,3);
    lcd.print('0');
  }
  nPost++;
  lcd.setCursor(nPost,3);
  lcd.print(nTime);
}

//*******************************************************************

void Decrement_Value(boolean b1){
  switch (Cursor){
    case 1:                        // Back oe Cancel
            Cursor = SubMENU_2;
            SubMENU_2 = 0;
            lcd_MENU_Show(2,SubMENU_1);
            lcd_Cursor_Show();
        break;
    case 2:
        if (SubMENU_1 == 1){
            if (b1){
              if (tempDelay_reg_1 > 0){
                tempDelay_reg_1--;
                lcdprintTempTimerValue();
                }
            }else{
              if (tempVolt_reg_1 > 0){
                tempVolt_reg_1--;
                lcd.setCursor(14,2);
                lcdprintTempVoltValue();
                }
            }
        }else if (SubMENU_1 == 2){
            if (b1){
              if (tempDelay_reg_2 > 0){
                tempDelay_reg_2--;
                lcdprintTempTimerValue();
                }
            }else {
              if (tempVolt_reg_2 > 0){
                tempVolt_reg_2--;
                lcd.setCursor(14,2);
                lcdprintTempVoltValue();
                }
            }
        }
        
        break;
    case 3:
          if (SubMENU_1 == 1){      // Data Temp SAVE
            Volt_reg_1 = tempVolt_reg_1;
            Delay_reg_1 = tempDelay_reg_1;
            Timer_count_1 = 0;
          }else if (SubMENU_1 == 2){
            Volt_reg_2 = tempVolt_reg_2;
            Delay_reg_2 = tempDelay_reg_2;
            Timer_count_2 = 0;
          }
            Cursor = SubMENU_2;
            SubMENU_2 = 0;
            lcd_MENU_Show(2,SubMENU_1);
            lcd_Cursor_Show();
            STARTchargher();
        break;
    default:
        break;
  }
}
//*******************************************************************
void Increment_Value(boolean b1){
  switch (Cursor){
    case 1:                        // Back oe Cancel
            Cursor = SubMENU_2;
            SubMENU_2 = 0;
            lcd_MENU_Show(2,SubMENU_1);
            lcd_Cursor_Show();
        break;
    case 2:
        if (SubMENU_1 == 1){
            if (b1){
              if (tempDelay_reg_1 < 10){
                tempDelay_reg_1++;
                lcdprintTempTimerValue();
                }
            }else{
              if (tempVolt_reg_1 < 3){
                tempVolt_reg_1++;
                lcdprintTempVoltValue();
                }
            }
        }else if (SubMENU_1 == 2){
            if (b1){
              if (tempDelay_reg_2 < 10){
                tempDelay_reg_2++;
                lcdprintTempTimerValue();
                }
            }else {
              if (tempVolt_reg_2 < 3){
                tempVolt_reg_2++;
                lcdprintTempVoltValue();
                }
            }
        }
        
        break;
    case 3:                         // Data Temp SAVE
          if (SubMENU_1 == 1){
            Volt_reg_1 = tempVolt_reg_1;
            Delay_reg_1 = tempDelay_reg_1;
            Timer_count_1 = 0;
          }else if (SubMENU_1 == 2){
            Volt_reg_2 = tempVolt_reg_2;
            Delay_reg_2 = tempDelay_reg_2;
            Timer_count_2 = 0;
          }
          Cursor = SubMENU_2;
          SubMENU_2 = 0;
          lcd_MENU_Show(2,SubMENU_1);
          lcd_Cursor_Show();
          STARTchargher();
        break;
    default:
        break;
  }
}

//*******************************************************************

void STARTchargher(){

  //if (Slot_1_togle == false){
    if ((Volt_reg_1 != 0)&&(Delay_reg_1 != 0)){
      //Slot_1_togle = true;
      digitalWrite( Relay_1, HIGH);
      // Send Comman to SLAVE
      Serial.write(0x60);
      Serial.write(Volt_reg_1);
      Serial.write('\r\n');
    }
  //}
  
  //if (Slot_2_togle == false){
    if ((Volt_reg_2 != 0)&&(Delay_reg_2 != 0)){
      //Slot_2_togle = true;
      digitalWrite( Relay_2, HIGH);
      // Send Comman SLAVE
      Serial.write(0x62);
      Serial.write(Volt_reg_2);
      Serial.write('\r\n');
      
    }
  //}
}


//*******************************************************************

void lcdprintVoltValue(){
  uint8_t xVal;
  if (SubMENU_1 == 1){
    xVal = Volt_reg_1;
  }else if (SubMENU_1 == 2){
    xVal = Volt_reg_2;
  }
  
  switch (xVal){
    case 0:
        lcd.print('0');
        break;
    case 1:
        lcd.print('5');
        break;
    case 2:
        lcd.print('9');
        break;
    case 3:
        lcd.print("12");
         break;
    default:
        break;
  }
          
}

//*******************************************************************

void lcdprintTempVoltValue(){
  uint8_t xVal;
  if (SubMENU_1 == 1){
    xVal = tempVolt_reg_1;
  }else if (SubMENU_1 == 2){
    xVal = tempVolt_reg_2;
  }

  if (xVal < 3){
    lcd.setCursor(14,2);
    lcd.print(' ');
    lcd.setCursor(15,2);
  }else{lcd.setCursor(14,2);}
  
  switch (xVal){
    case 0:
        lcd.print('0');
        break;
    case 1:
        lcd.print('5');
        break;
    case 2:
        lcd.print('9');
        break;
    case 3:
        lcd.print("12");
         break;
    default:
        break;
  }
          
}

//*******************************************************************

void lcdprintTempTimerValue(){
  uint8_t xVal;
  if (SubMENU_1 == 1){
    xVal = tempDelay_reg_1;
  }else if (SubMENU_1 == 2){
    xVal = tempDelay_reg_2;
  }

  if (xVal == 10){lcd.setCursor(12,2);}
  else {
    lcd.setCursor(12,2);
    lcd.print(' ');
    lcd.setCursor(13,2);
  }
  lcd.print(xVal);
}
