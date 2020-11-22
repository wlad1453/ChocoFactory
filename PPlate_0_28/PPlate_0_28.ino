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
  0. LCD display and keyboard!!!
  1. Pressure sensor implementation - > Alarm signal
  2. Launching procedure !!! --> "Ready to Work" signal (To clear the possibility of an automated pressure air valve)
  3. Shut-down procedure (should be done as an interrupt routine)
  4. Breakdown stop

  Version 21.02.20
*/

#include <avr/io.h>
// #include <Arduino.h>
// #include <Wire.h>           // Arduino IDE
// #include <PCylinder.h>
// #include <Cyclogramm.h>

#define cycleLength 1000    // Machine cycle length in milliSeconds
#define ProcNumber 26       // Number of processes to be done during the sycle
// #define ElementsNum 14      // Number of elements (units, components) the packing machine consists of
#define VelocityFactor 1    // "2" would mean that the velocity is devided by factor 2

#define sensorA 49              // Hall effect sensor at the fore end of the upper cylinder
#define sensorB 48              // Hall effect sensor in the middle of the U.cylinder
#define sensorC 47              // Hall effect sensor at the back end of the U.cylinder (working chamber)
#define sensorD 46              // Hall effect sensor at the back end of the bottom cylinder (working chamber)
#define sensorE 45              // HES Label Knife front end
#define sensorF 44              // HES Label Knife back end
#define sensorG 43              // HES Foil puller front end
#define sensorH 42              // HES Foil puller back end 

#define BRode_work 37           // Bottom rode working chamber. 5/3 NC valve with 2 solenoids
#define BRode_back 36           // Bottom rode back stroke chamber. 5/3 NC valve with 2 solenoids
#define URode_work 35           // Upper rode working chamber. 5/3 NO valve
#define URode_back 34           // Upper rode back stroke chamber. 5/3 NO valve 

#define BeltShifter 22          // Belt shifter. Shift the piece of chock. from the belt. 5/2 valve
#define MainPusher 23           // Main Pusher. Pushes the peece of chock. onto the table. 5/2 valve

#define LblKnife_work 24        // Label Knife to the left (work) movement. 5/3 NC valve 
#define LblKnife_back 25        // Label Knife to the right (back) movement. 5/3 NC valve 
#define FPuller_work 26         // Foil puller ahead (work) movement. 5/3 NC valve 
#define FPuller_back 27         // Foil puller back (pull the foil back) 1st and 2nd movement. 5/3 NC valve
#define FoilGraber 28           // Foil Graber On/Off. 3/2 valve
#define BeltShifterSol 29       // Belt Shifter Solenoid
#define FoilCutter 30           // Foil Cutter On/Off. 3/2 valve

#define SideFolders 33
#define FrontFolder 32
#define PackPusher 31

uint8_t btw;                // A variable to store the index in the bubble sorting procedure
boolean change;
uint16_t eventTimePnt[52];  // Start and stop time points of various machine processes (event time points)
uint8_t eventSequNum[52];   // Sequence number of events. Appearance of the event within the sycle (trigger points)
unsigned long CycleStart;   // Cycle start time
uint8_t slowFac (1);        // Slowdown factor
uint16_t Counter (0);       // Cycle counter


boolean feed(0), elevator(1), knife(0), foil(0), cutter(0), wrapper(1);


struct machineProc {
  int procNum;              // Number of the machineProc process (PROCESS NUMBER)
  char Name[25];            // Name of the PROCESS
  boolean startLow;         // Shows if at the cycle start the process isn't active (control signal is LOW)
  int Start;                // Start time (event) in milliSeconds from the cycle begin
  int Duration;             // Duration of the process between two appropriate machineProc events
  int End;                  // End time. If !startLow End event comes earlier then the Start (End time value is less then the Start time)
  boolean Active;           // Shows whether the process is going on.
};

// Initialization of the machine mProcs array. Each line describes a process (element movement, switching etc.)
// Initialization of all mProcs (elements) with data out of the timing diagramm 'ChocoPackMachine v2.2.xml'
// Later should be exchanged through automatic routine, wich read a flat file or CSV with mProcs timing data or just with #include

