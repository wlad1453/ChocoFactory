
#ifndef Cycle_h
#define Cycle_h

#include <Arduino.h>
#include <SingleProcess.h>

class Cycle {
	
public:
	
	int processNum;

	int cycleDur;
	
	Cycle();
		
	SingleProcess MProcesses[26];
	
	Events MEvents[52];
	
	void sorting(Events *MEvents[52]);
	
	void MProcTable(SingleProcess *MProcesses, Events *MEvents, int processNum);
	
	void MProcTable(SingleProcess MProcesses[26]);
	
	void MProcTable();
	
	void MProcTable(int processNum);
	

};

#endif