  /*
  BeltDriveShifterRotator //Rotates the shifter arm up (1) and down (0)
  BeltDriveShifterSlider  // Slide a peace of chocolate from the belt drive to the right edge of the table
  MainPusher        // Slide a peace of chocolate to the center of the working zone
  LabelKnife        // grabs a single lable from the banch
  LabelVacuumValve  // Switch on the vacuum to stick the lower lable
  FoilGreifer       // Grasp the adge of the package (foil and lable)
  FoilPuller        // Pulls the package onto the peace of chocolate
  FoilCutter        // Cutt a peace of foil
  UpperRodeHolder   // Holds the upper side of the cocolate
  BottomRodeSupport // Heaves the peace of chocolate and the package of foil and lable and pudhes it through the guiding window
  SideFolder        // Fold together foil wings (front and hind ones)
  MainFolder        // pushes the right package wing (foil and label) beneath the peace of chocolate
  GlueThrower       // throws two drops of glue onto the left end of the label
  LeftGreiferLifter // greies and shift the ready wrapped peace of chocolate
  LeftGreiferShifter

  2004 LCD

  Version 28.01.2020

  Next tasks
  1. Pressure sensor implementation - > Alarm signal
  2. Launching procedure !!! --> "Ready to Work" signal (To clear the possibility of an automated pressure air valve)
  3. Shut-down procedure (should be done as an interrupt routine)
  4. Breakdown stop

  Version 04.03.20
Base version PPlate_0_27.ino from 17.03.2020
*/
// #include <avr/io.h>
  
#include <ShokoDefine.h>      // Controller ports definitions
#include <Cyclogramm.h>       // Timing factors. Using #define ProcessNumber. Initialization of the machine mProcs array. 
                              // Each line describes a process (element movement, switching etc.)
// ********************** 25.03 ********************
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
// uint16_t senA(256), senB(256), senC(256), senD(256), senE(256), senF(256), senG(256), senH(256);
uint8_t slowFac (1);                                        // Slowdown factor

#define maxUnit 6
struct MachineUnit {
  uint8_t num;
  String unitName;
  bool active;
};

MachineUnit units[6]{
  1, "feed",      true,
  2, "elevator",  true, 
  3, "knife",     true,
  4, "foil",      true,
  5, "cutter",    true,
  6, "wrapper",   true  
};

#include "ControlBox1.h"         // A set of ConrloBox functions

char sensor(NULL);           // Name of a sensor which becomes active A B C D E F G H ( further: IJKL MNOP )

uint16_t eventTimePnt[52];  // Start and stop time points of various machine processes (event time points)
uint8_t eventSequNum[52];   // Sequence number of an events. Appearance of the event in the queue within the sycle (trigger points)
unsigned long CycleStart;   // Cycle start time
// uint16_t Counter (0);       // Cycle counter


boolean feed(1), elevator(1), knife(1), foil(1), cutter(1), wrapper(1);       // default units configuration
boolean startSeq(0), stopSeq(0);                                              // starting and stop procedure on/off

