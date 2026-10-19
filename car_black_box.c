#include <xc.h>
#include <string.h>

#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "car_black_box.h"
#include "EEprom.h"
#include "uart.h"

unsigned char time[7];
unsigned char log[11];

unsigned char pos = 0;
unsigned char log_count = 0;

unsigned char return_time, secs;
unsigned char saved_pwd[4] = {'1','1','1','1'};
const char *menu[] = {"View log", "Clear log", "Download", "Change pwd"};

/* -------- TIME -------- */
static void get_time(void)
{
    unsigned char h = read_ds1307(HOUR_ADDR);
    unsigned char m = read_ds1307(MIN_ADDR);
    unsigned char s = read_ds1307(SEC_ADDR);

    time[0] = ((h >> 4) & 0x03) + '0';
    time[1] = (h & 0x0F) + '0';
    time[2] = ((m >> 4) & 0x07) + '0';
    time[3] = (m & 0x0F) + '0';
    time[4] = ((s >> 4) & 0x07) + '0';
    time[5] = (s & 0x0F) + '0';
    time[6] = '\0';
}

/* -------- DASHBOARD -------- */
void display_dashboard(unsigned char event[], unsigned char speed)
{
    clcd_print("TIME    EVT  SP", LINE1(0));

    get_time();

    clcd_putch(time[0], LINE2(0));
    clcd_putch(time[1], LINE2(1));
    clcd_putch(':', LINE2(2));
    clcd_putch(time[2], LINE2(3));
    clcd_putch(time[3], LINE2(4));
    clcd_putch(':', LINE2(5));
    clcd_putch(time[4], LINE2(6));
    clcd_putch(time[5], LINE2(7));

    clcd_print((char *)event, LINE2(10));

    clcd_putch(speed / 10 + '0', LINE2(14));
    clcd_putch(speed % 10 + '0', LINE2(15));
}

/* -------- LOG EVENT -------- */
void log_event(unsigned char event[], unsigned char speed)
{
    unsigned char addr;

    if (log_count < 10)
        log_count++;

    if (pos == 10)
        pos = 0;

    addr = pos * 10;

    get_time();

    strncpy((char *)log, (char *)time, 6);
    strncpy((char *)&log[6], (char *)event, 2);

    log[8] = speed / 10 + '0';
    log[9] = speed % 10 + '0';
    log[10] = '\0';

    ext_eeprom_24C02_str_write(addr, (char *)log);

    pos++;
}

