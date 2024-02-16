/**
 * Hunter Adams (vha3@cornell.edu)
 * 
 * HARDWARE CONNECTIONS
 *  - GPIO 16 ---> VGA Hsync
 *  - GPIO 17 ---> VGA Vsync
 *  - GPIO 18 ---> 470 ohm resistor ---> VGA Green 
 *  - GPIO 19 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 20 ---> 330 ohm resistor ---> VGA Blue
 *  - GPIO 21 ---> 330 ohm resistor ---> VGA Red
 *  - RP2040 GND ---> VGA GND
 * 
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels obtained by claim mechanism
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 * Protothreads v1.1.3
 * Threads:
 * core 0:
 * Graphics demo
 * blink LED25 
 * core 1:
 * Toggle gpio 4 
 * Serial i/o 
 */
// ==========================================
// === VGA graphics library
// ==========================================
#include "vga16_graphics.h"
#include <stdio.h>
#include <stdlib.h>
// #include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
// // Our assembled programs:
// // Each gets the name <pio_filename.pio.h>
// #include "hsync.pio.h"
// #include "vsync.pio.h"
// #include "rgb.pio.h"

// ==========================================
// === protothreads globals
// ==========================================
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "string.h"
// protothreads header
#include "pt_cornell_rp2040_v1_3.h"

char user_string[40] = "Type up to 40 characters" ;
int new_str = 1;

// ==================================================
// === graphics demo -- RUNNING on core 0
// ==================================================
static PT_THREAD (protothread_graphics(struct pt *pt)) {
    PT_BEGIN(pt);
    // the protothreads interval timer
    PT_INTERVAL_INIT() ;

  
    // setCursor(320, 240) ;
    // setTextColor(WHITE) ;
    // writeStringBig("Timer") ;



    // Setup a 1Hz timer
    //static struct repeating_timer timer;
    //add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);
    static short min = 0;
    static short sec = 0;
    static short msec = 0;
    setTextSize(4);

    while(true) {
      msec++;
      if(msec == 1000){

        char buffer[5];
        setTextColor(BLACK);

        setCursor(250, 240) ;
        sprintf(buffer, "%d", sec);
        writeString(buffer);

        sec++;
        msec = 0;

        setTextColor(WHITE);
        setCursor(250, 240) ;
        sprintf(buffer, "%d", sec);
        writeString(buffer);

      }
      if(sec == 60){
        
        char buffer[5];

        setTextColor(BLACK);
        setCursor(250, 240) ;
        sprintf(buffer, "%d", sec);
        writeString(buffer);

        setCursor(180, 240) ;
        sprintf(buffer, "%d", min);
        writeString(buffer);

        sec = 0;
        min++;

        setTextColor(WHITE);
        setCursor(250, 240) ;
        sprintf(buffer, "%d", sec);
        writeString(buffer);

        setCursor(180, 240) ;
        sprintf(buffer, "%d", min);
        writeString(buffer);
        
      }
      


      PT_YIELD_usec(1000);
    }




   PT_END(pt);
} // graphics thread

// ==================================================
// === toggle25 thread on core 0
// ==================================================
// the on-board LED blinks
static PT_THREAD (protothread_toggle25(struct pt *pt))
{
    PT_BEGIN(pt);
    static bool LED_state = false ;
    
     // set up LED p25 to blink
     gpio_init(25) ;	
     gpio_set_dir(25, GPIO_OUT) ;
     gpio_put(25, true);
     // data structure for interval timer
     PT_INTERVAL_INIT() ;

      while(1) {
        // yield time 0.1 second
        //PT_YIELD_usec(100000) ;
        PT_YIELD_INTERVAL(100000) ;

        // toggle the LED on PICO
        LED_state = LED_state? false : true ;
        gpio_put(25, LED_state);
        //
        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // blink thread

// ==================================================
// === user's serial input thread on core 1
// ==================================================
// serial_read an serial_write do not block any thread
// except this one
static PT_THREAD (protothread_serial(struct pt *pt))
{
    PT_BEGIN(pt);
      static int test_in1, test_in2, sum ;
      //
      while(1) {
        // print prompt
        sprintf(pt_serial_out_buffer, "input string <40 char: ");
        // spawn a thread to do the non-blocking write
        serial_write ;

        // spawn a thread to do the non-blocking serial read
         serial_read ;
        // convert input string to number
        strcpy(user_string, pt_serial_in_buffer) ;
        new_str = true ;

        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // serial thread

// ========================================
// === core 1 main -- started in main below
// ========================================
void core1_main(){ 
  //
  //  === add threads  ====================
  // for core 1
  //pt_add_thread(protothread_toggle_gpio4) ;
  pt_add_thread(protothread_serial) ;
  //
  // === initalize the scheduler ==========
  pt_schedule_start ;
  // NEVER exits
  // ======================================
}

// ========================================
// === core 0 main
// ========================================
int main(){
  // set the clock
  //set_sys_clock_khz(250000, true); // 171us
  // start the serial i/o
  stdio_init_all() ;
  // announce the threader version on system reset
  printf("\n\rProtothreads RP2040 v1.3 two-core, priority\n\r");

  // Initialize the VGA screen
  initVGA() ;
     
  // start core 1 threads
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // === config threads ========================
  // for core 0
  pt_add_thread(protothread_graphics);
  pt_add_thread(protothread_toggle25);
  //
  // === initalize the scheduler ===============
  pt_schedule_start ;
  // NEVER exits
  // ===========================================
} // end main