void setup() {
  
  uint8_t btw;                // A variable to store the index in the bubble sorting procedure
  boolean change;
  
  DDRA = 0xFF;                  // Port A is set as Output PA0 -> pin 22 ... PA7 -> pin 29
  DDRC = 0xFF;                  // Port C is set as Output PC0 -> pin 37 ... PC7 -> pin 30
  DDRL = 0x00;                  // Port L is set as Input  PL0 -> pin 49 ... PL7 -> pin 42
  // PORTL = 0xFF;              // switches on pull up resistors 

  PORTA = 0x00;
  PORTB = 0x00;

  Serial.begin(115200);           // Serial communication with PC
  kpd.begin( makeKeymap(keys) );  
  lcd.begin(20, 4);             // (20,4) initialize the lcd for 20 chars 4 lines and turn on backlight

  greeting();                // Introduction on the LCD screen


  // CyclCalc calculation;
  // calculation.stopCalc();

  for (int i = 0; i < ProcessNumber; i++) {
    mProc[i].End = mProc[i].Start + mProc[i].Duration;  // Stop time calculation (mProc[n].End) and signal level setting at the sycle start 
    if (mProc[i].End > cycleLength) {                   // if the process ends in the next cycle
      mProc[i].startLow = false;
      mProc[i].End -= cycleLength;
    }
  } 

  // Launching procedure !!! --> "Ready to Work" signal
  // Shut-down procedure (should be done as an interrupt routine)
  // Breakdown stop

  // Trigger points (events time points) calculation. The Start events have even number and the Stop events get odd ones
  
  for (int i = 0; i < ProcessNumber * 2; i++) {
    if (i % 2 != 0) eventTimePnt[i] = mProc[ (i-1)/2 ].End;  // Refer to the mProc table. mProc - Machine process, e.g. each process woth two events - start and stop
    else eventTimePnt[i] = mProc[i/2].Start;
    eventSequNum[i] = i;                                     // All the step indexes are in a row, prepared for the later sorting
                                                             // Number of a certain event which appears in the queue by increasing of 'i' from 0 to 52
    // Serial.print(i); Serial.print("  "); Serial.print(eventSequNum[i]); Serial.print("  "); Serial.println(eventTimePnt[i]);
  }
  
  // **********  Bubble sorting  ****************
  // Machine steps sorting. Building of a mapping table where all the steps are timely in a row
  
  do {
    change = false;
    for (int i = 0; i < (ProcessNumber * 2 - 1); i++) {    // 0..51 - 52 events. (PrNu * 2 - 1) = 51 - the last Nu in the sequence

      if ( eventTimePnt[eventSequNum[i]] > eventTimePnt[eventSequNum[i + 1]]) {
        btw = eventSequNum[i];
        eventSequNum[i] = eventSequNum[i + 1];
        eventSequNum[i + 1] = btw;
        change = true;
      }
    }
  } while (change == true);

  MProcTable();
  SequenceTable();

  if ( startSeq ) StartingSequence();
  
 
} // End Setup

  
void loop() {
  // To do:
  // Main working sequence for(int i = 0; i < sizeOf(eventSequNum()); i++)
  // If some mProcs happen -> FullStop (Stop switch pressed, no chokolate with greifer running-out, not allowed situation etc.)
  randomSeed(analogRead(0));
  uint16_t  timePoint(0);
  uint8_t   eventN(0);
  uint8_t   ran = random(10, 30);

  key = kpd.getKey();  
  if (key){                                           // Start using menu    
    if ( key == 'D' ) {
      Menu(); 
      greeting();
    } else if ( key == '*' ) {                        // Start working cycle
      runScreen();                                    // print out the default screen with cycle count and sensors reading
      cycleNum = 0;
      while( key != '#' ) {                 
        clearSens();
        cycleNum++; lcd.setCursor( 4, 1); lcd.print( cycleNum );
  /*      
// ************************ ----------------------------------- *********************************
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
          delay ( 20 * slowFac );
      // *** Copied ***    
          lcd.setCursor( 4, 3); lcd.print( "  " );
          lcd.setCursor( 4, 3); lcd.print( stepNum + 1 ); 
         
          key = kpd.getKey();
          if ( stepWise ) {
            lcd.setCursor ( 0, 3);  lcd.print( "del" ); 
            while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
          }
                      
          if ( key == '#' ) break;      // Break the 'for' loop end exit the while loop
        // *** Copied ***    
          
        } // end for ( 0...52 ), e.g. end of the cycle  
// *** Copied ***    
        if ( singleCycle ) while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
        
        delay ( delayBTWcycles * 1000 ); 
 // *** Copied ***                 
        Serial.println("break third");       

// ************************ ----------------------------------- *********************************
   */  
                                                                     
  Serial.print(F("\n  ***************  Units: ")); 
  Serial.print( units[0].active + 2*units[2].active + 4*units[3].active + 8*units[4].active + 16*units[1].active + 32*units[5].active, BIN ); 
  Serial.print(F("  ***************  Cycle number: ")); Serial.println( cycleNum );                    
  
                                        // ************  Sycle control logic ************  
  CycleStart = millis();                                                                  // Starting point of each cycles  
  eventN = eventSequNum[0];
  
  for (uint8_t stepNum = 0; stepNum < ProcessNumber * 2; ) {                                              // Call all events in the cycle sequentually.
    timePoint = millis() - CycleStart;   
   
    if ( eventN % 2 == 0) {                                                       // In case of an EVEN number it is a Process Start event   

      if ( timePoint >= mProc[ eventN / 2].Start * slowFac  ){                     //If the Start point is reached check for the action
        action(stepNum, timePoint, NULL);

        stepNum++; eventN = eventSequNum[stepNum];                                            // Increments each time the current event is reached. (next time check the next event) 
      }      
      //deBugPrintE();
    } else {                                                                      // In case of ODD numbers it is a Process End event        
        if ( timePoint >= mProc[ (eventN - 1) / 2 ].End * slowFac ){                 //If the Stop point is reached check for the action
        action(stepNum, timePoint, NULL);
        stepNum++; eventN = eventSequNum[stepNum];
        }      
      //deBugPrintO();
    }

    // *******************    Sensor reading section. Made once each polling   *************************************
    /* Normal case. PLC reacts to the real world signals, which appear somewhere at the end of an appropriate move
     *  
    if ( mProc[14].Active && !( PINL & ( 1 << (49 - sensorA) ) )) sensor = 'A';     // SensorA reading, Upper rod down. Stop the movement. All connectors are open
      
    if ( mProc[17].Active && !( PINL & ( 1 << (49 - sensorB) ) )) sensor = 'B';     // Stops the movement Bottom rode Up 1st.       
                                                                                    // "Bottom rode Up" and "Upper rode Up 1st" with a piece of chocolate squeezed inbetween 
    if ( mProc[16].Active && !( PINL & ( 1 << (49 - sensorC) ) )) sensor = 'C';     // Upper rode Up 2nd. Stop the movement of bottom rode. All connections are open 
   
    if ( mProc[18].Active && !( PINL & ( 1 << (49 - sensorD) ) )) sensor = 'D';     // Stops the movement Bottom rode down at the lower end of cylinder 

    if ( mProc[5].Active  && !( PINL & ( 1 << (49 - sensorE) ) )) sensor = 'E';     // SensorE reading, Lable Knife ahead. Stop the movement. 5/3 NC
   
    if ( mProc[7].Active  && !( PINL & ( 1 << (49 - sensorF) ) )) sensor = 'F';     // SensorE reading, Lable Knife back. Stop the movement. 5/3 NC
  
    if ( mProc[9].Active  && !( PINL & ( 1 << (49 - sensorG) ) )) sensor = 'G';     // SensorG reading, Foil Grabber ahead. Stop the movement. 5/3 NC
     
    if ( mProc[10].Active && !( PINL & ( 1 << (49 - sensorH) ) )) sensor = 'H';     // SensorH reading, Foil Grabber back 1st move. Stop the movement. 5/3 NC
    */

    // ********************* Sensor reaction simulation *************************************

    if ( mProc[14].Active && timePoint >= ( 990 - ran ) ) sensor = 'A';     // SensorA reading, Upper rod down. Stop the movement. All connectors are open
      
    if ( mProc[17].Active && timePoint >= ( 340 - ran ) ) sensor = 'B';     // Stops the movement Bottom rode Up 1st.       
                                                                            // "Bottom rode Up" and "Upper rode Up 1st" with a piece of chocolate squeezed inbetween 
    if ( mProc[16].Active && timePoint >= ( 550 - ran ) ) sensor = 'C';     // Upper rode Up 2nd. Stop the movement of bottom rode. All connections are open 
   
    if ( mProc[18].Active && timePoint >= ( 540 - ran ) ) sensor = 'D';     // Stops the movement Bottom rode down at the lower end of cylinder 

    if ( mProc[5].Active  && timePoint >= ( 970 - ran ) ) sensor = 'E';     // SensorE reading, Lable Knife ahead. Stop the movement. 5/3 NC
   
    if ( mProc[7].Active  && timePoint >= ( 430 - ran ) ) sensor = 'F';     // SensorE reading, Lable Knife back. Stop the movement. 5/3 NC
  
    if ( mProc[9].Active  && timePoint >= ( 450 - ran ) ) sensor = 'G';     // SensorG reading, Foil Grabber ahead. Stop the movement. 5/3 NC
     
    if ( mProc[10].Active && timePoint >= ( 708 - ran ) ) sensor = 'H';     // SensorH reading, Foil Grabber back 1st move. Stop the movement. 5/3 NC
    
    if ( sensor ) {     
      switch ( sensor ) {
        case 'A': action( 29, timePoint, sensor ); lcd.setCursor( 11, 0); break;                          // ProcN = 14 Stop "Upper rode down" 
        case 'B': action( 35, timePoint, sensor ); lcd.setCursor( 11, 1); break;                          // ProcN = 17 Stop "Bottom rode Up"  
        case 'C': action( 33, timePoint, sensor ); lcd.setCursor( 11, 2); break;                          // ProcN = 16 Stop "Upper rode Up 2nd"
        case 'D': action( 37, timePoint, sensor ); lcd.setCursor( 11, 3); break;                          // ProcN = 18 Stop "Bottom rode Down"
        case 'E': action( 11, timePoint, sensor ); lcd.setCursor( 17, 0); break;                          // ProcN = 5  Stop Lable Knife ahead (work) movement.
        case 'F': action( 15, timePoint, sensor ); lcd.setCursor( 17, 1); break;                          // ProcN = 7  Stop Lable Knife back movement.
        case 'G': action( 19, timePoint, sensor ); lcd.setCursor( 17, 2); break;                          // ProcN = 9  Stop Foil puller ahead (work) movement.
        case 'H': action( 21, timePoint, sensor ); lcd.setCursor( 17, 3); break;                          // ProcN = 10 Stop Foil puller back (pull) 1st movement.                
      } 
      lcd.print( timePoint );     
      sensor = NULL;
    }

    lcd.setCursor( 4, 3); lcd.print( "  " );
    lcd.setCursor( 4, 3); lcd.print( stepNum ); 
          
    key = kpd.getKey();
          if ( stepWise ) {
            lcd.setCursor ( 0, 3);  lcd.print( "del" ); 
            while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
          }
                      
          if ( key == '#' ) break;      // Break the 'for' loop end exit the while loop
   } // End  for (stepNum = 0; stepNum < ProcNumber * 2; ) 52 steps
   
   if ( singleCycle ) while( key != '*' && key != '#' ) { key = kpd.getKey(); delay( 20 ); }
   
   delay ( delayBTWcycles * 1000 );   // Delay between cycles in seconds

      } // End while ( key == '#' ) - Stop button
    }   // End else if ( key == '*' ) - Start button
  }     // End of if ( key )
   
}    // End void loop()

