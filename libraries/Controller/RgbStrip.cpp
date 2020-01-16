#include "RgbStrip.h"
RGBdriver Driver(CLK,DIO);

RgbStrip::RgbStrip() {
	this->red = 255;
	this->green = 199;
	this->blue = 143;
	this->power = false;
	this->bright = 100;
}

void RgbStrip::set_rgb(long int rgb) {
  this->power = 1;
  int t_red = rgb >> 16 & 255;
  int t_green = rgb >> 8 & 255;
  int t_blue = rgb & 255;
  t_red = t_red * this->bright / 100;
  t_green = t_green * this->bright / 100;
  t_blue = t_blue * this->bright / 100;
  this->fade(t_red, t_green, t_blue);
  this->red = t_red;
  this->green = t_green;
  this->blue = t_blue;
}


void RgbStrip::set_bright(int bright) {
  if (bright < 1 or bright > 100) {return;}
  this->power = 1;
  int t_red = this->red * bright / 100;
  int t_green = this->green * bright / 100;
  int t_blue = this->blue * bright / 100;
  this->fade(t_red, t_green, t_blue);
  this->bright = bright;
}

void RgbStrip::set_power(bool power) {
  if (power and not this->power) {
    this->power = 1;
    int bright = this->bright;
    this->bright = 0;
    this->set_bright(bright);
  } else if (not power and this->power) {
    this->power = 0;
    this->fade(0, 0, 0);
  }
}


void RgbStrip::setRGB(int r, int g, int b) {
	Driver.begin();
	Driver.SetColor(r, g, b);
	Driver.end();
}

void RgbStrip::fade(int t_red, int t_green, int t_blue) {
    int r = this->getRed();
    int g = this->getGreen();
    int b = this->getBlue();

    int step_r = this->calcStep(r, t_red);
    int step_g = this->calcStep(g, t_green);
    int step_b = this->calcStep(b, t_blue);
    
    int loop = 0;

    while (r != t_red or g != t_green or b != t_blue) {
        r = this->calcVal(r, t_red, step_r, loop);
        g = this->calcVal(g, t_green, step_g, loop);
        b = this->calcVal(b, t_blue, step_b, loop);
        this->setRGB(r,g,b);
        loop++;
        delayMicroseconds(1300);  
    }
}

int RgbStrip::calcStep(int start, int end) {
  int step = end - start; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 765 / step;              //   divide by 1020
  } 
  return step;
}

int RgbStrip::calcVal(int val, int target, int step, int loop) {
  if (val == target) {return val;}

  if ((step) && loop % step == 0) { // If step is non-zero and its time to change a value,
    (step > 0) ? val++ : val--;
  }

  return val;
}

int RgbStrip::getRed() {
  return this->red * this->bright / 100;
}

int RgbStrip::getGreen() {
  return this->green * this->bright / 100;
}

int RgbStrip::getBlue() {
  return this->blue * this->bright / 100;
}

const char* RgbStrip::getPower() {
  if (this->power) {
    return "on";
  } else {
    return "off";
  }
}

void RgbStrip::report() {
	char ret[256];
  sprintf(ret,
  "{\"cmd\": \"report\", \"model\": \"rgbstrip\", \"sid\": \"rgb01\", \"data\": {\"red\": %d, \"green\": %d, \"blue\": %d,\"bright\": %d , \"power\": \"%s\"}}",
  this->red, this->green, this->blue, this->bright, this->getPower());
	Serial.println(ret);
  // cout << ret << endl;
}

// int main(int argc, char** argv) 
// { 
//     if (argc < 4) {
//         return 1;
//     }
//     RgbStrip rgb;
    
//     int t_red = atoi(argv[1]);
//     int t_green = atoi(argv[2]);
//     int t_blue = atoi(argv[3]);
//     int _rgb = (t_red << 16) + (t_green << 8) + t_blue;
//     rgb.set_rgb(_rgb);
//     rgb.report();
//     rgb.set_bright(50);
//     rgb.report();
//     rgb.set_power(false);
//     rgb.report();
//     rgb.set_power(true);
//     rgb.report();
    
//     return 0;
// }
