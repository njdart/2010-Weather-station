
#include "mbed.h"
#include "SDFileSystem.h"

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

class WeatherStation {
  private:
	AnalogIn _windVane(p15);
	AnalogIn _humid(p16);
	AnalogIn _temp(p17);
	Counter  _windSpeed(p14);
	Counter  _rainFall(p14);

  public:
	enum Direction {N, NE, E, SE, S, SW, W, NW}

	Direction windDirection() {
		int v = (_windVane * 16 + 1) % 16;
		v /= 2;
		return (Direction) v;
	}

	float windSpeed() {
		return _windSpeed;
	}
	float rainFall() {
		return _rainFall * 0.2;
	}
	float humidity() {
		return ((_humid * 3.3) -1.60)/0.0312;
	}
	float temperature() {
		return ((1000/_temp)-2005)/4;
	}

	void resetWind() {
		_windSpeed.reset();
	}

};

//----------------------------------------

Serial pc(USBTX, USBRX);
LocalFileSystem local("local");
SDFileSystem sd(p5, p6, p7, p8, "sd");


//data container
char Buffer[1024];

#define printToBuffer(...) writeToBuffer(__VA_ARGS__)

//kep-press
char c;

char FileName[16];

//file overwrite/read option







FILE* fp;


WeatherStation w;


//Write data to log file on microSD card (i used the BOB V1.0)
void Choice () {
	int N=4;
	pc.printf("\n\nInput file name : ");
	FileName[0] = '/';
	FileName[1] = 's';
	FileName[2] = 'd';
	FileName[3] = '/';
	do {
		FileName[N] = pc.getc();
		pc.printf("%c",FileName[N]);
	} while (FileName[N++] != '\n' || N < 11);
	pc.printf("\n%s\n", &FileName[0]);
	printf(&FileName[--N],".txt");
	pc.printf("\n%s\n", &FileName[0]);
	wait (0.5);
	//Does it Exist? Yes go to options no - create - Y/N??  Y -> Create and continue N-> Go to do
	fp = fopen(&FileName[0], "r");
	if (!fp) {
		pc.printf("%s Doesn't Exist! Creating new file...\n", &FileName[0]);
		fp = fopen(&FileName[0], "w");
		if (!fp) {
			pc.printf("ERROR CREATING FILE!");
		}
	} else {
		fclose(fp);
		pc.printf("\n\n%s already exists, do you want to: \n Append [A] \n Overwrite [O] \n or Create a new file [C]\n", &FileName[0]);
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
	pc.printf("\nFile opened successfuly\n");
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

char* getDirection(Direction d) {
	switch(d) {
		case N:  return "North";
		case NE: return "North East";
		case E:  return "East";
		case SE: return "South East";
		case S:  return "South";
		case SW: return "South West";
		case W:  return "West";
		case NW: return "North West";
	}
}


int main() {
	Choice();
	writeToBuffer("\n ----- M-BED Weather Monitoring -----\n\n");
	while(true) {
		writeToBuffer("\nWind: %03dm/s %s",       w.windSpeed(), getDirection(w.windDirection()));
		writeToBuffer("\nRain Fall: %03.1fmm",    w.rainFall());
		writeToBuffer("\nHumidity: %04.1f%s",     w.humidity(), "%% ");
		writeToBuffer("\nTemperature: %02.0f°C",  w.temperature());

		writeToBuffer("\n-----/0");
		Write();
		wait(1.0);
		w.resetWind();
	}
}