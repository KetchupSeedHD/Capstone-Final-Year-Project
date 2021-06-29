#include <rotary.h>   
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Define rotary encoder pins
#define PINA A0
#define PINB A1
#define PUSHB A2

LiquidCrystal_I2C lcd(0x27, 20, 4);
Rotary r = Rotary(PINA, PINB, PUSHB); 

int encoderVal = 0;
int cursorVal = 0;
int menu = 0;
boolean encoderHandler = false;

void setup() {
  // INIT LCD STUFF:
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.begin(20, 4);
  Wire.begin(4);
}

void loop() {
    unsigned char encoder = r.process();
    
    if (cursorVal < 0 || cursorVal > 3){
      cursorVal = 0;
    }

    //PLACE CURSOR:
    lcd.setCursor(0,cursorVal);
    lcd.print(">");

    //MOVE CURSOR:
   
        if (encoder == DIR_CCW) {
           lcd.clear();
           cursorVal++;
        } 
        else if (encoder == DIR_CCW){
           lcd.clear();
           cursorVal--;
        }
 

  switch(menu){
    case 0: //MAIN MENU
        lcd.setCursor(1,0);
        lcd.print("1. Charging Phone");
        lcd.setCursor(1,1);
        lcd.print("2. Maintenance");
        
        if (r.buttonPressedReleased(25)) {
          lcd.setCursor(1,3);
          lcd.print("Going to menu 2");
          
            if (cursorVal == 0){
              lcd.clear(); 
              menu = 1;
            }
            if (cursorVal == 1){
              lcd.clear(); 
              menu = 2;
            }
         }
         break;
        
    case 1: //SUB-MENU 1
        lcd.setCursor(1,0);
        lcd.print("Select Phone Slot:");
        
        lcd.setCursor(1,1);
        lcd.print("Slot 1");
        lcd.setCursor(1,2);
        lcd.print("Slot 2");

        if (r.buttonPressedReleased(25)) {
            if (cursorVal == 1){
              menu = 3;
            }
            if (cursorVal == 2){
              menu = 3;
            }  
         }
         break;
 
    case 2: //SUB-MENU 2
        lcd.setCursor(1,0);
        lcd.print("Check Manual!");
        
        if (r.buttonPressedReleased(25)) {
          menu = 0;
         }
        break;

    case 3: //SUB-MENU 1.1
        lcd.setCursor(1,0);
        lcd.print("Set Voltage:");
        
        lcd.setCursor(1,1);
        lcd.print("5V");
        lcd.setCursor(1,2);
        lcd.print("9V");
        lcd.setCursor(1,3);
        lcd.print("12V");
        break;
  }
}