machineProc mProc[ProcNumber] {
  //Nu.  Name                   SLow   Start Dur  End Active
  0, "Belt shifter ahead",      true,  0,    180, 0,  false,  // was 0, 450, 450 Should be changed because of 5/2 valve. 0, 520,
  1, "Belt shifter Back",       true,  540,  450, 0,  false,  // Became obsolete because of the simple movement
  2, "Belt shifter Up",         true,  510,  400, 0,  false,  // Should be changed due to changes of the process [0] 510, 400 (was 520, 470)
  3, "Main pusher left",        true,  540,  87, 0,  false,  // was 480, 500, 980 Should be changed because of 5/2 valve. 480, 450, (was 480,390)
  4, "Main pusher right",       false, 990,  330, 0,  false,  // Became obsolete because of the simple movement
  5, "Label knife left",        true,  540,  430, 0,  false,
  6, "Glue thrower",            true,  900,    8, 0,  false,  // NOT done. Lack of information - signal level, design
  7, "Label knife right",       true,  0,    430, 0,  false,
  8, "Vacum valve",             false, 980,  470, 0,  false,  // NOT done. Lack of information - signal level, design
  9, "Foil puller ahead",       true,  0,    450, 0,  false,
  10, "Foil puller 1st move",   true,  480,  228, 0,  false,
  11, "Foil puller 2nd move",   true,  760,  100, 0,  false,
  12, "Foil grabber",           true,  450,  410, 0,  false,  // OK
  13, "Foil cutter",            true,  700,  152, 0,  false,  // OK. Cutt/release action. Dur was 52
  14, "Upper rode down",        true,  720,  270, 0,  false,
  15, "Upper rode Up 1st",      true,  50,   230, 0,  false,
  16, "Upper rode Up 2nd",      true,  440,  110, 0,  false,  // 490,   90,
  17, "Bottom rode Up",         true,  50,   290, 0,  false,  // 50,  290\310\280\320(was ok)
  18, "Bottom rode Down",       true,  350,  190, 0,  false,  // 350\330\370 (was 0k),  190, 0,
  19, "Side folders",           true,  340,   25, 0,  false,  // Was 320, 88 (20), 30
  20, "Front folder",           true,  410,   60, 0,  false,  // was 410, 40, \ 410, 78 \ 370, 40 (was ok)
  21, "Package Pusher left",    true,  480,   45, 0,  false,  // was 450,  120, \ 500, 108
  22, "Package Pusher back",    true,  620,  108, 0,  false,  // 620,  108,
  23, "Package greifer Up",     false, 868,  782, 0,  false,  // For the later implementation
  24, "Package greifer left",   true,  660,  228, 0,  false,
  25, "Package greifer right",  false, 900,  240, 0,  false
};


