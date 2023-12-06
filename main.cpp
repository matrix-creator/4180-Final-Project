/* 4180 Final Project
 * Yash, Hyochang, Jack, Kameron
 * The RRR (The Rick Rolling Roomba)
 */

#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include <string>
#include "mbed.h"
#include "Motor.h"
#include "Servo.h"
#include "XNucleo53L0A1.h"

//LCD
uLCD_4DGL uLCD(p13, p14, p24); // tx,rx,rst

//Bluetooth
RawSerial blue(p10, p9);

//Lidar stuff
DigitalOut shdn(p26);
#define VL53L0_I2C_SDA   p28
#define VL53L0_I2C_SCL   p27

static XNucleo53L0A1 *board=NULL;


//Motorstuff
float straightSpeed = 0.6;
float turnSpeed = 0.5;
Motor left_motor(p21,p5,p6); //pwm,fwd,rev
Motor right_motor(p22,p7,p8); //pwm,fwd,rev




//SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
//AnalogOut DACout(p18);
//wave_player wav(&DACout); // 
//DigitalOut led1(LED1); //
//DigitalOut led2(LED2); //
//char buffer[4] = {0,0,0,0};


//int idx = 0;


Mutex lcd_mutex; // mutex for lcd
Mutex mode_mutex; // mutex for roomba mode



/*
void wav_thread(void const *args)
{
    FILE *wave_file;
    while(1) {
        if (buffer[0] == 's') {
            wave_file = fopen("/sd/mydir/airtraffic.wav","r");
            if(wave_file == NULL) {
                pc.printf("Could not open file for read\n\r");
            } else {
                pc.printf("Found .wav file\n\r");
                wav.play(wave_file);
            }
            fclose(wave_file);
        }
    }
}
*/



enum Mode {
    SPOT,
    RC,
    AUTONOMOUS,
    ENTERTAINMENT
};

Mode currentMode = AUTONOMOUS;

void setMode(Mode mode) {
    currentMode = mode;
    // Add any mode-specific initialization here
}

char bnum = 0;

void bluetooth_control(const void *args) {
    while(1) {
        if (blue.getc()=='!') {
            if (blue.getc()=='B') { //button data packet
                bnum = blue.getc(); //button number
                if (blue.getc()=='1') { //1=hit, 0=release
                    switch (bnum) {
                        case '1': // Spot mode
                            mode_mutex.lock();
                            setMode(SPOT);
                            mode_mutex.unlock();
                            break;
                        case '2': // RC mode
                            mode_mutex.lock();
                            setMode(RC);
                            mode_mutex.unlock();
                            break;
                        case '3': // Autonomous mode
                            mode_mutex.lock();
                            setMode(AUTONOMOUS);
                            mode_mutex.unlock();
                            break;
                        case '4': // Entertainment mode
                            mode_mutex.lock();
                            setMode(ENTERTAINMENT);
                            mode_mutex.unlock();
                            break;
                        case '5': //button 5 up arrow
                            if (currentMode == RC) {
                            left_motor.speed(straightSpeed);
                            right_motor.speed(straightSpeed);
                            }
                            break;
                        case '6': //button 6 down arrow
                            if (currentMode == RC) {
                            left_motor.speed(-1*straightSpeed);
                            right_motor.speed(-1*straightSpeed);
                            }
                            break;
                        case '7': //button 7 left arrow
                            if (currentMode == RC) {
                            left_motor.speed(-1*turnSpeed);
                            right_motor.speed(turnSpeed);
                            }
                            break;
                        case '8': //button 8 right arrow
                            if (currentMode == RC) {
                            left_motor.speed(turnSpeed);
                            right_motor.speed(-1*turnSpeed);
                            }
                            break;
                        default:
                            break;
                    }
                    blue.getc();
                }
            }
        }
    }
}

volatile uint32_t distanceValue = 0;  // Global variable for storing distance value

void autonomousDistanceThread(const void *args){
        int status;
        uint32_t distance;
        DevI2C *device_i2c = new DevI2C(VL53L0_I2C_SDA, VL53L0_I2C_SCL);
        /* creates the 53L0A1 expansion board singleton obj */
        board = XNucleo53L0A1::instance(device_i2c, A2, D8, D2);
        shdn = 0; //must reset sensor for an mbed reset to work
        wait(0.1);
        shdn = 1;
        wait(0.1);
        /* init the 53L0A1 board with default values */
        status = board->init_board();
        while (status) {
            pc.printf("Failed to init board! \r\n");
            status = board->init_board();
        }
        //loop taking and printing distance
        while (1) {
            status = board->sensor_centre->get_distance(&distance);
            //if (status == VL53L0X_ERROR_NONE) {
            //    pc.printf("D=%ld mm\r\n", distance);
            //}
            distanceValue = distance;
            wait(0.1);
        }
}

void spotMode() {
    while (currentMode == SPOT) {
        // Spot mode logic
        lcd_mutex.lock();
        uLCD.cls();
        uLCD.text_string("Spot Mode", 1, 1, FONT_7X8, GREEN);
        lcd_mutex.unlock();

        // Make the robot continuously rotate in place
        left_motor.speed(turnSpeed);
        right_motor.speed(-turnSpeed);
        
    }
}

void rcMode() {
    while (currentMode == RC) {
        // RC mode logic
        lcd_mutex.lock();
        uLCD.cls();
        uLCD.text_string("RC Mode", 1, 1, FONT_7X8, BLUE);
        lcd_mutex.unlock();
        // Your additional RC mode logic goes here
    }
}

void autonomousMode() {
    while (currentMode == AUTONOMOUS) {
        // Autonomous mode logic
        lcd_mutex.lock();
        uLCD.cls();
        uLCD.text_string("Autonomous Mode", 1, 1, FONT_7X8, RED);
        lcd_mutex.unlock();

        

        if (distanceValue < 150) {
        // Reverse slightly when less than 150 mm from an object
        left_motor.speed(-straightSpeed);
        right_motor.speed(-straightSpeed);
        wait(1.0);  //Time for reverse action
        }
       
    }
}

void entertainmentMode() {
    while (currentMode == ENTERTAINMENT) {
        // Entertainment mode logic
        lcd_mutex.lock();
        uLCD.cls();
        uLCD.text_string("Entertainment Mode", 1, 1, FONT_7X8, RED);
        lcd_mutex.unlock();
        // Your additional Entertainment mode logic goes here
    }
}

int main()
{

    Thread th1(bluetooth_control);
    Thread th2(autonomousDistanceThread);

    while (true) {
        mode_mutex.lock();    // since it could read currentmode while the bluetooth change the currentMode variable
        switch (currentMode) {
            case SPOT:
                spotMode();
                break;
            case RC:
                rcMode();
                break;
            case AUTONOMOUS:
                autonomousMode();
                break;
            case ENTERTAINMENT:
                entertainmentMode();
                break;
        mode_mutext.unlock(); // HC make sure unlcok to avoid deadlock
        }
    }
}
