#include <xc.h>


extern unsigned char return_time ; /// 5 
extern unsigned char secs;

void __interrupt() isr(void)
{
    static unsigned int count = 0;
    
    if (TMR2IF == 1) // when ov occurs timer2 int is requested by setting int flag 
    {
        if (++count == 20000)
        {
            count = 0;
            
            if(return_time > 0)
                return_time--; // 5 4 3 2 1 0
            
            if(secs >0)
                secs--;
            
            
        }
        
        TMR2IF = 0; // clear the flag 
    }
}