void action(uint8_t i, uint16_t tPoint, char sens) {
  String mMove;
  uint8_t j;
  if ( sens ) {                       // Direct 
    j = i; 
    mMove = "Sensor " + String( sens ) + " \t";
  } else {
    j = eventSequNum[i]; 
    mMove += "         \t";  
  }
                                       // Logical filter of switched on units. If no units are switched on - just 'return'
   if(  ( units[0].active && (j <= 9))                             // feed = units[0]           
     || ( units[2].active && (j >= 10) && (j <= 15))               // knife = units[2]
     || ( units[3].active && (j >= 18) && (j <= 25))               // foil = units[3]
     || ( units[4].active && ((j == 26) || (j == 27) ))            // cutter = units[4]
     || ( units[1].active && (j >= 28) && (j <= 37))               // elevator = units[1]
     || ( units[5].active && (j >= 38) && (j <= 43)) ) ;           // wrapper = units[5]
   else return;

  if ( j % 2 )  mMove += mProc[ ( j - 1 ) / 2 ].Name + String(" Stop at ");
  else          mMove += mProc[ j / 2 ].Name         + String(" Start at "); 
  
  Serial.print( i ); Serial.print("  "); Serial.print( j ); Serial.print("  ");
  Serial.print( mMove ); Serial.println( tPoint ); 
   
  
    switch (j) {  
      //*** Feed 
      case 0: PORTA |= ( 1 << (BeltShifter - 22));        // Belt shifter. Shift the piece of chock. from the belt. 5/2 valve                     
               mProc[0].Active = true;
               break; 
      case 1: PORTA &= ~( 1 << (BeltShifter - 22));       // Belt shifter. Come back to the starting position. Should be raised up                         
               mProc[0].Active = false;                   // by a solenoid or additional cylinder
               break; 
      case 6: PORTA |= ( 1 << (MainPusher - 22));      // Main Pusher. Pushes the peece of chock. onto the table. 5/2 valve                     
               mProc[3].Active = true;
               break; 
      case 7: PORTA &= ~( 1 << (MainPusher - 22));     // Main Pusher. Come back to the starting pos. Waiting for the next                      
               mProc[3].Active = false;                   // piece of chock.
               break; 




      //******************* /* */
      case 10: PORTA |= ( 1 << (LblKnife_work - 22));        // Start Lable Knife ahead (work) movement.                          
               mProc[5].Active = true;
               break; 
      case 11: PORTA &= ~( 1 << (LblKnife_work - 22));       // Stop Lable Knife ahead (work) movement.                          
               mProc[5].Active = false;
               break; 
      case 12:                                               // Open Glue thrower. To implement later                          
               mProc[6].Active = true;
               break; 
      case 13:                                               // Close Glue thrower. To implement later                      
               mProc[6].Active = false;
               break; 
              
      case 14: PORTA |= ( 1 << (LblKnife_back - 22));        // Start Lable Knife back movement.                           
               mProc[7].Active = true;
               break; 
      case 15: PORTA &= ~( 1 << (LblKnife_back - 22));       // Stop Lable Knife back movement.                          
               mProc[7].Active = false;
               break;
       
      case 18: PORTA |= ( 1 << (FPuller_work - 22));        // Start Foil puller ahead (work) movement.                          
               mProc[9].Active = true;
               break; 
      case 19: PORTA &= ~( 1 << (FPuller_work - 22));       // Stop Foil puller ahead (work) movement.                          
               mProc[9].Active = false;
               break; 
      case 20: PORTA |= ( 1 << (FPuller_back - 22));        // Start Foil puller back (pull) 1st movement.                          
               mProc[10].Active = true;
               break; 
      case 21: PORTA &= ~( 1 << (FPuller_back - 22));       // Stop Foil puller back (pull) 1st movement.                          
               mProc[10].Active = false;
               break; 
      case 22: PORTA |= ( 1 << (FPuller_back - 22));        // Start Foil puller back (pull) 2nd movement.                          
               mProc[11].Active = true;
               break; 
      case 23: PORTA &= ~( 1 << (FPuller_back - 22));       // Stop Foil puller back (pull) 2nd movement.                          
               mProc[11].Active = false;
               break; 
      case 24: PORTA |= ( 1 << (FoilGraber - 22));        // Foil Graber On (clanch).                          
               mProc[12].Active = true;
               break; 
      case 25: PORTA &= ~( 1 << (FoilGraber - 22));        // Foil Graber Off (release).                      
               mProc[12].Active = false;
               break; 
      case 26: PORTC |= ( 1 << (37 - FoilCutter));         // Foil Cutter On (cutt foil).                          
               mProc[13].Active = true;
               break; 
      case 27: PORTC &= ~( 1 << (37 - FoilCutter));        // Foil Cutter Off (release).                      
               mProc[13].Active = false;
               break; 
                   
      case 28: PORTC |= ( 1 << (37 - URode_work));          //Start "Upper rode down"   
               // Serial.print("Start Upper rode down at   "); Serial.println( tPoint );                                    
               mProc[14].Active = true;
               break;    
      case 29: PORTC &= ~( 1 << (37 - URode_work));         //Stop "Upper rode down" 
               // Serial.print("Stop Upper rode down at   "); Serial.println( tPoint );  
               mProc[14].Active = false;
               break;   
      case 30: mProc[15].Active = true;   break;            //"Upper rode Up 1st", Start. No upper rode drives active. 
                                                            // This movement is being carried out bei the bottom rode
      case 31: // if ( mProc[15].Active ) {                    //"Upper rode Up 1st", Stop
                 // PORTC &= ~( 1 << (37 - BRode_work));       //Stop "Bottom rode Up"  
                 //mProc[17].Active = false;
                 mProc[15].Active = false;   //}       
               break;    
      case 32: PORTC |= ( 1 << (37 - URode_back));         //Start "Upper rode Up 2nd"
               // Serial.print("Start Upper rode Up 2nd at   "); Serial.println( tPoint );  
                 mProc[16].Active = true;     
               //Serial.print("Upper rode Up 2nd, Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.print ( millis() - CycleStart);
               //Serial.print("  "); Serial.print ( mProc[16].Active); Serial.print("  "); Serial.println ( PORTB, BIN);
 
               break;                                                      
      case 33: if ( mProc[16].Active ) { PORTC &= ~( 1 << (37 - URode_back));         //Stop "Upper rode Up 2nd" 
               // Serial.print("Stop Upper rode Up 2nd at   "); Serial.println( tPoint ); 
                  //Serial.print("Upper rode Up 2nd, Stop1   "); Serial.print( mProc[(eventSequNum[i] - 1)/ 2].End); Serial.print("  "); Serial.println ( PORTB, BIN);
               mProc[16].Active = false; }
                  //Serial.print("Upper rode Up 2nd, Stop   "); Serial.print( mProc[(eventSequNum[i] - 1)/ 2].End); Serial.print("  "); Serial.print ( millis() - CycleStart);
               //Serial.print("  "); Serial.print ( mProc[16].Active); Serial.print("  "); Serial.println ( PORTB, BIN);
               break;       
      case 34: PORTC |= ( 1 << (37 - BRode_work));                                 //Start "Bottom rode Up"  
               // Serial.print("Start Bottom rode Up at   "); Serial.println( tPoint ); 
               mProc[17].Active = true; 
               // Serial.print("Bottom rode Up -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);
               break;   
      case 35: if ( mProc[17].Active ) { PORTC &= ~( 1 << (37 - BRode_work));      //Stop "Bottom rode Up"  
                  // Serial.print("Stop Bottom rode Up at   "); Serial.println( tPoint ); 
                  mProc[17].Active = false;
                  //mProc[15].Active = false;
                  }
               // Serial.print("Bottom rode Up -> Stop   "); Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);
               break;    
      case 36: PORTC |= ( 1 << (37 - BRode_back));                                 //Start "Bottom rode Down"
               mProc[18].Active = true;  
               // Serial.print("Start Bottom rode Down at   "); Serial.println( tPoint ); 
               // Serial.print(PORTC); Serial.print(" run "); Serial.println(BRode_back);
               // Serial.print("Bottom rode Down -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);     
               break;    
      case 37: if ( mProc[18].Active ) { PORTC &= ~( 1 << (37 - BRode_back));                             //Stop "Bottom rode Down"
              //  Serial.print("Stop Bottom rode Down at   "); Serial.println( tPoint ); 
               mProc[18].Active = false; }
               // Serial.print("Bottom rode Down -> Stop    ");  Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);   
               break;   
      case 38: PORTC |= ( 1 << (37 - SideFolders));                                 //Side Folders ON
               mProc[19].Active = true;  
               // Serial.print("Bottom rode Down -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);     
               break;    
      case 39: if ( mProc[19].Active ) { PORTC &= ~( 1 << (37 - SideFolders));     //Side Folders OFF
               mProc[19].Active = false; }
               // Serial.print("Bottom rode Down -> Stop    ");  Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);   
               break;    
      case 40: PORTC |= ( 1 << (37 - FrontFolder));                                 //Front Folder push (ON)
               mProc[20].Active = true;  
               // Serial.print("Bottom rode Down -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);     
               break;    
      case 41: if ( mProc[20].Active ) { PORTC &= ~( 1 << (37 - FrontFolder));      //Front Folder back (OFF)
               mProc[20].Active = false; }
               // Serial.print("Bottom rode Down -> Stop    ");  Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);   
               break;  
      case 42: PORTC |= ( 1 << (37 - PackPusher));                                 //Package Pusher (ON)
               mProc[21].Active = true;  
               // Serial.print("Bottom rode Down -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);     
               break;    
      case 43: if ( mProc[21].Active ) { PORTC &= ~( 1 << (37 - PackPusher));      //Package Pusher back (OFF)
               mProc[21].Active = false; }
               // Serial.print("Bottom rode Down -> Stop    ");  Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);   
               break; 
    }

    
}

