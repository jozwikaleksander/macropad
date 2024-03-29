#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "SPI.h"

#include <Keypad.h>
#include "HID-Project.h"

#include <config.h>

#define cs   2
#define dc   0
#define rst  1

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

// PINS

/*
  0 - DC
  1 - RESET
  2 - CS
  6 - 1 KEYPAD
  7 - 2 KEYPAD
  8 - 3 KEYPAD
  9 - 4 KEYPAD
  10 - 5 KEYPAD
  11 - 6 KEYPAD
  12 - 7 KEYPAD
  13 - 8 KEYPAD
*/

// KEYPAD VARIABLES

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// --------------------

// Muted or unmuted
bool muted = false;

// After pressing mute button color needs to be changed. This variable insures that it will by only changed on time.
bool screenUpdated = false;

bool notificationActive = false;

// --------------------

// SCREEN NOTIFICATION SETTINGS
int maxLineLength = 22;


// HELP MENU VARIABLES

// Help menu active or not
bool help = false;

// Help menu shortcuts
// LAYOUT > PAGE > SHORTCUT
String shortcuts[4][2][8] = {
  {
    {"1 - mute / unmute","* - help menu","4 - next layout", "5 - previous layout", "A - Spotify","B - Terminal","C - Code","D - Discord"},
    {"# - Monkeytype","7 - volume up","8 - volume down","9 - play / pause"}
  },
  {
    {"2 - text bold","A - # header","B - ## header", "C - ### header", "D - #### header", "3 - export to HTML"}
  },
  {
    {"2 - sleep", "3 - shutdown"}
  },
  {
    {"1 - normal session (25 min)", "2 - break time (5 min)"}
  }
};
// Current help screen layout
int currentHelpIndex = 0;

// --------------------

// Layouts variables
int layoutIndex = 0;
int layoutsLength = 4;
String layoutNames[4] = {"General mode","Markdown mode","Power management","Pomodoro mode"};
// --------------------

// Function for finding perfect x position to center an object
int findCenter(int length, int width = 6){
  return int((160-(length*width))/2);
}

// Function for converting colors from RGB to bytes
word ConvertRGB( byte R, byte G, byte B)
{
  return ( ((B & 0xF8) << 8) | ((G & 0xFC) << 3) | (R >> 3) );
}

// Color palette

word backgroundColor = ConvertRGB(42, 42, 56);
word whiteColor = ConvertRGB(255,255,255);
word yellowColor = ConvertRGB(255,150,0);
word magentaColor = ConvertRGB(116, 73, 132);
word blueColor = ConvertRGB(100, 157, 247);
word greenColor = ConvertRGB(61, 146, 111);
word redColor = ConvertRGB(212, 87, 101);

// Timer

int lessonTime = 25;
int breakTime = 5;
int currentSessionTime = lessonTime;
unsigned long startMillis = 0;
unsigned long currentMillis = 0;

int minutes = 0;
int seconds = 0;

bool timerStarted = false;

// -----------------
// CORNERS

const unsigned char leftDownCorner [] PROGMEM = {
	0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x20, 0x00, 0x20, 0x00, 
	0x20, 0x00, 0x10, 0x00, 0x08, 0x00, 0x06, 0x00, 0x01, 0xe0, 0x00, 0x1c
};
const unsigned char leftTopCorner [] PROGMEM = {
	0x00, 0x1c, 0x00, 0xe0, 0x07, 0x00, 0x08, 0x00, 0x10, 0x00, 0x20, 0x00, 0x20, 0x00, 0x40, 0x00, 
	0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
};
const unsigned char rightTopCorner [] PROGMEM = {
	0xe0, 0x00, 0x1c, 0x00, 0x03, 0x80, 0x00, 0x40, 0x00, 0x20, 0x00, 0x10, 0x00, 0x10, 0x00, 0x08, 
	0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04
};
const unsigned char rightDownCorner [] PROGMEM = {
	0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x10, 
	0x00, 0x10, 0x00, 0x20, 0x00, 0x40, 0x03, 0x80, 0x1c, 0x00, 0xe0, 0x00
};

// -----------------

// ICONS

// Unmuted icon
// 'unmuted', 10x14px
const unsigned char unmuteIcon [28] PROGMEM = {
	0x0c, 0x00, 0x1e, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0xbf, 0x40, 0xff, 0xc0, 
	0x5e, 0x80, 0x21, 0x00, 0x1e, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x1e, 0x00
};

