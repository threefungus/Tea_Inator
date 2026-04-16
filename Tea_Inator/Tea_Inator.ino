#include <SPI.h>
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>
#include <XPT2046_Touchscreen.h>

// Define Touchscreen Pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

// Define Dimensions of touchscreen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUTTON_FONT 2
#define FONT_SIZE 1

#define BUTTON_W 80
#define BUTTON_H 80

// Create instances of touchscreen and SPI libraries
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

//Define Tea Steeping Parameters
#define BLACK_TIME 300
#define BLACK_TEMP 212
#define GREEN_TIME 120
#define GREEN_TEMP 175
#define HERBAL_TIME 300
#define HERBAL_TEMP 212
#define OOLONG_TIME 150
#define OOLONG_TEMP 196
#define WHITE_TIME 180
#define WHITE_TEMP 175

// Define possible states for FSM, and a global to track the current state
enum State{
  INIT,
  READY,
  HEAT,
  STEEP,
  DONE
};
enum State currentState = INIT;

// Define Button instances
ButtonWidget btnBlack = ButtonWidget(&tft);
ButtonWidget btnGreen = ButtonWidget(&tft);
ButtonWidget btnHerbal = ButtonWidget(&tft);
ButtonWidget btnOolong = ButtonWidget(&tft);
ButtonWidget btnWhite = ButtonWidget(&tft);
ButtonWidget btnManual = ButtonWidget(&tft);

ButtonWidget btnBack = ButtonWidget(&tft);
ButtonWidget plusTemp = ButtonWidget(&tft);
ButtonWidget minusTemp = ButtonWidget(&tft);
ButtonWidget plusTime = ButtonWidget(&tft);
ButtonWidget minusTime = ButtonWidget(&tft);



ButtonWidget* teaBtn[] = {&btnBlack, &btnGreen, &btnHerbal, &btnOolong, &btnWhite, &btnManual};;
uint8_t teaBtnCnt = sizeof(teaBtn) / sizeof(teaBtn[0]);

ButtonWidget* menuTwoBtn[] = {&btnBack, &plusTemp, &minusTemp, &plusTime, &minusTime};;
uint8_t menuTwoCnt = sizeof(menuTwoBtn) / sizeof(menuTwoBtn[0]);

//Keep track of last point user input on touchscreen
int Last_Touch_X = 0;
int Last_Touch_Y = 0;
int Last_Touch_Z = 0;

// integer value in seconds for default steep time length
int steep_Time = 0;
// integer value of steeping temperature (Fahrenheit)
int steep_Temp = 0;

void btnBlack_pressAction(){
  Serial.print("Black Pressed");
  steep_Time = BLACK_TIME;
  steep_Temp = BLACK_TEMP;
  drawReadyScreen("Black Tea");
}

void btnGreen_pressAction(){
  Serial.print("Green Pressed");
  steep_Time = GREEN_TIME;
  steep_Temp = GREEN_TEMP;
  drawReadyScreen("Green Tea");
}

void btnHerbal_pressAction(){
  Serial.print("Herbal Pressed");
  steep_Time = HERBAL_TIME;
  steep_Temp = HERBAL_TEMP;
  drawReadyScreen("Herbal Tea");
}

void btnOolong_pressAction(){
  Serial.print("Oolong Pressed");
  steep_Time = OOLONG_TIME;
  steep_Temp = OOLONG_TEMP;
  drawReadyScreen("Oolong Tea");
}

void btnWhite_pressAction(){
  Serial.print("White Pressed");
  steep_Time = WHITE_TIME;
  steep_Temp = WHITE_TEMP;
  drawReadyScreen("White Tea");
}

void btnManual_pressAction(){
  Serial.print("Manual Pressed");
  steep_Time = BLACK_TIME;
  steep_Temp = BLACK_TEMP;
  drawReadyScreen("Custom Tea");
}

void btnBack_pressAction(){
  currentState = INIT;
  drawStartScreen();
}

void plusTemp_pressAction(){
  steep_Temp++;
  drawReadyScreen;
}

void minusTemp_pressAction(){
  steep_Temp--;
  drawReadyScreen;
}

void plusTime_pressAction(){
  steep_Time++;
  drawReadyScreen;
}

void minusTime_pressAction(){
  steep_Time--;
  drawReadyScreen;
}


// Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
void printTouchToSerial(int touchX, int touchY, int touchZ) {
  Serial.print("X = ");
  Serial.print(touchX);
  Serial.print(" | Y = ");
  Serial.print(touchY);
  Serial.print(" | Pressure = ");
  Serial.print(touchZ);
  Serial.println();
}

/** 
 *  Print Touchscreen info about X, Y and Pressure (Z) on the TFT Display
*/
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

/**
 * Draw the start screen on display for the idle state, clearing the screen and
 * placing tea variety buttons
 */
