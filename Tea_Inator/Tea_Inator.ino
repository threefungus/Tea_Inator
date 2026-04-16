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

// Define calibration file and whether to repeat calibration on each boot
#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

// Define Dimensions of touchscreen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUTTON_FONT 2
#define FONT_SIZE 1
// Define dimensions of buttons on the screen
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

// Define Button instances for first menu, one for each tea variety
ButtonWidget btnBlack = ButtonWidget(&tft);
ButtonWidget btnGreen = ButtonWidget(&tft);
ButtonWidget btnHerbal = ButtonWidget(&tft);
ButtonWidget btnOolong = ButtonWidget(&tft);
ButtonWidget btnWhite = ButtonWidget(&tft);
ButtonWidget btnManual = ButtonWidget(&tft);

// Buttons for second menu
ButtonWidget btnBack = ButtonWidget(&tft);
ButtonWidget plusTemp = ButtonWidget(&tft);
ButtonWidget minusTemp = ButtonWidget(&tft);
ButtonWidget plusTime = ButtonWidget(&tft);
ButtonWidget minusTime = ButtonWidget(&tft);


// Array to hold pointers to first menu buttons for easier access in the FSM
ButtonWidget* teaBtn[] = {&btnBlack, &btnGreen, &btnHerbal, &btnOolong, &btnWhite, &btnManual};;
// Calculate the number of buttons in the first menu for use in loops
uint8_t teaBtnCnt = sizeof(teaBtn) / sizeof(teaBtn[0]);

// Array to hold pointers to second menu buttons for easier access in the FSM
ButtonWidget* menuTwoBtn[] = {&btnBack, &plusTemp, &minusTemp, &plusTime, &minusTime};;
// Calculate the number of buttons in the second menu for use in loops
uint8_t menuTwoCnt = sizeof(menuTwoBtn) / sizeof(menuTwoBtn[0]);

//Keep track of last point user input on touchscreen
int Last_Touch_X = 0;
int Last_Touch_Y = 0;
int Last_Touch_Z = 0;

// integer value in seconds for default steep time length
int steep_Time = 0;
// integer value of steeping temperature (Fahrenheit)
int steep_Temp = 0;
// String to hold the type of tea being steeped for display purposes
char* teaType = "";

// Action methods for tea variety buttons, setting the steeping time and temperature to the 
// default values for the respective tea variety and drawing the ready screen with those values
void btnBlack_pressAction(){
  Serial.print("Black Pressed");
  steep_Time = BLACK_TIME;
  steep_Temp = BLACK_TEMP;
  teaType = "Black Tea";
  drawReadyScreen();
}
void btnGreen_pressAction(){
  Serial.print("Green Pressed");
  steep_Time = GREEN_TIME;
  steep_Temp = GREEN_TEMP;
  teaType = "Green Tea";
  drawReadyScreen();
}
void btnHerbal_pressAction(){
  Serial.print("Herbal Pressed");
  steep_Time = HERBAL_TIME;
  steep_Temp = HERBAL_TEMP;
  teaType = "Herbal Tea";
  drawReadyScreen();
}
void btnOolong_pressAction(){
  Serial.print("Oolong Pressed");
  steep_Time = OOLONG_TIME;
  steep_Temp = OOLONG_TEMP;
  teaType = "Oolong Tea";
  drawReadyScreen();
}
void btnWhite_pressAction(){
  Serial.print("White Pressed");
  steep_Time = WHITE_TIME;
  steep_Temp = WHITE_TEMP;
  teaType = "White Tea";
  drawReadyScreen();
}
void btnManual_pressAction(){
  Serial.print("Manual Pressed");
  steep_Time = BLACK_TIME;
  steep_Temp = BLACK_TEMP;
  teaType = "Custom Tea";
  drawReadyScreen();
}

// Action method for back button, resetting to initial state and redrawing start screen
void btnBack_pressAction(){
  currentState = INIT;
  drawStartScreen();
}