// Muted icon
// 'muted', 10x14px
const unsigned char muteIcon [28] PROGMEM = {
	0x0c, 0x40, 0x1e, 0x80, 0x3e, 0x80, 0x3d, 0x00, 0x3a, 0x00, 0x3a, 0x00, 0xb5, 0x40, 0xeb, 0xc0, 
	0x56, 0x80, 0x11, 0x00, 0x2e, 0x00, 0x4c, 0x00, 0x4c, 0x00, 0x9e, 0x00
};

// Clock icon
// 'clock, 16x16px
const unsigned char clockIcon [32] PROGMEM = {
	0x00, 0x00, 0x03, 0xc0, 0x0c, 0x30, 0x10, 0x08, 0x20, 0x84, 0x20, 0x84, 0x40, 0x82, 0x40, 0x82, 
	0x4f, 0x82, 0x40, 0x02, 0x20, 0x04, 0x20, 0x04, 0x10, 0x08, 0x0c, 0x30, 0x03, 0xc0, 0x00, 0x00
};
// ------------------------

// Function for drawing microphone icons
void drawIcon(int x, int y, bool muted = false){
  if(muted){
    tft.drawBitmap(x, y, muteIcon, 10, 14, redColor);
  }
  else{
    tft.drawBitmap(x, y, unmuteIcon, 10, 14, greenColor);
  }
}
// ------------------------

// Function for wrapping text
void wrapText(const char text[],int x,int y){
  tft.setCursor(x,y);

  for(unsigned int i = 0; i < strlen(text);i++){
    if((i+1) % 22 != 0){
      tft.print(text[i]);
    }
    else{
      tft.println("");
      tft.setCursor(x,tft.getCursorY()+2);
      tft.print(text[i]);
    }
  }
}

// Function for drawing corners
void drawBackground(){
  tft.fillScreen(backgroundColor);
  tft.drawRect(5, 5, 150, 118,magentaColor);
  tft.drawBitmap(5,5,leftTopCorner,14,14,magentaColor);
  tft.drawBitmap(141,5,rightTopCorner,14,14,magentaColor);
  tft.drawBitmap(5,109,leftDownCorner,14,14,magentaColor);
  tft.drawBitmap(141,109,rightDownCorner,14,14,magentaColor);
}

// Drawing main menu
void drawMainMenu(bool muted, int currentScreen = 0){
  drawBackground();

  if(currentScreen == 0){
    // Muted / unmuted prompt
    if(muted){
      drawIcon(findCenter(1,10),30,true);

      tft.setTextColor(whiteColor);
      tft.setTextSize(2);
      tft.setCursor(findCenter(5,12), 50);
      tft.print("Muted");

      tft.setTextColor(yellowColor);
      tft.setTextSize(1);
      tft.setCursor(findCenter(17), 80);
      tft.print("Press 1 to unmute");
    }
    else{
      drawIcon(findCenter(1,10),30);

      tft.setTextColor(whiteColor);
      tft.setTextSize(2);
      tft.setCursor(findCenter(7,12), 50);
      tft.print("Unmuted");

      tft.setTextColor(yellowColor);
      tft.setTextSize(1);
      tft.setCursor(findCenter(15), 80);
      tft.print("Press 1 to mute");
    }

    // Help menu
    tft.setTextColor(yellowColor);
    tft.setCursor(findCenter(16), 20);
    tft.print("Press * for help");

    // Layout name
    tft.setTextColor(blueColor);
    tft.setCursor(findCenter(layoutNames[layoutIndex].length()),100);
    tft.print((layoutNames[layoutIndex]).c_str());
  }
  else if(currentScreen == 1){
    // Help menu title
    tft.setTextColor(yellowColor);
    tft.setTextSize(1);
    tft.setCursor(findCenter((layoutNames[layoutIndex]).length()),10);
    tft.print((layoutNames[layoutIndex]).c_str());

    // Help menu entries
    tft.setTextColor(whiteColor);
    tft.setTextSize(1);

    int offset = 0;

    for(int i = 0; i < 8; i++){

      tft.setCursor(13,30 + (i*10)+offset);

      char shortcut[shortcuts[layoutIndex][currentHelpIndex][i].length() + 1];
      strcpy(shortcut, shortcuts[layoutIndex][currentHelpIndex][i].c_str());

      if(strlen(shortcut) > 22){
        wrapText(shortcut,13,30);
        offset = 10;
      }
      else{
        tft.print(shortcut);
        offset = 0;        
      }
    }

    // Page number
    tft.setTextColor(blueColor);
    tft.setCursor(findCenter(11),110);
    tft.print(("Page "+String(currentHelpIndex+1)+" of 2").c_str());
  }
  else if(currentScreen == 2){
    tft.drawBitmap(findCenter(14,1), 10, clockIcon, 16, 16, greenColor);
    tft.setTextColor(redColor);
    tft.setTextSize(1);
    if(timerStarted){
      tft.setCursor(findCenter(9),30);
      tft.print("Time left");
    }
    else{
      tft.setCursor(findCenter(16),30);
      tft.print("Press 1 to start");
    }
    tft.setTextColor(yellowColor);
    tft.setTextSize(2);
    tft.setCursor(findCenter(5,12),50);
    tft.print((String(currentSessionTime - minutes)+" min").c_str());
    tft.drawRect(findCenter(1,100),75,100,15,magentaColor);

    Serial.println((2.0/25.0)*94.0);

    tft.fillRect(findCenter(1,100)+3,78,int((float(minutes)/float(currentSessionTime))*94.0),9,yellowColor);

    // Layout name
    tft.setTextColor(blueColor);
    tft.setTextSize(1);
    tft.setCursor(findCenter(layoutNames[layoutIndex].length()),100);
    tft.print((layoutNames[layoutIndex]).c_str());
  }
}