void deBugPrintE() {
  delay (100);
   // Serial.print(millis() - CycleStart); Serial.print("  "); Serial.print(mProc[eventSequNum[i]/2].Start); Serial.print("  ");
   // Serial.print(mProc[eventSequNum[i]/2].Name); Serial.print("  "); Serial.println("Start");
}


void deBugPrintO() {
   // Serial.print(millis() - CycleStart); Serial.print("  "); Serial.print(mProc[(eventSequNum[i]-1)/2].End); Serial.print("  ");
   // Serial.print(mProc[(eventSequNum[i]-1)/2].Name); Serial.print("  "); Serial.println("Stop");
}

void MProcTable() {
 
  Serial.println();
  Serial.println("Event\tProcNo. Name[]\t\t\tStLow\tStart\tDur\tEnd\tActive\tEvntNo.\tEventTimeP"); 
  
  for (int i = 0; i < ProcessNumber * 2; i++) {    // To print out results of the sorting
    Serial.print(i); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].procNum); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].Name); Serial.print("\t"); 
    Serial.print(mProc[(int)(i/2)].startLow); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].Start); Serial.print("\t");
    Serial.print(mProc[(int)(i/2)].Duration); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].End); Serial.print("\t"); 
    Serial.print(mProc[(int)(i/2)].Active); Serial.print("\t"); Serial.print(eventSequNum[i]); Serial.print("\t"); Serial.println(eventTimePnt[i]);
  }
}

