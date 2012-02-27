
#include "mbed.h"
#include "SDFileSystem.h"


//----------------------------------------



//the definiton of what a counter is, as well as how to increace and reset it
class Counter {
private:
    InterruptIn _interrupt;
    volatile int _count;

public:
    Counter(PinName pin) : _interrupt(pin) {        // create the InterruptIn on the pin specified to Counter
        _interrupt.rise(this, &Counter::increment); // attach increment function of this counter instance
    }

    void increment() {
        _count++;
    }

    int read() {
        return _count;
    }

    int operator int() {
        return read();
    }

    void reset() {
        _count = 0;
    }
};

//----------------------------------------

AnalogIn windVane(p15); //The thingy that does not spin, but points
Serial pc(USBTX, USBRX);
Timeout timeout;
Counter WSCounter(p14);             //14
Counter RFCounter(p13);             //13
DigitalOut led1(LED1);
AnalogIn Humid(p16);
AnalogIn Temp(p17);
LocalFileSystem local("local");
SDFileSystem sd(p5, p6, p7, p8, "sd");

float Voltage;


//----------------------------------------

void reset_RN_value() {
    RFCounter.reset();
}

//DATA container
char Buffer[1024];

#define printToBuffer(...) writeToBuffer(__VA_ARGS__)

//kep-press
char c;

char FileName[16];

//file overwrite/read option
char choice [1];

FILE* fp;

//wind direction
void Wind_dir () {
    float v = windVane * 16;
    writeToBuffer("Wind Direction ");
    if (v < 1 || v >= 15) {
        writeToBuffer("North      at:");
    }
    else if (v < 3) {
        writeToBuffer("North East at:");
    }
    else if (v < 5) {
        writeToBuffer("East       at:");
    }
    else if (v < 7) {
        writeToBuffer("South East at:");
    }
    else if (v < 9) {
        writeToBuffer("South      at:");
    }
    else if (v < 11) {
        writeToBuffer("South West at:");
    }
    else if (v < 13) {
        writeToBuffer("West       at:");
    }
    else if (v < 15) {
        writeToBuffer("North West at:");
    }
}

void WindSpeed() {
    writeToBuffer("  %03dm/s  ", WSCounter.read());
    WSCounter.reset();
}

void RainGauge() {
    float RainFall=0.0;
    RainFall = RFCounter.read() * 0.2;
    writeToBuffer("Rain Fall per Hour: %03.1fmm ",RainFall);
}


void Humidity () {
    float humidity = 0.0;
    humidity = ((Humid * 3.3) -1.60)/0.0312;
    writeToBuffer("Humidity: ");
    writeToBuffer("%04.1f%s",humidity,"%% ");
}


void Temperature () {
    float Temperature1 = 0.0;
    Temperature1 = ((1000/Temp)-2005 )/4;
    writeToBuffer("temperature is %02.0f°C   ", Temperature1);
}

//Write data to log file on microSD card (i used the BOB V1.0)
void Choice () {
    int N=4;
    pc.printf("\r\n\r\nInput file name : ");
    FileName[0] = '/';
    FileName[1] = 's';
    FileName[2] = 'd';
    FileName[3] = '/';
    do {
        FileName[N] = pc.getc();
        pc.printf("%c",FileName[N]);
    } while (FileName[N++] != '\n' || N < 11);
    pc.printf("\n\r%s\n\r", &FileName[0]);
    printf(&FileName[--N],".txt");
    pc.printf("\n\r%s\n\r", &FileName[0]);
    wait (0.5);
    //Does it Exist? Yes go to options no - create - Y/N??  Y -> Create and continue N-> Go to do
    fp = fopen(&FileName[0], "r");
    if (!fp) {
        pc.printf("%s Doesn't Exist! Creating new file...\n\r", &FileName[0]);
        fp = fopen(&FileName[0], "w");
        if (!fp) {
            pc.printf("ERROR CREATING FILE!");
        }
    } else {
        fclose(fp);
        pc.printf("\r\n\r\n%s allready exists, do you want to: \n\r Append [A] \n\r Overwrite [O] \n\r or Create a new file [C]\n\r", &FileName[0]);
        while(c = pc.getc())
            if (c == 'A' || c == 'a') {
                pc.printf("Appending %s", &FileName[0]);
                fp = fopen(&FileName[0], "a");
                break;
            }
            if (c == 'O' || c == 'o') {
                pc.printf("Overwriting %s", &FileName[0]);
                fp = fopen(&FileName[0], "w");
                break;
            }
        }
    }
    pc.printf("\n\rFile opened successfuly\n\r");
    fclose(fp);
}


void Write () {
    pc.printf(Buffer);
    fp = fopen(&FileName[0], "a");
    fprintf(fp, Buffer);
    Buffer[0] = 0;
    fclose(fp);
}

//----------------------------------------

int main() {
    Choice();
    writeToBuffer("\r\n ----- M-BED Weather Monitoring -----\r\n\r\n");
    while (1) {
        Wind_dir();
        WindSpeed();
        RainGauge();
        Humidity();
        Temperature();
        writeToBuffer("\r\n-----\r\n");
        Write();
        wait(1.0);
    }
}
//----------------------------------------

