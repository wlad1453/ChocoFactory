#ifndef ControlBox_h
#define ControlBox_h

void greeting () {
  lcd.clear();
  lcd.home ();           lcd.print(" *** ShockoPack *** ");
  lcd.setCursor( 4, 2);  lcd.print("F4  for menu");
  lcd.setCursor( 4, 3);  lcd.print("Start to run");
  delay(20);
}

void mainMenu() {
  lcd.clear(); 
  lcd.setCursor ( 0, 0);  lcd.print( "1.Operating mode" );
  lcd.setCursor ( 0, 1);  lcd.print( "2.Units On/Off" );
  lcd.setCursor ( 0, 2);  lcd.print( "3.Start options" );
  lcd.setCursor ( 0, 3);  lcd.print( "4.Timing factors" );
  lcd.noBlink(); lcd.noCursor();
  
}

void runScreen() {
  lcd.clear(); 
  lcd.setCursor ( 0, 0);  lcd.print( "Cycle    A     E    " );
  lcd.setCursor ( 0, 1);  lcd.print( "         B     F    " );
  lcd.setCursor ( 0, 2);  lcd.print( "Move     C     G    " );
  lcd.setCursor ( 0, 3);  lcd.print( "         D     H    " );
  lcd.noBlink(); lcd.noCursor();
}

void  operatingModeLCD() {
        lcd.clear();
										// "*****-----*****-----"
		lcd.setCursor ( 0, 0);  lcd.print( "1.Single Cycle" );			// Y/N
		lcd.setCursor ( 0, 1);  lcd.print( "2.Stepwise" );				// Y/N
		lcd.setCursor ( 0, 2);  lcd.print( "3.Slowdown" );				// Number
		lcd.setCursor ( 0, 3);  lcd.print( "4.Delay btw cyc" );		// Number
		lcd.setCursor ( 17, 0);  if ( singleCycle == true ) lcd.print( "Y" ); else lcd.print ( "N" );      // Y/N
		lcd.setCursor ( 17, 1);  if ( stepWise == true )    lcd.print( "Y" ); else lcd.print ( "N" );      // Y/N
		lcd.setCursor ( 17, 2);  lcd.print( slowFac );        // Number
		lcd.setCursor ( 17, 3);  lcd.print( delayBTWcycles );   // Number
  
			
		lcd.noBlink(); lcd.noCursor();
        delay(2000);
      }
	  

void startingOptions(){
        lcd.clear();
        lcd.setCursor ( 0, 0);  lcd.print( "1.Start preparation" );		// Y/N
		lcd.setCursor ( 0, 1);  lcd.print( "2.Delayed start" );			// Y/N or Num or 0,5,10,15,20 sec
		lcd.setCursor ( 0, 2);  lcd.print( "3.Shutdown procedure" );	// Y/N
		lcd.setCursor ( 0, 3);  lcd.print( "4.Start!" );				// Go!, Start bytton, '4', 
        delay(2000);
  }

void timingFactors() {
        lcd.clear();
        lcd.setCursor ( 3, 0 ); lcd.print( "Timing factors" );
        lcd.setCursor ( 4, 1 ); lcd.print( "Press a key" );
        delay(2000);
  }

void showUnits( uint8_t offsetU ){     // Shows the units on the LCD screen with offset
  lcd.clear();
  for( int i = offsetU; i < ( 4 + offsetU ) ; i++ ) {
    lcd.setCursor ( 0, i - offsetU ); lcd.print( String(units[i].num) + "." + units[i].unitName );
    lcd.setCursor ( 12, i - offsetU ); if( units[i].active == true ) lcd.print("Y"); else lcd.print("N");    
  }
}

void clearSens() {
	for ( byte i = 0; i < 4; i++ ) {
		if ( i != 0) { lcd.setCursor( 11, i ); lcd.print( "   " ); } // Because sensor A is active just 10 mSec
		if ( i != 0) { lcd.setCursor( 17, i ); lcd.print( "   " ); }
	}
}
  
