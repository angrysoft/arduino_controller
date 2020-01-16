#ifndef RgbStrip_h
#define RgbStrip_h
#define CLK 9 //pins definitions for the driver        
#define DIO 10
#include <Arduino.h>
#include <RGBdriver.h>

class RgbStrip {
	
	private:
		int red;
		int green;
		int blue;
		int bright;
		bool power;
        int dupa();
        void fade(int t_red, int t_green, int t_blue);
		void setRGB(int r, int g, int b);
        int calcStep(int start, int end);
        int calcVal(int val, int target, int step, int loop);
		int getRed();
		int getGreen();
		int getBlue();
		const char* getPower();

    public:
		RgbStrip();
		void set_rgb(long int rgb);
		void set_bright(int bright);
		void set_power(int power);
		void report();
};
#endif