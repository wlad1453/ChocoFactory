#ifndef Cyclogramm_h
#define Cyclogramm_h

#include "Arduino.h"
#include <ShokoDefine.h>

//  uint16_t eventTimePnt[52];  // Start and stop time points of various machine processes (event time points)
// uint8_t eventSequNum[52];   // Sequence number of an events. Appearance of the event in the queue within the sycle (trigger points)

struct Cyclogramm {
public:
  int procNum;              // Number of the machineProc process (PROCESS NUMBER)
  char Name[25];            // Name of the PROCESS
  bool startLow;            // Shows if at the cycle start the process isn't active (control signal is LOW)
  int Start;                // Start time (event) in milliSeconds from the cycle begin
  int Duration;             // Duration of the process between two appropriate machineProc events
  int End;                  // End time. If !startLow End event comes earlier then the Start (End time value is less then the Start time)
  bool Active;              // Shows whether the process is going on.
  
};

// Cyclogramm cycl[] {};
// Cyclogramm();

// Cyclogramm::Cyclogramm[26] {};

// Cyclogramm *mP;

Cyclogramm mProc[26] {			// Version 20.03.2020 with full wrapping

//Nu.  Name                   	SLow   Start Dur  End Activ
  0, "Belt shifter ahead",      true,  0,    180, 0,  false,  // was 0, 450, 450 Should be changed because of 5/2 valve. 0, 520,
  1, "Belt shifter Back",       true,  540,  400, 0,  false,  // Became obsolete because of the simple movement
  2, "Belt shifter Up",         true,  510,  400, 0,  false,  // Should be changed due to changes of the process [0] 510, 400 (was 520, 470)
  3, "Main pusher left",        true,  540,   87, 0,  false,  // was 480, 500, 980 Should be changed because of 5/2 valve. 480, 450, (was 480,390)
  4, "Main pusher right",       false, 910,  330, 0,  false,  // Became obsolete because of the simple movement
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
  17, "Bottom rode Up",         true,  50,   290, 0,  false,  // 50,  290\310\280(was ok)
  18, "Bottom rode Down",       true,  350,  190, 0,  false,  // 350\330 (was 0k),  190, 0,
  19, "Side folders",           true,  340,   25, 0,  false,  // Was 320, 88 (20)
  20, "Front folder",           true,  410,   60, 0,  false,  // was 410, 40, \ 410, 78 \ 370, 40 (was ok)
  21, "Package Pusher left",    true,  480,   45, 0,  false,  // was 450,  120, \ 500, 108
  22, "Package Pusher back",    true,  620,  108, 0,  false,  // 620,  108,
  23, "Package greifer Up",     false, 868,  782, 0,  false,  // For the later implementation
  24, "Package greifer left",   true,  660,  228, 0,  false,
  25, "Package greifer right",  false, 900,  240, 0,  false
  };
/*  
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
  */
  
  
// class Process{};
/* 
class CyclCalc{
	public:
	void stopCalc();
	void eventCalc();
	void stepsSorting();
}; */

// Initialization of the machine mProcs array. Each line describes a process (element movement, switching etc.)
// Initialization of all mProcs (elements) with data out of the timing diagramm 'ChocoPackMachine v2.2.xml'
// Later should be exchanged through automatic routine, wich read a flat file or CSV with mProcs timing data or just with #include

#endif
