#include <xc.h>
#include "digital_keypad.h"

void init_digital_keypad(void)
{
    /* Set Keypad Port as input */  //TRISB = TRISB7 6 5 4 3 2 1 0 //RB0 to RB5 as input 
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES; //TRISB = TRISB | 0x1F -> 
}

unsigned char read_digital_keypad()
{
    static char once = 0 ;
	static int longpressed = 1 ;
	static unsigned char pre_key ;
	unsigned char key =  KEYPAD_PORT & INPUT_LINES ;
	if ( key != ALL_RELEASED && !once )
	{
		once = 1;
		longpressed = 0 ;
		pre_key = key;
	}
	else if( key == ALL_RELEASED && once )
	{
		once = 0;
		if ( longpressed < 50 )
		return pre_key ;  // SW1 SW2 SW3
	}
	else if (once && longpressed <= 50 )
		longpressed++;
	else if ( once && longpressed == 51  && key == SW4)
	{ 
		longpressed ++;
		return  LPSW4;
	}
    else if ( once && longpressed == 51  && key == SW5)
	{ 
		longpressed++;
		return  LPSW5;
	}
	return ALL_RELEASED;

   
}
