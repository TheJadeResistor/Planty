#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <ezButton.h>
#include <dht.h>

#define TFT_CS 10
#define TFT_RST 8 
#define TFT_DC 9
#define TFT_BL 7
#define DHT11_PIN 3
#define photoLeft A0
#define photoBack A1
#define photoRight A2

#define photoThresh 50


Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
dht DHT;


const int SHORT_PRESS_TIME = 700; 
const int LONG_PRESS_TIME  = 700; 
const int LONG_LONG_PRESS_TIME  = 2000; 
ezButton button(2);  // create ezButton object that attach to pin 2;
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;



//https://www.instructables.com/TESTED-Timekeeping-on-ESP8266-Arduino-Uno-WITHOUT-/
unsigned long timeNow = 0;
unsigned long timeLast = 0;
//Time start Settings:
int startingHour = 0; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int seconds = 0;
int minutes = 0; //set starting minutes
int hours = startingHour;
int days = 0;
//Accuracy settings
int dailyErrorFast = 0; // set the average number of milliseconds your microcontroller's time is fast on a daily basis
int dailyErrorBehind = 0; // set the average number of milliseconds your microcontroller's time is behind on a daily basis
int correctedToday = 1; // do not change this variable, one means that the time has already been corrected today for the error in your boards crystal. This is true for the first day because you just set the time when you uploaded the sketch.  
char TEXTE[5];


void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Hello! - Planty")); //F passes from flash memory, doesnt use up RAM
  button.setDebounceTime(50);

  tft.init(240, 240);   // Init ST7789 240x240
  //tft.setSPISpeed(40000000);
  digitalWrite(TFT_BL, LOW); //does nothing it seems
  tft.fillScreen(ST77XX_BLACK);

  //pinMode(button, INPUT_PULLUP);
  pinMode(photoLeft, INPUT); //0-1023
  pinMode(photoBack, INPUT);
  pinMode(photoRight, INPUT);

  // tft.fillCircle(120, 60, 40, ST77XX_BLUE); //x(up),y(right),r
  // tft.fillCircle(190, 40, 40, ST77XX_BLACK);
  // // tft.fillCircle(110, 60, 25, ST77XX_WHITE);
  // // tft.fillCircle(100, 70, 10, ST77XX_BLACK);
  // tft.fillCircle(120, 180, 40, ST77XX_BLUE); //x,y,r
  // tft.fillCircle(190, 200, 40, ST77XX_BLACK);
}

void loop() {
  button.loop(); // MUST call the loop() function first
  if(button.isPressed()) pressedTime = millis();
  if(button.isReleased()) {
    releasedTime = millis();
    long pressDuration = releasedTime - pressedTime;
    if(pressDuration < SHORT_PRESS_TIME){
      Serial.println("Short press");
      minutes=0; hours=0; days=0; //reset timer
    }
    if(pressDuration > LONG_LONG_PRESS_TIME){
      Serial.println("Long Long press");
      //do long long press task
    }
    else if(pressDuration > LONG_PRESS_TIME){
      Serial.println("Long press");
      //do long press task
    }
  }

  timeKeep();
  photoBars(3);

  if(days >= 30){
    //dead face
    
  }
  else if(days >= 3){
    //need water face
    tft.fillCircle(120, 60, 40, ST77XX_BLUE); //x(up),y(right),r
    tft.fillCircle(190, 40, 40, ST77XX_BLACK);
    tft.fillCircle(120, 180, 40, ST77XX_BLUE); //x,y,r
    tft.fillCircle(190, 200, 40, ST77XX_BLACK);
  }
  else if(photoRead("L")>50 || photoRead("R")>50 || photoRead("B")>50){
    //happy sun face
    tft.fillCircle(120, 60, 40, ST77XX_YELLOW); //x(up),y(right),r
    tft.fillCircle(120, 180, 40, ST77XX_YELLOW); //x,y,r
  }
  else{// if(days < 3){
    tft.fillCircle(120, 60, 40, ST77XX_BLUE); //x(up),y(right),r
    tft.fillCircle(120, 180, 40, ST77XX_BLUE); //x,y,r
  }

  String(dhtRead("T")).toCharArray(TEXTE, 5);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setRotation(1);
  tft.setCursor((tft.width()/2)-35, 15);
  tft.write("Planty");
  tft.setCursor((tft.width()/2)-105, 35);
  tft.write("Temp: ");
  tft.setCursor((tft.width()/2)-45, 35); 
  tft.write(TEXTE);
  String(dhtRead("H")).toCharArray(TEXTE, 5);
  tft.setCursor((tft.width()/2)+15, 35); 
  tft.write("Hum: ");
  tft.setCursor((tft.width()/2)+60, 35); 
  tft.write(TEXTE);

  String(days).toCharArray(TEXTE, 5);
  tft.setCursor((tft.width()/2)-62, tft.height()-40);
  tft.write("Last water: ");
  tft.setCursor((tft.width()/2)-40, tft.height()-20);
  tft.write(TEXTE);
  tft.setCursor((tft.width()/2)-5, tft.height()-20);
  tft.write("days");
  tft.setRotation(0);

}