void setup() {

  
  DDRA = 0xFF;                  // Port A is set as Output PA0 -> pin 22 ... PA7 -> pin 29
  DDRC = 0xFF;                  // Port C is set as Output PC0 -> pin 37 ... PC7 -> pin 30
  DDRL = 0x00;                  // Port L is set as Input  PL0 -> pin 49 ... PL7 -> pin 42

  PORTA = 0x00;
  PORTB = 0x00;
  // PORTL = 0xFF;              // in this case it switches on pull up resistors

  Serial.begin(115200);           // Serial communication with PC

  // Stop time calculation (mProc[n].End) and signal level setting at the sycle start 
  
  for (int i = 0; i < ProcNumber; i++) {
    mProc[i].End = mProc[i].Start + mProc[i].Duration;
    if (mProc[i].End > cycleLength) {
      mProc[i].startLow = false;
      mProc[i].End -= cycleLength;
    }
    
    /*Serial.print(i); Serial.print("  "); Serial.print(mProc[i].Name); Serial.print("  "); Serial.print(mProc[i].startLow); Serial.print("  ");
    Serial.print(mProc[i].Start); Serial.print("  "); Serial.print(mProc[i].Duration); Serial.print("  ");
    Serial.println(mProc[i].End); */
  }

  // Launching procedure !!! --> "Ready to Work" signal
  // Shut-down procedure (should be done as an interrupt routine)
  // Breakdown stop
  

  // Trigger points (events time points) calculation. The Start events have even number and the Stop events get odd ones
  
  for (int i = 0; i < ProcNumber * 2; i++) {
    if (i % 2 != 0) eventTimePnt[i] = mProc[ (i-1)/2 ].End;  // Refer to the mProc table
    else eventTimePnt[i] = mProc[i/2].Start;
    eventSequNum[i] = i;                                     // All the step indexes are in a row, prepared for the later sorting
    // Serial.print(i); Serial.print("  "); Serial.print(eventSequNum[i]); Serial.print("  "); Serial.println(eventTimePnt[i]);
  }
  
  // **********  Bubble sorting  ****************
  // Machine steps sorting. Building of a mapping table where all the steps are timely in a row
  
  do {
    change = false;
    for (int i = 0; i < (ProcNumber * 2 - 1); i++) {    // 0..51 - 52 events. (PrNu * 2 - 1) = 51 - the last Nu in the sequence

      //Serial.print(i); Serial.print("  "); Serial.print(eventSequNum[i]); Serial.print("  "); Serial.print(eventSequNum[i+1]); Serial.print("  ");
      //Serial.print(eventTimePnt[eventSequNum[i]]); Serial.print("  "); Serial.println(eventTimePnt[eventSequNum[i+1]]);

      if ( eventTimePnt[eventSequNum[i]] > eventTimePnt[eventSequNum[i + 1]]) {
        btw = eventSequNum[i];
        eventSequNum[i] = eventSequNum[i + 1];
        eventSequNum[i + 1] = btw;
        change = true;
        // Serial.print(i); Serial.print("  "); Serial.print(eventSequNum[i]); Serial.print("  "); Serial.println(eventSequNum[i+1]);
      }
    }
  } while (change == true);

  MProcTable();
  SequenceTable();
  StartingSequence();
 
} // End Setup

  
void loop() {
  // To do:
  // Keypad polling module (or interrupt routine)
  // In case a key pressed changing the default values (could be done via interrapts)
  // Main working sequence for(int i = 0; i < sizeOf(eventSequNum()); i++)
  // If some mProcs happen -> FullStop (Stop switch pressed, no chokolate with greifer running-out, not allowed situation etc.)

  CycleStart = millis();   // Starting point of each cycles
  Counter++; Serial.print("Cycle number:  "); Serial.println(Counter); // Number of the cycle from the begin
  

  // Sycle control logic
  
  for (int i = 0; i < ProcNumber * 2; ) {           // Call all events in the cycle sequentually.
    // EorO = eventSequNum[i] % 2;                     // Check whether the event number is Even or Odd
    
    // Serial.print(i); Serial.print("  "); Serial.println(eventSequNum[i]);
    
    if (eventSequNum[i] % 2 == 0) {                 // In case of an EVEN number it is a Process Start event   

      if ( (millis() - CycleStart) >= mProc[eventSequNum[i] / 2].Start * slowFac){  //If the Start point is reached check for the action
        // if ( !(mProc[eventSequNum[i] / 2].Active) ) action(i);                   // make an action (switch ON) in case the process is stopped
        action(i);
        i++;                                        // Increments each time the current event is reached. (next time check the next event) 
      }      
      //deBugPrintE();
    }
    else {                                          // In case of ODD numbers it is a Process End event   
      
      if ( (millis() - CycleStart) >= mProc[(eventSequNum[i] - 1) / 2].End * slowFac ){  //If the Stop point is reached check for the action
        // if ( mProc[(eventSequNum[i] - 1) / 2].Active ) action(i);   // make an action (switch OFF) in case the process is going on only
        action(i);
        i++;
      }
      
      //deBugPrintO();
    }
    
//Serial.print(digitalRead(sensorC)); Serial.print(digitalRead(sensorB)); Serial.print(digitalRead(sensorA)); 
//Serial.print(" PINA: "); Serial.print(PINA, BIN); Serial.print(" PINC: "); Serial.println(PINC, BIN);

    // *******************    Sensor reading section. Made once each polling   *************************************
    
    if ( mProc[14].Active && !(PINL & ( 1 << (49 - sensorA) ) )) {    // SensorA reading, Upper rod down. Stop the movement. All connectors are open
      PORTC &= ~( 1 << (37 - URode_work));
      mProc[14].Active = false;
      // Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorA - > Upper Rode down Stop"));
      Serial.print("Sensor A at  "); Serial.println( millis() - CycleStart);
      }      

    if ( mProc[17].Active && !( PINL & ( 1 << (49 - sensorB)) )) {    // Stops the movement Bottom rode Up 1st. 
      PORTC &= ~( 1 << (37 - BRode_work));                            // "Bottom rode Up" and "Upper rode Up 1st" with a piece of chocolate squeezed inbetween 
      mProc[17].Active = false;
      // Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorB - > Bottom rode Up & Upper Rode Up 1st -> Stop"));
      Serial.print("Sensor B at  "); Serial.println( millis() - CycleStart);
      }
      
    if ( mProc[16].Active && !( PINL & ( 1 << (49 - sensorC) ) )) {   // Upper rode Up 2nd. Stop the movement of bottom rode. All connections are open 
      Serial.print (digitalRead(URode_back)); Serial.print("  "); Serial.print (PINL); Serial.print("  "); Serial.println (mProc[16].Active);
      PORTC &= ~( 1 << (37 - URode_back));                            // Stop "Upper rode Up 2nd"
      mProc[16].Active = false;
      /* Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorC - > Upper Rode Up 2nd -> Stop"));
      Serial.print (digitalRead(URode_back)); Serial.print("  "); Serial.print (PINL);  Serial.print("  "); Serial.println (mProc[16].Active); */
      Serial.print("Sensor C at   "); Serial.println( millis() - CycleStart);
      }  

    if ( mProc[18].Active && !( PINL & ( 1 << (49 - sensorD))) ) {  // Stops the movement Bottom rode down at the lower end of cylinder 
      Serial.println (digitalRead(BRode_back));
      PORTC &= ~( 1 << (37 - BRode_back));                            //"Bottom rode down", Stop
      mProc[18].Active = false; 
      // action(37);
      /* Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.print (F("  sensorD - > Bottom Rode down -> Stop  ")); 
      Serial.println (digitalRead(BRode_back)); */
      Serial.print("Sensor D at   "); Serial.println( millis() - CycleStart);
      }  

    //*** New sensors conditions 27.11.19 
    
    if ( mProc[5].Active && !(PINL & ( 1 << (49 - sensorE) ) )) {    // SensorE reading, Lable Knife ahead. Stop the movement. 5/3 NC
      PORTA &= ~( 1 << (LblKnife_work - 22));
      mProc[5].Active = false;
      Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorE - > Lable Knife ahead -> Stop"));
      }      

    if ( mProc[7].Active && !( PINL & ( 1 << (49 - sensorF)) )) {    // SensorE reading, Lable Knife back. Stop the movement. 5/3 NC
      PORTA &= ~( 1 << (LblKnife_back - 22));                       
      mProc[7].Active = false;
      Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorF - > Lable Knife back -> Stop"));
      }
      
    if ( mProc[9].Active && !(PINL & ( 1 << (49 - sensorG) ) )) {    // SensorG reading, Foil Grabber ahead. Stop the movement. 5/3 NC
      PORTA &= ~( 1 << (FPuller_work - 22));
      mProc[9].Active = false;
      Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorG - > Foil Grabber ahead -> Stop"));
      }      

    if ( mProc[10].Active && !( PINL & ( 1 << (49 - sensorH)) )) {    // SensorH reading, Foil Grabber back 1st move. Stop the movement. 5/3 NC
      PORTA &= ~( 1 << (FPuller_back - 22));                     
      mProc[10].Active = false;
      Serial.print(PORTL); Serial.print("  "); Serial.print(PINL); Serial.println (F("  sensorH - > Foil Grabber back 1st move -> Stop"));
      }
  /* New sensors 27.11*/
  
   } // End  (i = 0; i < ProcNumber * 2; i++) 
   
   // delay(10000);    // Delay between cycles  
}    // End void loop()

