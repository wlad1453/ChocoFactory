#include <Cycle.h>

Cycle::Cycle() {  // Object constructor

	char N[10]("abcdefghi");

	//Serial.print("Declaration says:  "); Serial.print(MProcesses[20].Name); Serial.print("  "); Serial.println(MEvents[40].timePoint);

	// SingleProcess 
	/*SPro MProcesses[26] {
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
	25, "Package greifer right",  false, 900,  240, 0,  false 
  }; */
  
	Serial.print("Constructor says:  "); Serial.print(MProcesses[20].Name);
	
	processNum = 26;	// Default value
	cycleDur = 1000;	// Default value
	
	for (int i = 0; i < processNum; i++) {									// Table data processing
	
		MProcesses[i].End = MProcesses[i].Start + MProcesses[i].Duration;	// Stop time calculation
		
		if (MProcesses[i].End > cycleDur) {									// Signal level (Low vs. High level) calculation at the cycle start
		    MProcesses[i].startLow = false;
		    MProcesses[i].End -= cycleDur;
		}
	
	}
	
	//Serial.print("Calculation says:  "); Serial.print(MProcesses[20].Name); Serial.print("  "); Serial.println(MEvents[40].timePoint);
	//Serial.print("  "); Serial.println(MProcesses[20].End);
	  
	for (int i = 0; i < processNum * 2; i++) {								// Trigger points (events time points) calculation.
		
		if (i % 2 != 0) MEvents[i].timePoint = MProcesses[ (i-1)/2 ].End;  	// Odd number events are Stop events
		else            MEvents[i].timePoint = MProcesses[ i/2 ].Start;		// Even number events are Start events
		MEvents[i].sequenceNu = i;                                     		// All the step indexes are in a row, prepared for the later sorting
		MEvents[i].Name[1] =  (char)"a";
		//Serial.print("  "); Serial.println(MEvents[i].Name);
	}
	Serial.print("  "); Serial.println(MEvents[40].timePoint);
}

void sorting(Events *MEvents[52]){
	
	};
	
void Cycle::MProcTable() {};

void Cycle::MProcTable(int PNum){ 
	PNum = 20; 
	Serial.print(PNum);
	};
	
void Cycle::MProcTable(SingleProcess *MProcesses) { };
	
void Cycle::MProcTable(SingleProcess *MProcesses, Events *PEvents, int pNum) {  // or (SingleProcess MProcesses[26]) or (SingleProcess MProcesses[])
 
  Serial.println();
  Serial.println("Event\tProcNo. Name[]\t\t\tStLow\tStart\tDur\tEnd\tActive\tEvntNo.\tEventTimeP"); 
  
  for (int i = 0; i < pNum * 2; i++) {    // To print out results of the sorting
    Serial.print(i); Serial.print("\t"); Serial.print(MProcesses[(int)(i/2)].procNum); Serial.print("\t"); Serial.print(MProcesses[(int)(i/2)].Name); Serial.print("\t"); 
    Serial.print(MProcesses[(int)(i/2)].startLow); Serial.print("\t"); Serial.print(MProcesses[(int)(i/2)].Start); Serial.print("\t");
    Serial.print(MProcesses[(int)(i/2)].Duration); Serial.print("\t"); Serial.print(MProcesses[(int)(i/2)].End); Serial.print("\t"); 
    Serial.print(MProcesses[(int)(i/2)].Active); Serial.print("\t"); 
	Serial.print(PEvents[i].sequenceNu); Serial.print("\t"); Serial.println(PEvents[i].timePoint);
  };
};
	
	