void drawStartScreen(){
  // Clear the screen before writing to it
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  // Set the placement position of the button on the screen
  uint16_t spaceX = ((SCREEN_WIDTH / 3) - BUTTON_W) / 2;
  uint16_t spaceY = ((SCREEN_HEIGHT / 2) - BUTTON_H) / 2;
  uint16_t butX = spaceX;
  uint16_t butY = spaceY;
  //Instantiate the buttons, and assign press action methods and release action methods
  // Very poor documentation on these methods, I assume this one is ->
  // initButtonUL(target tft, xCord, yCord, Width, Height, Border Color, Fill Color, Text Color, Label, text font)
  btnBlack.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "Black", BUTTON_FONT);
  btnBlack.setPressAction(btnBlack_pressAction);
  btnBlack.drawButton();

  butX = (SCREEN_WIDTH / 3) + spaceX;
  btnGreen.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "Green", BUTTON_FONT);
  btnGreen.setPressAction(btnGreen_pressAction);
  btnGreen.drawButton();

  butX = (SCREEN_WIDTH / 3) * 2 + spaceX;
  btnHerbal.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "Herbal", BUTTON_FONT);
  btnHerbal.setPressAction(btnHerbal_pressAction);
  btnHerbal.drawButton();

  butX = spaceX;
  butY = (SCREEN_HEIGHT / 2) + spaceY;
  btnOolong.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "Oolong", BUTTON_FONT);
  btnOolong.setPressAction(btnOolong_pressAction);
  btnOolong.drawButton();

  butX = (SCREEN_WIDTH / 3) + spaceX;
  btnWhite.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "White", BUTTON_FONT);
  btnWhite.setPressAction(btnWhite_pressAction);
  btnWhite.drawButton();

  butX = (SCREEN_WIDTH / 3) * 2 + spaceX;
  btnManual.initButtonUL(butX, butY, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_GREENYELLOW, TFT_BLACK, "Manual", BUTTON_FONT);
  btnManual.setPressAction(btnManual_pressAction);
  btnManual.drawButton();
}

/**
 * Draw ready screen on display, starting with a given time and temp for steeping
 */
void drawReadyScreen(char* teaType){
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(FONT_SIZE);
  int centerX = SCREEN_WIDTH / 2;
  int textY = 60;
  
  tft.drawString(teaType, 5, 5, 2);

  String tempText = "Steeping Time: " + String(steep_Time / 60) +"min " + String(steep_Time % 60) +"sec";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  plusTime.initButtonUL(SCREEN_WIDTH-75, textY-15, BUTTON_W/3, BUTTON_H/3, TFT_BLACK, TFT_WHITE, TFT_BLACK, "+", BUTTON_FONT);
  plusTime.setPressAction(plusTime_pressAction);
  plusTime.drawButton();

  minusTime.initButtonUL(SCREEN_WIDTH-75, textY+15, BUTTON_W/3, BUTTON_H/3, TFT_BLACK, TFT_WHITE, TFT_BLACK, "-", BUTTON_FONT);
  minusTime.setPressAction(minusTime_pressAction);
  minusTime.drawButton();

  tft.setTextSize(FONT_SIZE);
  textY += 80;
  tempText = "Steeping Temperature: " + String(steep_Temp) +" F";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  plusTemp.initButtonUL(SCREEN_WIDTH-75, textY-15, BUTTON_W/3, BUTTON_H/3, TFT_BLACK, TFT_WHITE, TFT_BLACK, "+", BUTTON_FONT);
  plusTemp.setPressAction(plusTemp_pressAction);
  plusTemp.drawButton();

  minusTemp.initButtonUL(SCREEN_WIDTH-75, textY+15, BUTTON_W/3, BUTTON_H/3, TFT_BLACK, TFT_WHITE, TFT_BLACK, "-", BUTTON_FONT);
  minusTemp.setPressAction(minusTemp_pressAction);
  minusTemp.drawButton();

  btnBack.initButtonUL(SCREEN_WIDTH-65, SCREEN_HEIGHT-45, BUTTON_W-20, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_RED, "Back", BUTTON_FONT);
  btnBack.setPressAction(btnBack_pressAction);
  btnBack.drawButton();  
  currentState = READY;
}

/**
 * Initialize the touch screen using the SPI library, setting both the display and touch
 * input rotations
 */
void initTouchScreen(){
  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(3);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);
}

void setup() {
  Serial.begin(115200);
  initTouchScreen();
  touch_calibrate();
  drawStartScreen();
}

void loop() {
  // static uint32_t scanTime = millis();
  // uint16_t touch_x = 9999, touch_y = 9999;
  bool pressed = false;
  if(touchscreen.touched()){
    TS_Point p = touchscreen.getPoint();
    Last_Touch_X = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    Last_Touch_Y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    Last_Touch_Z = p.z;
    printTouchToSerial(Last_Touch_X,Last_Touch_Y,Last_Touch_Z);
    pressed = true;

    delay(50);
  }
  else{
    pressed = false;
  }
  switch(currentState){
    case INIT:
      for(uint8_t b = 0; b < teaBtnCnt; b++){
        if(pressed){
          if(teaBtn[b]->contains(Last_Touch_X,Last_Touch_Y)){
            teaBtn[b]->press(true);
            teaBtn[b]->pressAction();
          }
        }
        else{
          teaBtn[b]->press(false);
        }
      }
    break;
    case READY:
      for(uint8_t b = 0; b < menuTwoCnt; b++){
        if(pressed){
          if(menuTwoBtn[b]->contains(Last_Touch_X,Last_Touch_Y)){
            menuTwoBtn[b]->press(true);
            menuTwoBtn[b]->pressAction();
          }
        }
        else{
          menuTwoBtn[b]->press(false);
        }
      }
    break;
    case HEAT:

    break;
    case STEEP:

    break;
    case DONE:

    break;
    default:
    break;
  }
  
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}