// Function for launching programs from Windows's Start Menu
void launchFromStartMenu(String program){
  Keyboard.press(KEY_LEFT_GUI);
  delay(1000);  
  Keyboard.releaseAll();
  delay(200);
  Keyboard.println(program);
  delay(100);
  Keyboard.press(KEY_RETURN);
  delay(100);
  Keyboard.releaseAll();
}

// Displaying notification
void displayNotification(const char text[]){
  // Max amount of character in one line is 22
  drawBackground();

  tft.setTextColor(yellowColor);
  tft.setTextSize(1);

  tft.setCursor(findCenter(7),10);
  tft.print("Message");

  tft.setTextWrap(false);

  tft.setTextColor(whiteColor);
  
  if(strlen(text) > 22){
    wrapText(text,13,30);
  }
  else{
    tft.setCursor(13, 20);
    tft.print(text);
  }

  tft.setTextColor(yellowColor);
  tft.setTextSize(1);
  tft.setCursor(findCenter(16),103);
  tft.print("Press 1 to close");
}

// Function for sending notification if not sent already
void sendNotification(char const text[]){

  if(notificationActive == false){
    displayNotification(text);
    notificationActive = true;
    delay(500);
  }
}

// Pomodro timer funciton
void pomodoroTimer() {
  seconds = int((millis() - startMillis) / 1000.0);

  if(seconds % 60 == 0 && seconds > 0){
    minutes++;
    if(minutes >= currentSessionTime){
      if(currentSessionTime == lessonTime){
        currentSessionTime = breakTime;
      }
      else{
        currentSessionTime = lessonTime;
      }
      timerStarted = false;
      minutes = 0;

      sendNotification("Times up!");
    }
    else{
      startMillis = millis();
      seconds = 0;
      screenUpdated=false;
    }
  }
}

void setup(){
  Serial.begin(9600);
  Keyboard.begin();
  Consumer.begin();
  System.begin();

  tft.initR(INITR_GREENTAB);
  tft.setRotation(1);
  tft.setTextWrap(false);

  drawBackground();
}
  
