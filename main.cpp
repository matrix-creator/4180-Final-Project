/* 4180 Final Project
 * Yash, Hyochang, Jack, Kameron
 * The RRR (The Rick Rolling Roomba)
 */
#include "mbed.h"
#include "rtos.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(p13, p14, p24);
RawSerial bluemod(p10, p9);
RawSerial lidar(p28, p27);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

Mutex tof;
Mutex lcd;
Mutex bluetooth;
char bnum = 0;
char bhit = 0;

void tof_control(const void *args) {}
void lcd_control(const void *args) {}
void bluetooth_control(const void *args) {
    while(1) {
        bluetooth.lock();
        if (bluemod.getc() == '!') {
            if (bluemod.getc() == 'B') {
                bnum = bluemod.getc();
                bhit = bluemod.getc();
                if (bluemod.getc() == char(~('!' + 'B' + bnum + bhit))) {
                    switch (bnum) {
                        case '1':
                            break;
                        case '2':
                            break;
                        case '3':
                            break;
                        case '4':
                            break;
                        default: break;
                    }
                }
                bluetooth.unlock();
                Thread::wait(100);
            }
        }
    }
}

int main()
{
    Thread sonar_thread(tof_control);
    Thread bluetooth_thread(bluetooth_control);
    Thread lcd_thread(lcd_control);
    while(1) { 
        // Have two DC brushless motors spin indefinitely opposite of each other
        Thread:: wait(100); 
    }
}