void action(int i) {
  byte j;
  j = eventSequNum[i];

   // Serial.print(F("Action:  ")); Serial.print(i); Serial.print("  "); Serial.println(eventSequNum[i]); 

   if( (feed && (j <= 9)) 
        || ( knife && (j >= 10) && (j <= 15)) 
        || ( foil && (j >= 18) && (j <= 25)) 
        || ( cutter && ( (j == 26) || (j == 27) )) 
        || ( elevator && (j >= 28) && (j <= 37))
        || ( wrapper && (j >= 38) && (j <= 43)) ) 
     {Serial.print(feed + 2*knife + 4*foil + 8*cutter + 16*elevator + 32*wrapper); Serial.print("  "); Serial.print(i); Serial.print("  "); Serial.println(j);}
   else return;
  
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
               Serial.print("Start Upper rode down at   "); Serial.println( millis() - CycleStart);                                    
               mProc[14].Active = true;
               break;    
      case 29: PORTC &= ~( 1 << (37 - URode_work));         //Stop "Upper rode down" 
               Serial.print("Stop Upper rode down at   "); Serial.println( millis() - CycleStart);  
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
               Serial.print("Start Upper rode Up 2nd at   "); Serial.println( millis() - CycleStart);  
                 mProc[16].Active = true;     
               //Serial.print("Upper rode Up 2nd, Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.print ( millis() - CycleStart);
               //Serial.print("  "); Serial.print ( mProc[16].Active); Serial.print("  "); Serial.println ( PORTB, BIN);
 
               break;                                                      
      case 33: if ( mProc[16].Active ) { PORTC &= ~( 1 << (37 - URode_back));         //Stop "Upper rode Up 2nd" 
               Serial.print("Stop Upper rode Up 2nd at   "); Serial.println( millis() - CycleStart); 
                  //Serial.print("Upper rode Up 2nd, Stop1   "); Serial.print( mProc[(eventSequNum[i] - 1)/ 2].End); Serial.print("  "); Serial.println ( PORTB, BIN);
               mProc[16].Active = false; }
                  //Serial.print("Upper rode Up 2nd, Stop   "); Serial.print( mProc[(eventSequNum[i] - 1)/ 2].End); Serial.print("  "); Serial.print ( millis() - CycleStart);
               //Serial.print("  "); Serial.print ( mProc[16].Active); Serial.print("  "); Serial.println ( PORTB, BIN);
               break;       
      case 34: PORTC |= ( 1 << (37 - BRode_work));                                 //Start "Bottom rode Up"  
               Serial.print("Start Bottom rode Up at   "); Serial.println( millis() - CycleStart); 
               mProc[17].Active = true; 
               // Serial.print("Bottom rode Up -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);
               break;   
      case 35: if ( mProc[17].Active ) { PORTC &= ~( 1 << (37 - BRode_work));      //Stop "Bottom rode Up"  
                  Serial.print("Stop Bottom rode Up at   "); Serial.println( millis() - CycleStart); 
                  mProc[17].Active = false;
                  //mProc[15].Active = false;
                  }
               // Serial.print("Bottom rode Up -> Stop   "); Serial.print( mProc[eventSequNum[i] / 2].End); Serial.print("  "); Serial.println ( millis() - CycleStart);
               break;    
      case 36: PORTC |= ( 1 << (37 - BRode_back));                                 //Start "Bottom rode Down"
               mProc[18].Active = true;  
               Serial.print("Start Bottom rode Down at   "); Serial.println( millis() - CycleStart); 
               // Serial.print(PORTC); Serial.print(" run "); Serial.println(BRode_back);
               // Serial.print("Bottom rode Down -> Start   "); Serial.print( mProc[eventSequNum[i] / 2].Start); Serial.print("  "); Serial.println ( millis() - CycleStart);     
               break;    
      case 37: if ( mProc[18].Active ) { PORTC &= ~( 1 << (37 - BRode_back));                             //Stop "Bottom rode Down"
               Serial.print("Stop Bottom rode Down at   "); Serial.println( millis() - CycleStart); 
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
  
  for (int i = 0; i < ProcNumber * 2; i++) {    // To print out results of the sorting
    Serial.print(i); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].procNum); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].Name); Serial.print("\t"); 
    Serial.print(mProc[(int)(i/2)].startLow); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].Start); Serial.print("\t");
    Serial.print(mProc[(int)(i/2)].Duration); Serial.print("\t"); Serial.print(mProc[(int)(i/2)].End); Serial.print("\t"); 
    Serial.print(mProc[(int)(i/2)].Active); Serial.print("\t"); Serial.print(eventSequNum[i]); Serial.print("\t"); Serial.println(eventTimePnt[i]);
  }
}

void SequenceTable() {
    
  Serial.println();
  Serial.println("SeqNum\tEvntNo.\tEventTime"); 
  for (int i = 0; i < ProcNumber * 2; i++) {    // To print out results of the sorting
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

  
