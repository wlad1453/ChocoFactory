#ifndef Cyclogramm_h
#define Cyclogramm_h


struct Cycl : public Cyclogramm {
public:
  int procNum;              // Number of the machineProc process (PROCESS NUMBER)
  char Name[25];            // Name of the PROCESS
  bool startLow;            // Shows if at the cycle start the process isn't active (control signal is LOW)
  int Start;                // Start time (event) in milliSeconds from the cycle begin
  int Duration;             // Duration of the process between two appropriate machineProc events
  int End;                  // End time. If !startLow End event comes earlier then the Start (End time value is less then the Start time)
  bool Active;              // Shows whether the process is going on.
  
  // Cyclogramm();

};

Cyclogramm mProc[26] {			// Cyclogramm mProc[26] {
	  
	//Nu.  Name                   SLow   Start Dur  End Activ
	0, "Belt shifter ahead",      true,  0,    450, 0,  false,
	1, "Belt shifter Back",       true,  540,  450, 0,  false,
	2, "Belt shifter Up",         true,  520,  470, 0,  false,
	3, "Main pusher left",        true,  480,  390, 0,  false, 
	4, "Main pusher right",       false, 990,  330, 0,  false,
  5, "Label knife left",        true,  540,  430, 0,  false,
  6, "Glue thrower",            true,  900,    8, 0,  false,
  7, "Label knife right",       true,  0,    430, 0,  false,
  8, "Vacum valve",             false, 980,  470, 0,  false,
  9, "Foil puller ahead",       true,  0,    450, 0,  false,
  10, "Foil puller 1st move",   true,  480,  228, 0,  false,
  11, "Foil puller 2nd move",   true,  760,  100, 0,  false,
  12, "Foil grabber",           true,  450,  410, 0,  false,
  13, "Foil cutter",            true,  700,   52, 0,  false,
  14, "Upper rode down",        true,  720,  270, 0,  false,
  15, "Upper rode Up 1st",      true,  50,   230, 0,  false,
  16, "Upper rode Up 2nd",      true,  490,   90, 0,  false,
  17, "Bottom rode Up",         true,  50,   230, 0,  false,
  18, "Bottom rode Down",       true,  290,  210, 0,  false,
  19, "Side folders",           true,  320,   88, 0,  false,
  20, "Front folder",           true,  410,   78, 0,  false,
  21, "Package Pusher left",    true,  500,  108, 0,  false,
  22, "Package Pusher back",    true,  620,  108, 0,  false,
  23, "Package greifer Up",     false, 868,  782, 0,  false,
  24, "Package greifer left",   true,  660,  228, 0,  false,
  25, "Package greifer right",  false, 900,  240, 0,  false, 
  };
  
  

// Initialization of the machine mProcs array. Each line describes a process (element movement, switching etc.)
// Initialization of all mProcs (elements) with data out of the timing diagramm 'ChocoPackMachine v2.2.xml'
// Later should be exchanged through automatic routine, wich read a flat file or CSV with mProcs timing data or just with #include

#endif
