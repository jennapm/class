
#include </Applications/microchip/xc32/v1.34/pic32mx/include/xc.h> //processor SFR definitions
#include<sys/attribs.h> // __ISR macro

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // not boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins by turning sosc off
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 40MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_20 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 40MHz
#pragma config UPLLIDIV = DIV_2 // divide 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0x0000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = ON // allow only one reconfiguration
#pragma config IOL1WAY = ON // allow only one reconfiguration
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // controlled by USB module

int readADC(void);

int main(){

//startup
__builtin_disable_interrupts();

// set the CP0 CONFIG register to indicate that
// kseg0 is cacheable (0x3) or uncacheable (0x2)
// see Chapter 2 "CPU for Devices with M4K Core"
// of the PIC32 reference manual
__builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

// no cache on this chip!

// 0 data RAM access wait states
BMXCONbits.BMXWSDRM = 0x0;

// enable multi vector interrupts
INTCONbits.MVEC = 0x1;

// disable JTAG to be able to use TDI, TDO, TCK, TMS as digital
DDPCONbits.JTAGEN = 0;

__builtin_enable_interrupts();



//set up USER pin as input
//U1RXRbits.U1RXR = 0011;                 // set U1RX to pin B13
ANSELBbits.ANSB13 = 0;
TRISBbits.TRISB13 = 1;                  //set USER to digital input



// set up LED1 pin as a digital output
//RPB7Rbits.RPB7R = 0b0001;               // set B7 to U1TX
TRISBbits.TRISB7 = 0;                   //set LED1 to digital output
LATBbits.LATB7 = 1;



// set up LED2 as OC1 using Timer2 at 1kHz
RPB15Rbits.RPB15R = 0b0101;             // set B15 to U1TX
ANSELBbits.ANSB15 = 0;
T2CONbits.TCKPS = 0b000;                //set T2 prescaler to 1
TMR2 = 0;                               //set count to 0
PR2 = 39999;                            //period register
OC1CONbits.OCM = 0b110;                 //PWM mode on OC1, fault pin disabled
OC1R = 20000;                           //duty cycle initially 50%
T2CONbits.ON = 1;                       //T2 on
OC1CONbits.ON = 1;                      //OC1 is on




 // set up A0 as AN0
    ANSELAbits.ANSA0 = 1;
    AD1CON3bits.ADCS = 3;
    AD1CHSbits.CH0SA = 0;
    AD1CON1bits.ADON = 1;
 
int val;
while (1) {
   
    _CP0_SET_COUNT(0); // set core timer to 0, remember it counts at half the CPU clock = 20MHz
    LATBINV = 0x80; // invert pin B7

    // wait for half a second, setting LED brightness to pot angle while waiting
    while (_CP0_GET_COUNT() < 10000000) {
        val = readADC();                    //10 bit value
        OC1RS = val * 36.36;                //((ADC counts)*0.003/3.3)*40,000

        if (PORTBbits.RB13 == 1) {         //1 when not pressed
            // nothing
        } else {
            LATBINV = 0x80;
        }
    }
}
}

int readADC(void) {
    int elapsed = 0;
    int finishtime = 0;
    int sampletime = 20;
    int a = 0;

    AD1CON1bits.SAMP = 1;
    elapsed = _CP0_GET_COUNT();
    finishtime = elapsed + sampletime;
    while (_CP0_GET_COUNT() < finishtime) {
    }
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE) {
    }
    a = ADC1BUF0;                   //10 bit conversion result (0-1023)
    return a;
}