void SequenceTable() {
    
  Serial.println();
  Serial.println("SeqNum\tEvntNo.\tEventTime"); 
  for (int i = 0; i < ProcessNumber * 2; i++) {    // To print out results of the sorting
    Serial.print(i); Serial.print("\t"); 
    Serial.print(eventSequNum[i]); Serial.print("\t"); 
    Serial.println(eventTimePnt[eventSequNum[i]]);
    }
}

void StartingSequence() {
  Serial.print("Starting Sequence"); Serial.println(" ***************************************** "); 
  delay(500);
  PORTC &= ~( 1 << (37 - FoilCutter)); mProc[13].Active = false;  // Case 27. Foil Cutter Off (release).                      
                       
  PORTA &= ~( 1 << (BeltShifter - 22));  mProc[0].Active = false; // Belt shifter. Come back to the starting position.                       
                     
  PORTA &= ~( 1 << (MainPusher - 22));  mProc[3].Active = false;  // Main Pusher. Come back to the starting pos.                   
             
  BRodePrep();
  
  // CylinderStart ( BRode_back,  sensorD, 18, 450 );                     // Bottom rode down, case 36, 37
  /*CycleStart = millis();         
  PORTC |= ( 1 << (37 - BRode_back));          //case 36:  Start "Bottom rode Down"
            mProc[18].Active = true;  
  while( PINL & ( 1 << (49 - sensorD) ) ){
     Serial.println("Starting procedure. Waiting for the Sensor D (bottom rod down)");
     if (CycleStart < ( millis() - 500 )) break; }    
  PORTC &= ~( 1 << (37 - BRode_back));          // case 37: Stop "Bottom rode Down"
            mProc[18].Active = false; */
            
                             

  CylinderStart ( LblKnife_back,  sensorF, 7, 450 );                   //  Lable Knife back movement.  Case 14, 15   
  /* CycleStart = millis();                             
  PORTA |= ( 1 << (LblKnife_back - 22));        // Case 14. Start Lable Knife back movement.                           
  mProc[7].Active = true;
  while( PINL & ( 1 << (49 - sensorF) ) ){
     Serial.println("Starting procedure. Waiting for the Sensor F (lable knife to the right)");
     if (CycleStart < ( millis() - 1000 )) break; }
  PORTA &= ~( 1 << (LblKnife_back - 22));       // Case 15. Stop Lable Knife back (work) movement.                          
  mProc[7].Active = false;*/
  
  PORTA &= ~( 1 << (FoilGraber - 22)); mProc[12].Active = false;    // Case 25. Foil Graber Off (release).                      
            
                              
  CylinderStart ( FPuller_back,  sensorH, 11, 450 );                     // *** Foil puller back. Case 22, 23                          
  /* CycleStart = millis();
  
  PORTA |= ( 1 << (FPuller_back - 22));        // Case 22. Start Foil puller back (pull) 2nd movement.                          
            mProc[11].Active = true;
  while( PINL & ( 1 << (49 - sensorH) ) ){
     Serial.println("Starting procedure. Waiting for the Sensor H (foil puller back)");
     if (CycleStart < ( millis() - 500 )) break; }
  PORTA &= ~( 1 << (FPuller_back - 22));        // Case 23. Stop Foil puller back (pull) 2nd movement.                          
            mProc[11].Active = false; */
  
  CylinderStart ( URode_back,  sensorC, 16, 450 );  // Upper rode Up. Case 32, 33
  /* CycleStart = millis();         
  PORTC |= ( 1 << (37 - URode_back));         // case 32: Start "Upper rode Up 2nd"
            mProc[16].Active = true;    
  while( (PINL & ( 1 << (49 - sensorC) )) ){
     Serial.println("Starting procedure. Waiting for the Sensor H (foil puller back)");
     if (CycleStart < ( millis() - 500 )) break; }    
  PORTC &= ~( 1 << (37 - URode_back));          //case 33:  Stop "Upper rode Up 2nd" 
            mProc[16].Active = false; */

  delay(10000); 

  CylinderStart ( URode_work,  sensorA, 14, 450 );  // Upper rode down, Case 28. 29
  /* CycleStart = millis();            
  PORTC |= ( 1 << (37 - URode_work));           //case 28: Start "Upper rode down"                                       
     mProc[14].Active = true;
  while( PINL & ( 1 << (49 - sensorA) ) ){
     Serial.println("Starting procedure. Waiting for the Sensor A (lower position of the upper road)"); 
     delay(10);
     if (CycleStart < ( millis() - 500 )) break; }
  PORTC &= ~( 1 << (37 - URode_work));          //case 29: Stop "Upper rode down" 
               mProc[14].Active = false;      */              
  }

  void CylinderStart(uint8_t GPIO_pin, uint8_t sensor, uint8_t proc, uint16_t duration) {

    Serial.print("** Cylinder Start  "); Serial.print(GPIO_pin); Serial.print("  Sensor:  "); Serial.print(sensor); Serial.print(" Procedure:  ");
    
    Serial.print(proc); Serial.print("  Dur:  ");Serial.print(duration); Serial.println(" ** ");

    CycleStart = millis();  
    if( GPIO_pin > 29) { PORTC |= ( 1 << (37 - GPIO_pin)); }  // Start of the movement back
      else {         PORTA |= ( 1 << (GPIO_pin - 22)); }       
    mProc[proc].Active = true;
   
    while( (PINL & ( 1 << (49 - sensor))) ) { // waiting till the appropriate sensor give a signal
       Serial.print("Starting procedure. Waiting for the Sensor "); Serial.println(sensor); 
       delay(5);
       if (CycleStart < ( millis() - duration )) { // or exit the cycle after 450 mS
          Serial.println ("Timeout!");
          break; } 
    }
     if( GPIO_pin > 29) { PORTC &= ~( 1 << (37 - GPIO_pin));}  // Movement stop
        else {            PORTA &= ~( 1 << (GPIO_pin - 22)); }  
     mProc[proc].Active = false;     
   
  }

  void BRodePrep() {
    for(uint8_t i = 0; i < 3; i++) {
      CylinderStart ( BRode_work,  sensorC, 17, 80 );  // Bottom rode Up, case 34, 35
      delay(10);
      CylinderStart ( BRode_back,  sensorD, 18, 450 );  // Bottom rode down, case 36, 37
      // delay(15);
      // CylinderStart ( BRode_work,  sensorB, 17, 20 );  // Bottom rode Up, case 34, 35
      }
    }
