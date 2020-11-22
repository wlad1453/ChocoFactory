#ifndef ShokoDefine_h
#define ShokoDefine_h

#define cycleLength 1000    	// Machine cycle length in milliSeconds
#define ProcessNumber 26     	// Number of processes to be done during the sycle
// #define ElementsNum 14      	// Number of elements (units, components) the packing machine consists of
// #define VelocityFactor 1    	// "2" would mean that the velocity is devided by factor 2

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

#define SideFolders 33		// Side Folders with a return spring On/Off. 3/2 valve
#define FrontFolder 32		// Front Folder with a return spring On/Off. 3/2 valve
#define PackPusher 31		// Package Pusher with a return spring On/Off. 3/2 valve

#endif