void drawEye(int x, int y, int r, bool inverse){
  if(!inverse)tft.fillCircle(x, y, r, ST77XX_BLUE); //x,y,r
  else tft.fillCircle(x, y, r, ST77XX_BLACK);
  //
}

int photoRead(String x){ //0=left, 1=right, 2=back
  if(x == "left" || x == "L")
    return (int)map(analogRead(photoLeft), 0, 1023, 0, 100);
  if(x == "right" || x == "R")
    return (int)map(analogRead(photoRight), 0, 1023, 0, 100);
  if(x == "back" || x == "B")
    return (int)map(analogRead(photoBack), 0, 1023, 0, 100);
}

double dhtRead(String x){
  DHT.read11(DHT11_PIN);
  if(x == "temp" || x == "T")
    return (double)DHT.temperature;
  if(x == "hum" || x == "H")
    return (double)DHT.humidity;
}

void photoBars(int barWidth){
  if(photoRead("R") > photoThresh){
    //Serial.println("Left Photo Bright");
    tft.fillRect(0, 0, tft.width(), barWidth, ST77XX_YELLOW);
  }
  else
    tft.fillRect(0, 0, tft.width(), barWidth, ST77XX_BLACK);
  if(photoRead("B") > photoThresh){
    //Serial.println("Right Photo Bright");
    tft.fillRect(tft.width()-barWidth, 0,  barWidth, tft.height(), ST77XX_YELLOW);
  }
  else
    tft.fillRect(tft.width()-barWidth, 0,  barWidth, tft.height(), ST77XX_BLACK);
  if(photoRead("L") > photoThresh){
    //Serial.println("Back Photo Bright");
    tft.fillRect(0, tft.height()-barWidth, tft.width(), barWidth, ST77XX_YELLOW);
  }
  else
    tft.fillRect(0, tft.height()-barWidth, tft.width(), barWidth, ST77XX_BLACK);
}

void timeKeep(){
  timeNow = millis()/1000; // the number of milliseconds that have passed since boot
  seconds = timeNow - timeLast;//the number of seconds that have passed since the last time 60 seconds was reached.

  if (seconds == 60) {
    timeLast = timeNow;
    minutes = minutes + 1;
  }
  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes == 60){ 
    minutes = 0;
    hours = hours + 1;
  }
  // if one hour has passed, start counting minutes from zero and add one hour to the clock
  if (hours == 24){
    hours = 0;
    days = days + 1;
    }

  //if 24 hours have passed , add one day
  if (hours ==(24 - startingHour) && correctedToday == 0){
    delay(dailyErrorFast*1000);
    seconds = seconds + dailyErrorBehind;
    correctedToday = 1;
  }
  //every time 24 hours have passed since the initial starting time and it has not been reset this day before, add milliseconds or delay the progran with some milliseconds. 
  //Change these varialbes according to the error of your board. 
  // The only way to find out how far off your boards internal clock is, is by uploading this sketch at exactly the same time as the real time, letting it run for a few days 
  // and then determine how many seconds slow/fast your boards internal clock is on a daily average. (24 hours).
  if (hours == 24 - startingHour + 2) { 
    correctedToday = 0;
  }
  //let the sketch know that a new day has started for what concerns correction, if this line was not here the arduiono
  // would continue to correct for an entire hour that is 24 - startingHour. 
}

//   Serial.print("The time is:           ");
//   Serial.print(days);
//   Serial.print(":");
//   Serial.print(hours);
//   Serial.print(":");
//   Serial.print(minutes);
//   Serial.print(":"); 
//   Serial.println(seconds); 



// ST77XX_MAGENTA
// tft.fillScreen(BLACK);
// tft.setCursor(0, 0);
// tft.fillScreen(ST77XX_BLACK);
// tft.setTextColor(ST77XX_WHITE);
// tft.setTextSize(0);
// tft.drawRoundRect(x, y, w, h, 5, color);
// tft.drawTriangle(w, y, y, x, z, x, color);
// tft.drawCircle(x, y, radius, color);
// tft.fillCircle(x, y, radius, color);
// tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
// tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
// tft.width();
// tft.height();
// tft.invertDisplay(true);