void activeUnit(uint8_t maxUnt) {      // Switches on/off the units of the machine
  uint8_t unitN(0);                    // Unit number {0...maxUnit}
  uint8_t LCDoffset(0);                // Shift of the LCD window against the whole Units array
  uint8_t LCDrow(0);                   // LCD screen row {0..3 for 20x4}
  
  showUnits(LCDoffset);
  
  lcd.setCursor ( 13, LCDrow ); lcd.cursor(); lcd.blink();   // Fault! should be row!
  
  key = NULL;  
  while ( key != '4' && key != 'D' ) {
    
    key = kpd.getKey();  
    if (key){      
      if ( key == '2' ) {
        if ( LCDrow > 0 ) { LCDrow--; unitN--; } 
        else if ( LCDoffset > 0 ) { unitN--; LCDoffset--; showUnits ( LCDoffset );}    
      }
      if ( key == '8' ) {
        if ( LCDrow < 3 ) { LCDrow++; unitN++; } 
        else if ( LCDoffset < maxUnt - 4 ) { unitN++; LCDoffset++; showUnits ( LCDoffset );}    
      }
      lcd.setCursor ( 13, LCDrow ); lcd.cursor(); lcd.blink();                    // Shift the cursor to the current position
                                                                                  // unitN = LCDrow + LCDoffset
            
      if ( key == '6' || key == '5' ) {                                           // 'Enter' or change the value
        units[unitN].active = !units[unitN].active;                               // Change unit status. On/off particular unit
        lcd.setCursor ( 12, LCDrow );                                             // LCDrow = unitN - LCDoffset 
        if( units[unitN].active == true ) lcd.print("Y"); else lcd.print("N");    // Print unit status
      }
      
      if ( key == '4' || key == 'D' ) {                                           // Exit
        key = NULL;
        return; 
        }
    } // End if ( key )
    delay(20);
        
  } // End while ( key != '4' && key != 'D' )
} // End activeUnit()

void operatingMode() {
    
  operatingModeLCD();
 
  key = NULL;
  while( key != 'D' ) {
    
    key = kpd.getKey();
    if ( key ) {
      switch ( key ) {
        case '1': singleCycle = !singleCycle;
                  lcd.setCursor ( 17, 0);  
                  if ( singleCycle == true ) lcd.print( "Y" ); 
                  else lcd.print ( "N" );
                  break;
        case '2': stepWise = !stepWise;
                  lcd.setCursor ( 17, 1);  
                  if ( stepWise == true ) lcd.print( "Y" ); 
                  else lcd.print ( "N" );   
                  break;
        case '3': slowFac *= 2; 
                  if ( slowFac == 32 ) slowFac = 1;
                  lcd.setCursor ( 17, 2);  lcd.print( "  " ); 
                  lcd.setCursor ( 17, 2);  lcd.print( slowFac );  
                  break;
        case '4': delayBTWcycles += 5;  
                  if ( delayBTWcycles == 35 ) delayBTWcycles = 0;    
                  lcd.setCursor ( 17, 3);  lcd.print( "  " ); 
                  lcd.setCursor ( 17, 3);  lcd.print( delayBTWcycles );  
                  break;
      }
    }
  }  // end while( key != 'D' )
  return;  
} 

void Menu() {
    
  mainMenu();
  
  key = NULL;
  while( key != 'D' ) {
   
    if ( key ) {
      switch ( key ) {
        case '1': operatingMode();      break;
        case '2': activeUnit(maxUnit);  break;
        case '3': startingOptions();    break;
        case '4': timingFactors();      break;
      }
      mainMenu();
      // delay( 500 );
    }
    
    key = NULL; key = kpd.getKey();
    
  }  // end while( key != '*' )
  return;  
} 



  
// class Process{};
/* 
class CyclCalc{
	public:
	void stopCalc();
	void eventCalc();
	void stepsSorting();
}; */

#endif