// Action methods for plus and minus buttons on steeping time and temperature, incrementing or 
// decrementing the respective variable and redrawing the ready screen with updated values
void plusTemp_pressAction(){
  Serial.print("Plus Temp Pressed");
  steep_Temp++;
  if(steep_Temp > 212){
    steep_Temp = 212;
  }
  updateTemp();
}
void minusTemp_pressAction(){
  Serial.print("Minus Temp Pressed");
  steep_Temp--;
  if(steep_Temp < 100){
    steep_Temp = 100;
  }
  updateTemp();
}
void plusTime_pressAction(){
  Serial.print("Plus Time Pressed");
  steep_Time++;
  updateTime();
}
void minusTime_pressAction(){
  Serial.print("Minus Time Pressed");
  steep_Time--;
  if(steep_Time < 0){
    steep_Time = 0;
  }
  updateTime();
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
void drawReadyScreen(){
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(FONT_SIZE);
  int centerX = SCREEN_WIDTH / 2;
  int textY = 60;
  
  tft.drawString(teaType, 5, 5, 2);

  String tempText = "Steeping Time: " + String(steep_Time / 60) +"min " + String(steep_Time % 60) +"sec";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  plusTime.initButtonUL(SCREEN_WIDTH-50, textY-40, BUTTON_W/2, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_BLACK, "+", BUTTON_FONT);
  plusTime.setPressAction(plusTime_pressAction);
  plusTime.drawButton();

  minusTime.initButtonUL(SCREEN_WIDTH-50, textY+5, BUTTON_W/2, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_BLACK, "-", BUTTON_FONT);
  minusTime.setPressAction(minusTime_pressAction);
  minusTime.drawButton();

  tft.setTextSize(FONT_SIZE);
  textY += 80;
  tempText = "Steeping Temperature: " + String(steep_Temp) +" F";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  plusTemp.initButtonUL(SCREEN_WIDTH-50, textY-30, BUTTON_W/2, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_BLACK, "+", BUTTON_FONT);
  plusTemp.setPressAction(plusTemp_pressAction);
  plusTemp.drawButton();

  minusTemp.initButtonUL(SCREEN_WIDTH-50, textY+15, BUTTON_W/2, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_BLACK, "-", BUTTON_FONT);
  minusTemp.setPressAction(minusTemp_pressAction);
  minusTemp.drawButton();

  btnBack.initButtonUL(5, SCREEN_HEIGHT-45, BUTTON_W-20, BUTTON_H/2, TFT_BLACK, TFT_WHITE, TFT_RED, "Back", BUTTON_FONT);
  btnBack.setPressAction(btnBack_pressAction);
  btnBack.drawButton();  
  currentState = READY;
}

// Update the steeping time displayed on the ready screen, called from the plus and minus time buttons
void updateTime(){
  int centerX = SCREEN_WIDTH / 2;
  int textY = 60;
  tft.fillRect(80, textY-20, SCREEN_WIDTH - 160, 40, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(FONT_SIZE);
  String tempText = "Steeping Time: " + String(steep_Time / 60) +"min " + String(steep_Time % 60) +"sec";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);
}
// Update the steeping temperature displayed on the ready screen, called from the plus and minus temp buttons
void updateTemp(){
  int centerX = SCREEN_WIDTH / 2;
  int textY = 140;
  tft.fillRect(80, textY-20, SCREEN_WIDTH - 160, 40, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(FONT_SIZE);
  String tempText = "Steeping Temperature: " + String(steep_Temp) +" F";
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);
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
  // Check if the screen is being touched, and if so get the X, Y and Z (pressure) values of the touch and print to serial
  bool pressed = false;
  if(touchscreen.touched()){
    TS_Point p = touchscreen.getPoint();
    Last_Touch_X = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    Last_Touch_Y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    Last_Touch_Z = p.z;
    printTouchToSerial(Last_Touch_X,Last_Touch_Y,Last_Touch_Z);
    pressed = true;
    // Small delay to prevent multiple presses being registered from a single touch
    delay(50);
  }
  else{
    pressed = false;
  }

  //Begin FSM
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

/**
 * Calibrate the touchscreen, checking for existing calibration data in the file system and using it if valid, 
 * otherwise running the calibration routine and saving the new calibration data to the file system
 */
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