void loop(){
  char customKey = customKeypad.getKey();

  if(Serial.available()){
    String inputStr = Serial.readStringUntil('\n');
    char input[inputStr.length()+1];
    inputStr.toCharArray(input, inputStr.length()+1);
    
    sendNotification(input);
  }

  // If notification is currently displayed
  if(notificationActive && customKey == '1'){
    screenUpdated = false;
    notificationActive = false;
    delay(100);
  }
  // If help menu is activated
  else if(help){
    // If screen needs to be updated
    if(screenUpdated == false){
      Serial.println("Get help");
      drawMainMenu(muted,1);
      screenUpdated = true;
    }
    // HELP MENU BINDS
    if(customKey == '5'){
      Serial.println(currentHelpIndex);
      if(currentHelpIndex == 1){
        currentHelpIndex = 0;
      }
      else{
        currentHelpIndex++;
      }
      screenUpdated = false;
    }
    else if(customKey == '4'){
      Serial.println(currentHelpIndex);
      if(currentHelpIndex == 0){
        currentHelpIndex = 1;
      }
      else{
        currentHelpIndex--;
      }
      screenUpdated = false;
    }
  }
  else{

    if(screenUpdated == false){
      if(layoutIndex == 3){
        drawMainMenu(muted,2);
        screenUpdated = true;
      }
      else{
        drawMainMenu(muted,0);
        screenUpdated = true;
      }
    }

    // Changing layouts
    if(customKey == '5'){
      if(layoutIndex == layoutsLength-1){
        layoutIndex = 0;
      }
      else{
        layoutIndex++;
      }
      currentHelpIndex = 0;
      screenUpdated = false;
      delay(100);
    }
    if(customKey == '4'){
      if(layoutIndex == 0){
        layoutIndex = layoutsLength-1;
      }
      else{
        layoutIndex--;
      }
      currentHelpIndex = 0;
      screenUpdated = false;
      delay(100);
    }
    // ----------------------

    // LAYOUT INDEPENDENT KEY BINDINGS 
    if (customKey == '1' && layoutIndex != 3){ // Muting
        muted = !muted;
        screenUpdated = false;
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.write('/');
        delay(100);
        Keyboard.releaseAll();
    }

    // ----------------------

    // LAYOUTS

    // GENERAL MODE
    if(layoutIndex == 0){
      if(customKey == 'D'){
        launchFromStartMenu("discord");
      }
      else if(customKey == 'C'){
        launchFromStartMenu("code");
      }
      else if(customKey == 'B'){
        launchFromStartMenu("terminal");
      }
      else if(customKey == 'A'){
        launchFromStartMenu("spotify");
      }
      else if(customKey == '#'){
        launchFromStartMenu("monkeytype");
      }
      else if(customKey == '7'){
        Consumer.write(MEDIA_VOLUME_UP);
        delay(100);
      }
      else if(customKey == '8'){
        Consumer.write(MEDIA_VOLUME_DOWN);
        delay(100);
      }
      else if(customKey == '9'){
        Consumer.write(MEDIA_PLAY_PAUSE);
        delay(100);
      }
    }
    // MARKDOWN MODE (for taking notes in markdown)
    else if(layoutIndex == 1){
      if(customKey == '2'){
        Keyboard.print("**");
      }
      else if(customKey == 'A'){
        Keyboard.print("# ");
      }
      else if(customKey == 'B'){
        Keyboard.print("## ");
      }
      else if(customKey == 'C'){
        Keyboard.print("### ");
      }
      else if(customKey == 'D'){
        Keyboard.print("#### ");
      }
      else if(customKey == '3'){
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.write('P');
        delay(10);
        Keyboard.releaseAll();
        delay(10);
        Keyboard.press(KEY_LEFT_SHIFT);
        delay(10);
        Keyboard.releaseAll();
        Keyboard.println("Run task");
        delay(10);
        Keyboard.println("Convert to HTML");
        delay(10);
        Keyboard.releaseAll();
      }
    }
    else if(layoutIndex == 2){
      if(customKey == '2'){
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.write('r');
        delay(10);
        Keyboard.releaseAll();
        delay(100);
        Keyboard.println(sleep_script);
        delay(100);
        Keyboard.releaseAll();
      }
      else if(customKey == '3'){
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.write('r');
        delay(10);
        Keyboard.releaseAll();
        delay(100);
        Keyboard.println(shutdown_script);
        delay(100);
        Keyboard.releaseAll();
      }
    }
    else if(layoutIndex == 3){
      if(customKey == '1'){
        delay(10);
        timerStarted = !timerStarted;
        currentSessionTime = lessonTime;
        minutes = 0;
        seconds = 0;
        startMillis = millis();
        drawMainMenu(muted,2);
        delay(10);
      }
      else if(customKey == '2'){
        delay(10);
        timerStarted = !timerStarted;
        currentSessionTime = breakTime;
        minutes = 0;
        seconds = 0;
        startMillis = millis();
        drawMainMenu(muted,2);
        delay(10);
      }
    }
  }
  // Help menu binding (always available)
  if(customKey == '*'){
      help = !help;
      screenUpdated=false;
      delay(100);
  }

  // If timer is started run pomodoroTimer() function
  if(timerStarted){
    pomodoroTimer();
  }
}
