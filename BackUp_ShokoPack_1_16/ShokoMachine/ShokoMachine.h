#ifndef ShokoMachine_h
#define ShokoMachine_h

#define maxUnit 6

#include "Arduino.h"

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


#endif
