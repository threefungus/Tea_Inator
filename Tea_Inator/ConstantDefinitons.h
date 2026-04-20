// Define Touchscreen Pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

// Define Ultrasonic Sensor Pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// Define speed of sound in cm/us for ultrasonic sensor distance calculation
#define SOUND_SPEED 0.0343 // Speed of sound in cm/us

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