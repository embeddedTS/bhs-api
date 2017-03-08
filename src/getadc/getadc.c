/* To compile mx28adcctl, use the appropriate cross compiler and run the
* command:
*
*  gcc -fno-tree-cselim -Wall -O0 -mcpu=arm9 -o mx28adcctl mx28adcctl.c
*/

#include <assert.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void gpioExport(int gpio)
{
    int fd;
    char buf[255];
    fd = open("/sys/class/gpio/export", O_WRONLY);
    sprintf(buf, "%d", gpio); 
    write(fd, buf, strlen(buf));
    close(fd);
}

void gpioDirection(int gpio, int direction) // 1 for output, 0 for input
{
    int fd;
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(buf, O_WRONLY);

    if (direction)
    {
        write(fd, "out", 3);
    }
    else
    {
        write(fd, "in", 2);
    }
    close(fd);
}

void gpioSet(int gpio, int value)
{
    int fd;
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(buf, O_WRONLY);
    sprintf(buf, "%d", value);
    write(fd, buf, 1);
    close(fd);
}


int main(int argc, char **argv) {
    volatile unsigned int *mxlradcregs;
    volatile unsigned int *mxhsadcregs;
    volatile unsigned int *mxclkctrlregs;
    unsigned int i, x;
    unsigned long long chan[8] = {0,0,0,0,0,0,0,0};
    int devmem;
    int activityLed = 58;

    // Setup Activity LED
    gpioExport(activityLed); 
    gpioDirection(activityLed, 1);

    // Turn on the activity LED 
    gpioSet(activityLed, 0); // 1 = off, 0 = on

    devmem = open("/dev/mem", O_RDWR|O_SYNC);
    assert(devmem != -1);

    // LRADC
    mxlradcregs = (unsigned int *) mmap(0, getpagesize(),
      PROT_READ | PROT_WRITE, MAP_SHARED, devmem, 0x80050000);

    mxlradcregs[0x148/4] = 0xfffffff; //Clear LRADC6:0 assignments
    mxlradcregs[0x144/4] = 0x6543210; //Set LRDAC6:0 to channel 6:0
    mxlradcregs[0x28/4] = 0xff000000; //Set 1.8v range
    for(x = 0; x < 7; x++)
      mxlradcregs[(0x50+(x * 0x10))/4] = 0x0; //Clear LRADCx reg

    for(x = 0; x < 10; x++) {
        mxlradcregs[0x18/4] = 0x7f; //Clear interrupt ready
        mxlradcregs[0x4/4] = 0x7f; //Schedule conversaion of chan 6:0
        while(!((mxlradcregs[0x10/4] & 0x7f) == 0x7f)); //Wait
        for(i = 0; i < 7; i++)
          chan[i] += (mxlradcregs[(0x50+(i * 0x10))/4] & 0xffff);
    }

    mxhsadcregs = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,
      devmem, 0x80002000);
    mxclkctrlregs = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,
      devmem, 0x80040000);

    // HDADC
    //Lets see if we need to bring the HSADC out of reset
    if(mxhsadcregs[0x0/4] & 0xC0000000) {
        mxclkctrlregs[0x154/4] = 0x70000000;
        mxclkctrlregs[0x1c8/4] = 0x8000;
        //ENGR116296 errata workaround
        mxhsadcregs[0x8/4] = 0x80000000;
        mxhsadcregs[0x0/4] = ((mxhsadcregs[0x0/4] | 0x80000000) & (~0x40000000));
        mxhsadcregs[0x4/4] = 0x40000000;
        mxhsadcregs[0x8/4] = 0x40000000;
        mxhsadcregs[0x4/4] = 0x40000000;

        usleep(10);
        mxhsadcregs[0x8/4] = 0xc0000000;
    }

    mxhsadcregs[0x28/4] = 0x2000; //Clear powerdown
    mxhsadcregs[0x24/4] = 0x31; //Set precharge and SH bypass
    mxhsadcregs[0x30/4] = 0xa; //Set sample num
    mxhsadcregs[0x40/4] = 0x1; //Set seq num
    mxhsadcregs[0x4/4] = 0x40000; //12bit mode

    while(!(mxhsadcregs[0x10/4] & 0x20)) {
        mxhsadcregs[0x50/4]; //Empty FIFO
    }

    mxhsadcregs[0x50/4]; //An extra read is necessary

    mxhsadcregs[0x14/4] = 0xfc000000; //Clr interrupts
    mxhsadcregs[0x4/4] = 0x1; //Set HS_RUN
    usleep(10);
    mxhsadcregs[0x4/4] = 0x08000000; //Start conversion
    while(!(mxhsadcregs[0x10/4] & 0x1)) ; //Wait for interrupt

    for(i = 0; i < 5; i++) {
        x = mxhsadcregs[0x50/4];
        chan[7] += ((x & 0xfff) + ((x >> 16) & 0xfff));
    }

    if (argc < 2) {
	printf("%s returns the analog input voltage in mV\n", argv[0]);
        printf("   Usage: %s <ADC_PIN_NUMBER>\n", argv[0]);

        // Turn off activity LED 
        gpioSet(activityLed, 1); 
        return 1;
    }

    int adcPin = atoi(argv[1]);

    //printf("LRADC_ADC%d_val=%d\n", adcPin, (unsigned int)((((chan[adcPin]/10)*45177)*6235)/100000000));
    printf("%d", (unsigned int)((((chan[adcPin]/10)*45177)*6235)/100000000));

    // Turn off activity LED 
    gpioSet(activityLed, 1);

    return 0;
}
