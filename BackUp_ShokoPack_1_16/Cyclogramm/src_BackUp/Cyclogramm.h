#define Cyclogramm_h



struct Cyclogramm {
  int procNum;              // Number of the machineProc process (PROCESS NUMBER)
  char Name[25];            // Name of the PROCESS
  bool startLow;            // Shows if at the cycle start the process isn't active (control signal is LOW)
  int Start;                // Start time (event) in milliSeconds from the cycle begin
  int Duration;             // Duration of the process between two appropriate machineProc events
  int End;                  // End time. If !startLow End event comes earlier then the Start (End time value is less then the Start time)
  bool Active;              // Shows whether the process is going on.

};

// Initialization of the machine mProcs array. Each line describes a process (element movement, switching etc.)
// Initialization of all mProcs (elements) with data out of the timing diagramm 'ChocoPackMachine v2.2.xml'
// Later should be exchanged through automatic routine, wich read a flat file or CSV with mProcs timing data or just with #include

