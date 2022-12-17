#include <Keypad.h>

#include <TFT.h>
#include <SPI.h>

#include "HID-Project.h"

#define cs   2
#define dc   0
#define rst  1

TFT TFTscreen = TFT(cs, dc, rst);

// CONNECTION

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

// --------------------

// HELP MENU VARIABLES

// Help menu active or not
bool help = false;

// Help menu shortcuts
// LAYOUT > PAGE > SHORTCUT
String shortcuts[4][2][8] = {
  {
    {"1 - mute / unmute","* - help menu","4 - next layout", "5 - previous layout", "A - Spotify","B - Terminal","C - Code","D - Discord"},
    {"# - Monkeytype","7 - volume up","8 - volume down"}
  },
  {
    {"2 - text bold","A - # header","B - ## header", "C - ### header", "3 - export to HTML"}
  }
};
// Current help screen layout
int currentHelpIndex = 0;

// --------------------

// Layouts variables
int layoutIndex = 0;
int layoutsLength = 4;
String layoutNames[4] = {"General mode","Markdown mode","Undefined mode","Undefined mode"};
// --------------------

int findCenter(int length, int width = 6){
  return int((160-(length*width))/2);
}

word ConvertRGB( byte R, byte G, byte B)
{
  return ( ((B & 0xF8) << 8) | ((G & 0xFC) << 3) | (R >> 3) );
}

// Color palette

word iconColor = ConvertRGB(50, 255, 50);

// Drawing icon

// Unmuted icon
// 'unmuted', 10x14px
const unsigned char unmuteIcon [28] PROGMEM = {
	0x0c, 0x00, 0x1e, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0xbf, 0x40, 0xff, 0xc0, 
	0x5e, 0x80, 0x21, 0x00, 0x1e, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x1e, 0x00
};

// 'muted', 10x14px
const unsigned char muteIcon [28] PROGMEM = {
	0x0c, 0x40, 0x1e, 0x80, 0x3e, 0x80, 0x3d, 0x00, 0x3a, 0x00, 0x3a, 0x00, 0xb5, 0x40, 0xeb, 0xc0, 
	0x56, 0x80, 0x11, 0x00, 0x2e, 0x00, 0x4c, 0x00, 0x4c, 0x00, 0x9e, 0x00
};

// // ------------------------

// // This is horrible, I know...
void drawIcon(int x, int y, bool muted = false){
  if(muted){
    TFTscreen.drawBitmap(x, y, muteIcon, 10, 14, iconColor);
  }
  else{
    TFTscreen.drawBitmap(x, y, unmuteIcon, 10, 14, iconColor);
  }
}

// Drawing main menu
void drawMainMenu(bool muted, bool help){
  TFTscreen.background(0, 0, 0);
  TFTscreen.drawRect(5, 5, 150, 118,ConvertRGB(155, 33, 255));

  if(help == false){
    // MUTED / UNMUTED PROMPT
    if(muted){
      iconColor = ConvertRGB(255,50,50);
      drawIcon(findCenter(1,10),30,true);

      TFTscreen.stroke(50, 50, 255);
      TFTscreen.setTextSize(2);
      TFTscreen.text("Muted",findCenter(5,12),50);

      TFTscreen.stroke(255, 255, 255);
      TFTscreen.setTextSize(1);
      TFTscreen.text("Press 1 to unmute",findCenter(17),80);
    }
    else{
      iconColor = ConvertRGB(50,255,50);
      drawIcon(findCenter(1,10),30);

      TFTscreen.stroke(50, 255, 50);
      TFTscreen.setTextSize(2);
      TFTscreen.text("Unmuted",findCenter(7,12),50);

      TFTscreen.stroke(255, 255, 255);
      TFTscreen.setTextSize(1);
      TFTscreen.text("Press 1 to mute",findCenter(15),80);
    }

    // HELP MENU
    TFTscreen.stroke(0, 150, 255);
    TFTscreen.text("Press * for help",findCenter(16),20);

    // LAYOUT NAME
    TFTscreen.stroke(255, 150, 0);
    TFTscreen.text((layoutNames[layoutIndex]).c_str(),findCenter(layoutNames[layoutIndex].length()),100);
  }
  else{
    // HELP MENU TITLE
    TFTscreen.stroke(0, 150, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.text((layoutNames[layoutIndex]).c_str(),findCenter((layoutNames[layoutIndex]).length()),10);

    // HELP MENU ELEMENTS
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setTextSize(1);

    for(int i = 0; i < 8; i++){
      TFTscreen.text((shortcuts[layoutIndex][currentHelpIndex][i]).c_str(),findCenter((shortcuts[layoutIndex][currentHelpIndex][i]).length()),30 + (i*10));
    }

    // PAGE NUMBER
    TFTscreen.stroke(0, 150, 255);
    TFTscreen.text(("Page "+String(currentHelpIndex+1)+" of 2").c_str(),findCenter(11),110);
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

void setup(){
  Serial.begin(9600);
  Keyboard.begin();
  pinMode(13,OUTPUT);
  TFTscreen.begin();
  Consumer.begin();
}
  
void loop(){
  char customKey = customKeypad.getKey();

  // If help menu activated
  if(help){
    // If screen needs to be updated
    if(screenUpdated == false){
      Serial.println("Get help");
      drawMainMenu(muted,help);
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
  // If muted
  else if(muted){
    digitalWrite(13,HIGH);
    
    if(screenUpdated == false){
      Serial.println("Screen is getting updated");
      drawMainMenu(muted,help);
      screenUpdated = true;
    }
  }
  // If not muted and not in help menu
  else{
    digitalWrite(13,LOW);

    if(screenUpdated == false){
      drawMainMenu(muted,help);
      screenUpdated = true;
    }
  }
  // Help menu binding
  if(customKey == '*'){
      help = !help;
      screenUpdated=false;
      delay(100);
  }
  // If not in help menu; Main bindings
  if(!help){

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

    // NOT LAYOUT DEPENDENT KEYBINDINGS 
    if (customKey == '1'){ // Muting
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

    // Layouts

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
        drawMainMenu(muted,help);
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
  }
}
