#include <xc.h>
#include <string.h>

#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "car_black_box.h"
#include "adc.h"
#include "timers.h"
#include "uart.h"

#pragma config WDTE = OFF

unsigned char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};

static void init_config(void)
{
    init_i2c(100000);
    init_ds1307();
    init_clcd();
    init_digital_keypad();
    init_adc();
    init_timer2();

    init_uart(9600);

    PEIE = 1;
    GIE = 1;
}

void main(void)
{
    unsigned char screen_flag = DASH_BOARD;
    unsigned char reset_flag = RESET_NOHING;

    unsigned char key;
    unsigned char speed = 0;
    unsigned char menu_pos = 0;

    unsigned char event[3] = "ON";
    unsigned char gr = 0;

    static unsigned char prev_screen = 0xFF;

    init_config();

    log_event(event, 0);   // initial log

    while (1)
    {
        key = read_digital_keypad();

        /* -------- SPEED (SAFE FOR SIMULATION) -------- */
        speed = read_adc(CHANNEL0) / 10;
        if (speed > 99)
            speed = 99;

        /* -------- EVENT CONTROL -------- */
        if (screen_flag == DASH_BOARD)
        {
            if (key == SW1)
            {
                strcpy((char *)event, "C ");
                log_event(event, speed);
            }
            else if (key == SW2)
            {
                if (gr < 5)
                    gr++;
                strcpy((char *)event, gear[gr]);
                log_event(event, speed);
            }
            else if (key == SW3)
            {
                if (gr > 0)
                    gr--;
                strcpy((char *)event, gear[gr]);
                log_event(event, speed);
            }
            else if (key == SW4 || key == SW5)
            {
                screen_flag = PASSWORD_SCREEN;
                reset_flag = RESET_PASSWORD;

                TMR2ON = 1;
            }
        }

        /* -------- MENU SELECT -------- */
        else if (screen_flag == MENU_FLAG && key == LPSW4)
        {
            switch (menu_pos)
            {
                case 0:
                    screen_flag = VIEW_LOG;
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;

                case 1:
                    screen_flag = CLEAR_LOG;
                    reset_flag = RESET_MENU;
                    break;

                case 2:
                    screen_flag = DOWNLOAD_LOG;
                    reset_flag = RESET_MENU;
                    break;

                case 3:
                    screen_flag = CHANGE_PSWD;
                    reset_flag = RESET_MENU;
                    break;
            }
        }

        /* -------- CLEAR SCREEN ONLY ON CHANGE -------- */
        if (prev_screen != screen_flag)
        {
            clear_screen();
            prev_screen = screen_flag;
        }

        /* -------- STATE MACHINE -------- */
        switch (screen_flag)
        {
            case DASH_BOARD:
                display_dashboard(event, speed);
                break;

            case PASSWORD_SCREEN:
            {
                clcd_print(" Enter Password ", LINE1(0));
                unsigned char ret = check_password(key, reset_flag);

                if (ret == RETRUN_SUCCESS)
                {
                    screen_flag = MENU_FLAG;
                    reset_flag = RESET_MENU;
                    TMR2ON = 0;
                }
                else if (ret == RETURN_BACK)
                {
                    screen_flag = DASH_BOARD;
                    TMR2ON = 0;
                }
                break;
            }

            case MENU_FLAG:
                menu_pos = menu_screen(reset_flag, key);
                break;

            case VIEW_LOG:
                view_log(key, reset_flag);
                if (key == LPSW5)
                {
                    screen_flag = MENU_FLAG;
                    reset_flag = RESET_MENU;
                }
                break;

            case CLEAR_LOG:
                if (clear_log(reset_flag) == RETURN_BACK)
                {
                    screen_flag = MENU_FLAG;
                    reset_flag = RESET_MENU;
                }
                break;

            case DOWNLOAD_LOG:
                if (download_log(reset_flag) == RETURN_BACK)
                {
                    screen_flag = MENU_FLAG;
                    reset_flag = RESET_MENU;
                }
                break;

            case CHANGE_PSWD:
                 if (change_password(key, reset_flag) == RETURN_BACK)
                 {
                     screen_flag = MENU_FLAG;
                     reset_flag = RESET_MENU;
                }
                 break;
        }

        reset_flag = RESET_NOHING;
    }
}