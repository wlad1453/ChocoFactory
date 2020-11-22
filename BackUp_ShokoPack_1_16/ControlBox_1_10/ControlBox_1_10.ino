#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

const byte ROWS = 4; //four rows const byte  
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},      // 1 2 3 F1
  {'4','5','6','B'},      // 4 5 6 F2
  {'7','8','9','C'},      // 7 8 9 F3
  {'*','0','#','D'}       // Start 0 Stop F4
  };
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad 
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad
int i2caddress = 0x21; // PCF8574 address
Keypad_I2C  kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS, i2caddress, PCF8574 );

char key;                                                 // Key pad reading result 1..9, 0, A, B, C, D
bool singleCycle(false);                                  // Single cycle mode on/off
bool stepWise(false);                                     // Step mode on/off. Each step on pressing "Start" button
uint8_t delayBTWcycles(0);                                // Delay between single cycles in Sec. E.g. Sec*1000

bool startPrep(false);
uint8_t startDelay(0);                                     // Start delay in Sec (Sec*1000)
bool shutdownProc(false);                                  // Special shutdown procedure. Also by emergency stop

uint16_t cycleNum(0), stepNum(0);                           // Cycle counter, step couter
uint16_t senA(256), senB(256), senC(256), senD(256), senE(256), senF(256), senG(256), senH(256);
uint8_t slowFac (1);                                        // Slowdown factor

#define maxUnit 6
struct MachineUnit {
  uint8_t num;
  String unitName;
  bool active;
};

MachineUnit units[6]{
  1, "feed",      false,
  2, "elevator",  false, 
  3, "knife",     false,
  4, "foil",      false,
  5, "cutter",    false,
  6, "wrapper",   false  
};

#include "ControlBox.h"         // A set of ConrloBox functions

void setup() {

  Serial.begin(9600);
  kpd.begin( makeKeymap(keys) );  
  lcd.begin(20, 4);             // (20,4) initialize the lcd for 20 chars 4 lines and turn on backlight

  greeting();                // Introduction on the LCD screen
}

void loop() {  
  
  key = kpd.getKey();  
  
  if (key){                                           // Start using menu
    
    if ( key == 'D' ) {
      Menu(); 
      greeting();
    } else if ( key == '*' ) {                        // Start machine movement (this time simulation)
      
      runScreen();                                    // print out the default screen with cycle count and sensors reading
      cycleNum = 0;
      
      while( key != '#' ) {                 
        clearSens();
        cycleNum++; lcd.setCursor( 4, 1); lcd.print( cycleNum );
              
        for( uint8_t stepNum = 0; stepNum < 52; stepNum++) {
          switch ( stepNum ) {
            case 20: printSens('A'); break;
            case 10: printSens('B'); break;
            case  2: printSens('C'); break;
            case 29: printSens('D'); break;
            case  9: printSens('E'); break;
            case 32: printSens('F'); break;
            case 41: printSens('G'); break;
            case 12: printSens('H'); break;            
          }
          
          lcd.setCursor( 4, 3); lcd.print( "  " );
          lcd.setCursor( 4, 3); lcd.print( stepNum + 1 ); 
          delay ( 20 * slowFac );
          
          key = kpd.getKey();
          if ( stepWise ) {
            lcd.setCursor ( 0, 3);  lcd.print( "del" ); 
            while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
          }
                      
          if ( key == '#' ) break;      // Break the 'for' loop end exit the while loop
          
        } // end for ( 0...52 ), e.g. end of the cycle  

        if ( singleCycle ) while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
        
        delay ( delayBTWcycles * 1000 ); 
              
        Serial.println("break third");       
      } // end while( key != '#' )
      
    greeting();
    } // end else if ( key == '*' ];    // Start machine movement (this time simulation)
    
  } // end if( key ) e.g. if any key is pressed
} // end void loop()


void activeUnit(uint8_t maxUnt) {      // Switches on/off the units of the machine
  uint8_t unitN(0);                    // Unit number {0...maxUnit}
  uint8_t LCDoffset(0);                // Shift of the LCD window against the whole Units array
  uint8_t LCDrow(0);                   // LCD screen row {0..3 for 20x4}
  
  showUnits(LCDoffset);
  
  lcd.setCursor ( 13, LCDrow ); lcd.cursor(); lcd.blink();   // Fault! should be row!
  
  key = NULL;  
  while ( key != '4' && key != 'D' ) {
    
    key = kpd.getKey();  
    if (key){      
      if ( key == '2' ) {
        if ( LCDrow > 0 ) { LCDrow--; unitN--; } 
        else if ( LCDoffset > 0 ) { unitN--; LCDoffset--; showUnits ( LCDoffset );}    
      }
      if ( key == '8' ) {
        if ( LCDrow < 3 ) { LCDrow++; unitN++; } 
        else if ( LCDoffset < maxUnt - 4 ) { unitN++; LCDoffset++; showUnits ( LCDoffset );}    
      }
      lcd.setCursor ( 13, LCDrow ); lcd.cursor(); lcd.blink();                    // Shift the cursor to the current position
                                                                                  // unitN = LCDrow + LCDoffset
            
      if ( key == '6' || key == '5' ) {                                           // 'Enter' or change the value
        units[unitN].active = !units[unitN].active;                               // Change unit status. On/off particular unit
        lcd.setCursor ( 12, LCDrow );                                             // LCDrow = unitN - LCDoffset 
        if( units[unitN].active == true ) lcd.print("Y"); else lcd.print("N");    // Print unit status
      }
      
      if ( key == '4' || key == 'D' ) {                                           // Exit
        key = NULL;
        return; 
        }
    } // End if ( key )
    delay(20);
        
  } // End while ( key != '4' && key != 'D' )
} // End activeUnit()

void Menu() {
    
  mainMenu();
  
  key = NULL;
  while( key != 'D' ) {
   
    if ( key ) {
      switch ( key ) {
        case '1': operatingMode();      break;
        case '2': activeUnit(maxUnit);  break;
        case '3': startingOptions();    break;
        case '4': timingFactors();      break;
      }
      mainMenu();
      // delay( 500 );
    }
    
    key = NULL; key = kpd.getKey();
    
  }  // end while( key != '*' )
  return;  
} 

void operatingMode() {
    
  operatingModeLCD();
 
  key = NULL;
  while( key != 'D' ) {
    
    key = kpd.getKey();
    if ( key ) {
      switch ( key ) {
        case '1': singleCycle = !singleCycle;
                  lcd.setCursor ( 17, 0);  
                  if ( singleCycle == true ) lcd.print( "Y" ); 
                  else lcd.print ( "N" );
                  break;
        case '2': stepWise = !stepWise;
                  lcd.setCursor ( 17, 1);  
                  if ( stepWise == true ) lcd.print( "Y" ); 
                  else lcd.print ( "N" );   
                  break;
        case '3': slowFac *= 2; 
                  if ( slowFac == 32 ) slowFac = 1;
                  lcd.setCursor ( 17, 2);  lcd.print( "  " ); 
                  lcd.setCursor ( 17, 2);  lcd.print( slowFac );  
                  break;
        case '4': delayBTWcycles += 5;  
                  if ( delayBTWcycles == 35 ) delayBTWcycles = 0;    
                  lcd.setCursor ( 17, 3);  lcd.print( "  " ); 
                  lcd.setCursor ( 17, 3);  lcd.print( delayBTWcycles );  
                  break;
      }
    }
  }  // end while( key != 'D' )
  return;  
} 


  
  
