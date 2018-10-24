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
*/
#include "Arduino.h"
#define cycleLength 1000  // Machine cycle length in milliSeconds
#define ElementsNumber 25 // Number of elements (units, components) the packing machine consists of
  
  uint8_t MStep = 0;
  uint8_t btw;            // A variable to store the index in the bubble sorting procedure
  boolean change;
  uint16_t triggerTime[50]; // Start and stop time points of various machine funktions
  uint8_t eventIndex[50]; // Sequence number of trigger points
  unsigned long TimeStamp;


struct machineState {
  int secNum;      // Order number of the event
  char Name[25];
  boolean startLow; // Shows if at the cycle start the process isn't active
  int Start;  // Start time in milliSeconds from the cycle begin 
  int Duration;    // Duration of the process between two events
  int End;  // End time. If !startLow End time comes earlier then Start time
};

 machineState event[ElementsNumber] {  
 //Nu.  Name                  SLow Start Dur End
    0, "Belt shifter ahead",  true, 0,   460, 0,
    1, "Belt shifter Back",   true, 540, 460, 0,
    2, "Belt shifter Up",     true, 520, 480, 0,
    3, "Main pusher left",    true, 480, 410, 0,
    4, "Main pusher right",   true, 990, 340, 0,
    5, "Label knife left",    true, 540, 440, 0,
    6, "Glue thrower",        true, 900, 11,  0,
    7, "Label knife right",   true, 0,   440, 0,
    8, "Vacum valve",         true, 980, 480, 0,
    9, "Foil puller ahead",   true, 0,   460, 0,
    10,"Foil puller 1st move",true, 500, 236, 0,
    11,"Foil puller 2nd move",true, 810, 160, 0,
    12,"Foil greifer",        true, 470, 510, 0,
    13,"Foil cutter",         true, 730, 80,  0,
    14,"Upper rode down",     true, 570, 460, 0,
    15,"Upper rode Up",       true, 50,  240, 0,
    16,"Bottom rode Up",      true, 50,  240, 0,
    17,"Bottom rode Down",    true, 290, 220, 0,
    18,"Side folders",        true, 350, 96,  0,
    19,"Front folder",        true, 430, 86,  0,
    20,"Package Pusher left", true, 520, 236, 0,
    21,"Package Pusher back", true, 780, 236, 0,
    22,"Package greifer Up",  true, 878, 730, 0,
    23,"Package greifer left",true, 520, 236, 0,
    24,"Package greifer right",true, 790, 250, 0
  };

void setup() {
  //uint16_t TimeStamp;
  
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);           // Used to type in characters and for setUp

  // randomSeed(analogRead(0));

  // Initialization of the machine events array
  // Initialiyation of all events (elements) with data from timing diagramm ChocoPackMachine v2.2.xml
  // Later should be exchanged through automatic routine, wich read a flat file or CSV with events timing data  

  // Stop time (event[n].End) and start level calculation
  for (int i = 0; i < ElementsNumber; i++) {    
    event[i].End = event[i].Start + event[i].Duration;
    if (event[i].End > cycleLength) {
      event[i].startLow = false;
      event[i].End -= cycleLength;
    }
    // else event[i].startLow = true;
    Serial.print(i); Serial.print("  "); Serial.print(event[i].Name); Serial.print("  "); Serial.print(event[i].startLow); Serial.print("  "); Serial.print(event[i].Start); Serial.print("  "); Serial.print(event[i].Duration); Serial.print("  "); 
    Serial.println(event[i].End);   
  }

// Launching procedure !!! --> "Ready to Work" signal
// Shut-down procedure (should be done as an interrupt routine)
// Breakdown stop

  // Trigger points calculation, e.g. even elements get Start trigger points and odd once End points
  TimeStamp = micros();
  for (int i = 0; i < ElementsNumber*2; i++) {
    if (i%2 != 0) triggerTime[i] = event[(i-1)/2].End;
    else triggerTime[i] = event[i/2].Start;
    eventIndex[i] = i;
    // Serial.print(i); Serial.print("  "); Serial.print(eventIndex[i]); Serial.print("  "); Serial.println(triggerTime[i]);
  }
  Serial.print("Trigger point calculation done for "); Serial.print(micros() - TimeStamp);  Serial.println(" /uS");
  
  // Events sorting. Building of a mapping table where all events are timely in a row
  TimeStamp = micros();
  do {
  change = false;
  for (int i = 0; i < 49; i++) {
   
   // Serial.print(i); Serial.print("  "); Serial.print(eventIndex[i]); Serial.print("  "); Serial.print(eventIndex[i+1]); Serial.print("  "); 
   // Serial.print(triggerTime[eventIndex[i]]); Serial.print("  "); Serial.println(triggerTime[eventIndex[i+1]]);
   
    if ( triggerTime[eventIndex[i]] > triggerTime[eventIndex[i+1]]) {
      btw = eventIndex[i];
      eventIndex[i] = eventIndex[i+1];
      eventIndex[i+1] = btw;
      change = true;
      // Serial.print(i); Serial.print("  "); Serial.print(eventIndex[i]); Serial.print("  "); Serial.println(eventIndex[i+1]);
      }
    }
  } while (change == true);
  Serial.print("Events sorting done for "); Serial.print(micros() - TimeStamp);  Serial.println(" /uS");

   for (int i = 0; i < ElementsNumber*2; i++) {
   
   Serial.print(i); Serial.print("  "); Serial.print(eventIndex[i]); Serial.print("  "); Serial.println(triggerTime[eventIndex[i]]);
   }
}


void loop() {

// Keypad polling module (or interrart routine)
// In case a key pressed changing the default values (could be done via interrapts)
// Main working sequence for(int i = 0; i < sizeOf(eventIndex()); i++)
// If some events happen -> FullStop (Stop switch pressed, no chokolate with greifer running-out, not allowed situation etc.)

  TimeStamp = millis();

  for (int i = 0; i < ElementsNumber * 2; i++) {
    
  if (eventIndex[i]%2 == 0) {
    while ((millis() - TimeStamp) < event[eventIndex[i]/2].Start); //asm ("nop"); // delay(1);
    Serial.print(millis() - TimeStamp); Serial.print("  "); Serial.print(event[eventIndex[i]/2].Start); Serial.print("  "); 
    Serial.print(event[eventIndex[i]/2].Name); Serial.print("  "); Serial.println("Start");
     
    }  
  else {
    while ((millis() - TimeStamp) < event[(eventIndex[i]-1)/2].End); //asm ("nop"); // delay(1);
    Serial.print(millis() - TimeStamp); Serial.print("  "); Serial.print(event[(eventIndex[i]-1)/2].End); Serial.print("  "); 
    Serial.print(event[(eventIndex[i]-1)/2].Name); Serial.print("  "); Serial.println("Stop");
    }
  
  }
}