/* -------- VIEW LOG -------- */
void view_log(unsigned char key, unsigned char reset_flag)
{
    static unsigned char index = 0;
    unsigned char data[11];
    unsigned char i, addr, start;
    clcd_print("#   TIME   EV SP", LINE1(0));
    if (reset_flag == RESET_VIEW_LOG_POS)
        index = 0;

    if (key == SW5 && index < log_count - 1)
    {
        index++;
        clear_screen();
    }
    else if (key == SW4 && index > 0)
    {
        index--;
        clear_screen();
    }

    if (log_count == 0)
    {
        clcd_print("No Logs", LINE2(0));
        return;
    }

    start = (log_count < 10) ? 0 : pos;

    addr = ((start + index) % 10) * 10;

    for (i = 0; i < 10; i++)
        data[i] = ext_eeprom_24C02_read(addr + i);

    data[10] = '\0';

    clcd_putch(index + '0', LINE2(0));

    clcd_putch(data[0], LINE2(2));
    clcd_putch(data[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(data[2], LINE2(5));
    clcd_putch(data[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(data[4], LINE2(8));
    clcd_putch(data[5], LINE2(9));

    clcd_putch(data[6], LINE2(11));
    clcd_putch(data[7], LINE2(12));

    clcd_putch(data[8], LINE2(14));
    clcd_putch(data[9], LINE2(15));
}

/* -------- CLEAR LOG -------- */
unsigned char clear_log(unsigned char reset_flag)
{
    if (reset_flag == RESET_MENU)
    {
        pos = 0;
        log_count = 0;

        clear_screen();
        clcd_print("Logs Cleared", LINE1(0));
        __delay_ms(1500);

        return RETURN_BACK;
    }
    return RETRUN_SUCCESS;
}

/* -------- DOWNLOAD LOG -------- */
unsigned char download_log(unsigned char reset_flag)
{
    unsigned char i, j, addr, start;
    unsigned char data[11];

    if (reset_flag == RESET_MENU)
    {
        clear_screen();
        clcd_print("Downloading...", LINE1(0));

        puts("\n\r---- CAR BLACK BOX LOG ----\n\r");

        if (log_count == 0)
        {
            puts("No logs available\n\r");
        }
        else
        {
            puts("No   TIME     EVT SP\n\r");

            start = (log_count < 10) ? 0 : pos;

            for (i = 0; i < log_count; i++)
            {
                addr = ((start + i) % 10) * 10;

                for (j = 0; j < 10; j++)
                    data[j] = ext_eeprom_24C02_read(addr + j);

                putchar(i + '0'); puts("   ");

                putchar(data[0]); putchar(data[1]); putchar(':');
                putchar(data[2]); putchar(data[3]); putchar(':');
                putchar(data[4]); putchar(data[5]);

                puts("   ");

                putchar(data[6]); putchar(data[7]);

                puts("   ");

                putchar(data[8]); putchar(data[9]);

                puts("\n\r");
            }
        }

        puts("---------------------------\n\r");

        __delay_ms(1500);
        return RETURN_BACK;
    }

    return RETRUN_SUCCESS;
}


/* -------- MENU -------- */
unsigned char menu_screen(unsigned char reset_flag, unsigned char key)
{
    static unsigned char pos = 0;

    if (reset_flag == RESET_MENU)
    {
        pos = 0;
        clear_screen();
    }

    if (key == SW5 && pos < 3)
    {
        pos++;
        clear_screen();
    }
    else if (key == SW4 && pos > 0)
    {
        pos--;
        clear_screen();
    }

    if (pos < 3)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[pos], LINE1(2));
        clcd_print(menu[pos + 1], LINE2(2));
    }
    else
    {
        clcd_print(menu[pos - 1], LINE1(2));
        clcd_print(menu[pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }

    return pos;
}


/* -------- PASSWORD -------- */
unsigned char check_password(unsigned char key, unsigned char reset_flag)
{
    static unsigned char user_pwd[4];
    
    static unsigned char i, attempts;

    if (reset_flag == RESET_PASSWORD)
    {
        i = 0;
        attempts = 3;
        return_time = 5;
    }

    if (key == SW4 && i < 4)
    {
        user_pwd[i++] = '1';
        clcd_putch('*', LINE2(4 + i));
        return_time = 5;
    }
    else if (key == SW5 && i < 4)
    {
        user_pwd[i++] = '0';
        clcd_putch('*', LINE2(4 + i));
        return_time = 5;
    }

    if (return_time == 0)
        return RETURN_BACK;

    if (i == 4)
    {
        if (strncmp((char *)user_pwd, (char *)saved_pwd, 4) == 0)
        {
            clear_screen();
            clcd_print("Login Success", LINE1(0));
            __delay_ms(1000);
            return RETRUN_SUCCESS;
        }
        else
        {
            attempts--;

            if (attempts == 0)
            {
                clear_screen();
                clcd_print("BLOCKED", LINE1(0));
                secs = 10;

                while (secs)
                {
                    clcd_putch(secs / 10 + '0', LINE2(0));
                    clcd_putch(secs % 10 + '0', LINE2(1));
                }

                return RETURN_BACK;
            }

            clear_screen();
            clcd_print("Wrong Password", LINE1(0));
            __delay_ms(1000);

            i = 0;
        }
    }

    return RESET_NOHING;
}

unsigned char change_password(unsigned char key, unsigned char reset_flag)
{
    static unsigned char state;
    static unsigned char i;
    static unsigned char old_pwd[4], new_pwd[4], confirm_pwd[4];
    static unsigned char stored_pwd[4];

    unsigned char j;

    if (reset_flag == RESET_MENU)
    {
        state = 0;
        i = 0;

        /* Read stored password from EEPROM */
        for (j = 0; j < 4; j++)
            stored_pwd[j] = saved_pwd[j];


        clear_screen();
        clcd_print("Enter Old Pwd", LINE1(0));
        clcd_write(DISP_ON_AND_CURSOR_ON, 0);
    }

    /* -------- INPUT -------- */
    if ((key == SW4 || key == SW5) && i < 4)
    {
        unsigned char val = (key == SW4) ? '1' : '0';

        if (state == 0)
            old_pwd[i] = val;
        else if (state == 1)
            new_pwd[i] = val;
        else if (state == 2)
            confirm_pwd[i] = val;

        clcd_putch('*', LINE2(4 + i));
        i++;
    }

    /* -------- VERIFY OLD PASSWORD -------- */
    if (state == 0 && i == 4)
    {
        if (strncmp((char *)old_pwd, (char *)stored_pwd, 4) == 0)
        {
            state = 1;
            i = 0;

            clear_screen();
            clcd_print("Enter New Pwd", LINE1(0));
        }
        else
        {
            clear_screen();
            clcd_print("Wrong Password", LINE1(0));
            __delay_ms(1500);
            return RETURN_BACK;
        }
    }

    /* -------- ENTER NEW PASSWORD -------- */
    else if (state == 1 && i == 4)
    {
        state = 2;
        i = 0;

        clear_screen();
        clcd_print("Confirm Pwd", LINE1(0));
    }

    /* -------- CONFIRM PASSWORD -------- */
    else if (state == 2 && i == 4)
    {
        if (strncmp((char *)new_pwd, (char *)confirm_pwd, 4) == 0)
        {
            /* Save to EEPROM */
            for (j = 0; j < 4; j++)
                saved_pwd[j] = confirm_pwd[j];

            clear_screen();
            clcd_print("Pwd Changed", LINE1(0));
            __delay_ms(1500);

            return RETURN_BACK;
        }
        else
        {
            clear_screen();
            clcd_print("Mismatch", LINE1(0));
            __delay_ms(1500);

            return RETURN_BACK;
        }
    }

    return RETRUN_SUCCESS;
}