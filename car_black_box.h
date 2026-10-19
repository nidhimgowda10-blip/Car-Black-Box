#ifndef CAR_BLACK_BOX_H
#define	CAR_BLACK_BOX_H

#define DASH_BOARD              0x01
#define MENU_FLAG               0x02
#define PASSWORD_SCREEN         0x03
#define VIEW_LOG                0x04
#define CLEAR_LOG               0x05
#define DOWNLOAD_LOG            0x06
#define CHANGE_PSWD             0x07

#define RESET_PASSWORD         0x13
#define RESET_MENU             0x12
#define RESET_NOHING           0x00
#define RESET_VIEW_LOG_POS     0x14

#define RETRUN_SUCCESS         0xA0
#define RETURN_BACK            0xA1

void display_dashboard(unsigned char event[], unsigned char speed);
void log_event(unsigned char event[], unsigned char speed);

unsigned char check_password(unsigned char key, unsigned char reset_flag);
unsigned char menu_screen(unsigned char reset_flag, unsigned char key);

/* ? ADD THESE (YOU MISSED THEM) */
void view_log(unsigned char key, unsigned char reset_flag);
unsigned char clear_log(unsigned char reset_flag);
unsigned char download_log(unsigned char reset_flag);
unsigned char change_password(unsigned char key, unsigned char reset_flag);
#endif