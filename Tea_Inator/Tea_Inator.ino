#include <SPI.h>
#include <TFT_eSPI.h>
#include <TFT_eWidget>
#include <XPT2046_Touchscreen.h>

// Define Touchscreen Pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

// Create instances of touchscreen and SPI libraries
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Define Dimensions of touchscreen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

// Define possible states for FSM, and a global to track the current state
enum State{
  INIT,
  READY,
  HEAT,
  STEEP,
  DONE
};
enum State currentState = INIT;

ButtonWidget btnBlack = ButtonWidget(&tft);
ButtonWidget btnGreen = ButtonWidget(&tft);
ButtonWidget btnHerbal = ButtonWidget(&tft);
ButtonWidget btnOolong = ButtonWidget(&tft);
ButtonWidget btnManual = ButtonWidget(&tft);

#define BUTTON_W 50
#define BUTTON_H 50

//Keep track of last point user input on touchscreen
int Last_Touch_X = 0;
int Last_Touch_Y = 0;
int Last_Touch_Z = 0;

// Print Touchscreen info about X, Y and Pressure (Z) on the TFT Display
void printTouchToDisplay(int touchX, int touchY, int touchZ) {
  // Clear TFT screen
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  int centerX = SCREEN_WIDTH / 2;
  int textY = 80;
 
  String tempText = "X = " + String(touchX);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Y = " + String(touchY);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Pressure = " + String(touchZ);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);
}

void drawStartScreen(){
  // Clear the screen before writing to it
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

}

void drawReadyScreen(int steepTime, int steepTemp){

}

void initTouchScreen(){
  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(1);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);
}

void setup() {
  Serial.begin(115200);
  initTouchScreen();
  drawStartScreen();
}

void loop() {
  switch(currentState){
    case INIT:

    break;
    case READY:

    break;
    case HEAT:

    break;
    case STEEP:

    break;
    case DONE:

    break;
    default:

  }
  if(touchscreen.touched()){
    TS_Point p = touchscreen.getPoint();
    Last_Touch_X = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    Last_Touch_Y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    Last_Touch_Z = p.z;
    printTouchToDisplay(Last_Touch_X,Last_Touch_Y,Last_Touch_Z);

    delay(20);
  }
}
