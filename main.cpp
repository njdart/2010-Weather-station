

#include "mbed.h"
#include "SDFileSystem.h"


//----------------------------------------



//the definiton of what a counter is, as well as how to increace and reset it
class Counter {
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
    void reset() {
        _count = 0;
    }



private:
    InterruptIn _interrupt;
    volatile int _count;
};

//----------------------------------------

DigitalOut lde1(LED1);
AnalogIn In1 (p15);
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

//kep-press
char c;

char FileName[16];

//file overwrite/read option
char choice [1];

FILE* fp;

//wind direction
void Wind_dir () {
    Voltage = In1;
    printf(Buffer + strlen(Buffer), "Wind Direction ");
    if ((In1 >= 0.9375)||(In1 < 0.0625)) {
        printf(Buffer + strlen(Buffer), "North      at:");
    }
    if ((In1 >= 0.0625)&&(In1 < 0.1875)) {
        printf(Buffer + strlen(Buffer), "North East at:");
    }
    if ((In1 >= 0.1875)&&(In1 < 0.3125)) {
        printf(Buffer + strlen(Buffer), "East       at:");
    }
    if ((In1 >= 0.341275)&&(In1 < 0.4375)) {
        printf(Buffer + strlen(Buffer), "South East at:");
    }
    if ((In1 >= 0.4375)&&(In1 < 0.5625)) {
        printf(Buffer + strlen(Buffer), "South      at:");
    }
    if ((In1 >= 0.5625)&&(In1 < 0.6875)) {
        printf(Buffer + strlen(Buffer), "South West at:");
    }
    if ((In1 >= 0.6875)&&(In1 < 0.8125)) {
        printf(Buffer + strlen(Buffer), "West       at:");
    }
    if ((In1 >= 0.8125)&&(In1 <= 0.9375)) {
        printf(Buffer + strlen(Buffer), "North West at:");
    }
}

void WindSpeed() {
    printf(Buffer + strlen(Buffer),"  %03dm/S  ", WSCounter.read());
    WSCounter.reset();
}

void RainGauge() {
    float RainFall=0.0;
    RainFall = RFCounter.read() * 0.2;
    printf(Buffer + strlen(Buffer),"Rain Fall per Hour: %03.1fmm ",RainFall);
}


void Humidity () {
    float humidity = 0.0;
    humidity = ((Humid * 3.3) -1.60)/0.0312;
    printf(Buffer + strlen(Buffer),"Humidity: ");
    printf(Buffer + strlen(Buffer), "%04.1f%s",humidity,"%% ");
}


void Temperature () {
    float Temperature1 = 0.0;
    Temperature1 = ((1000/Temp)-2005 )/4;
    printf(Buffer + strlen(Buffer), "temperature is %02.0f°C   ", Temperature1);
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
    } while ((FileName[N++] == '\n') | (N < 11));
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
        do {
            c = pc.getc();
        } while ( !((c == 'A')||(c == 'a')||(c == 'O')||(c == 'o')) );
        if ((c == 'A')||(c == 'a')) {
            pc.printf("Appending %s", &FileName[0]);
            fp = fopen(&FileName[0], "a");
        }
        if ((c == 'O') || (c == 'o')) {
            pc.printf("Overwriting %s", &FileName[0]);
            fp = fopen(&FileName[0], "w");
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
    printf(Buffer + strlen(Buffer), "\r\n ----- M-BED Weather Monitoring -----\r\n\r\n");
    while (1) {
        Wind_dir();
        WindSpeed();
        RainGauge();
        Humidity();
        Temperature();
        printf(Buffer + strlen(Buffer), "\r\n-----\r\n");
        Write();
        wait(1.0);
    }
}
//----------------------------------------

