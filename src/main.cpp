#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 32
#define JOY_DELETE 33

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

String ssidInput = "";
String passInput = "";
bool inputMode = true; // true=SSID, false=Password

// Телевизорная клавиатура
const char keys[4][10] = {
  {'A','B','C','D','E','F','G','H','I','J'},
  {'K','L','M','N','O','P','Q','R','S','T'},
  {'U','V','W','X','Y','Z','0','1','2','3'},
  {'4','5','6','7','8','9','@','.','-','_'}
};
int cursorX=0, cursorY=0;
bool joyPressed=false, delPressed=false;
unsigned long lastDebounce=0;

// Меню приложений
int menuIndex=0;
const int menuSize=8;
String menuItems[menuSize] = {"Browser","Hotspot","Clock","Calculator","Status","Notes","Weather","Settings"};

String notesText = ""; // текст заметок

void drawKeyboardScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,0);
  display.print("Enter WiFi Credentials");

  display.setCursor(0,10);
  display.print("SSID: "+ssidInput);
  if(inputMode) display.print("_");

  display.setCursor(0,20);
  display.print("Password: "+passInput);
  if(!inputMode) display.print("_");

  // клавиатура
  for(int y=0;y<4;y++){
    for(int x=0;x<10;x++){
      int px = x*12;
      int py = 30+y*10;
      if(x==cursorX && y==cursorY){
        display.fillRect(px-1,py-1,10,10,SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(px,py);
        display.print(keys[y][x]);
        display.setTextColor(SSD1306_WHITE);
      } else {
        display.setCursor(px,py);
        display.print(keys[y][x]);
      }
    }
  }
  display.display();
}

void drawStatusBar(){
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  if(WiFi.status()==WL_CONNECTED) display.print("[WiFi]");
  else display.print("[NoNet]");
  display.setCursor(80,0);
  display.print(timeClient.getFormattedTime());
}

void drawMenu(){
  display.clearDisplay();
  drawStatusBar();
  for(int i=0;i<menuSize;i++){
    if(i==menuIndex){
      display.setCursor(0,10+i*6);
      display.print("> ");
      display.print(menuItems[i]);
    } else {
      display.setCursor(10,10+i*6);
      display.print(menuItems[i]);
    }
  }
  display.display();
}

void drawClock(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(2);
  display.setCursor(10,20);
  display.print(timeClient.getFormattedTime());
  display.display();
}

void drawCalculator(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Calculator ready...");
  display.display();
}

void drawStatus(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Battery: 100%");
  display.setCursor(0,30);
  display.print("CPU: OK");
  display.display();
}

void drawBrowser(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("GET https://example.com");
  display.display();
}

void drawHotspot(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Hotspot active!");
  WiFi.softAP("ESP32_OS_Hotspot","12345678");
  display.setCursor(0,30);
  display.print("SSID: ESP32_OS_Hotspot");
  display.display();
}

void drawNotes(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Notes:");
  display.setCursor(0,30);
  display.print(notesText);
  display.display();
}

void drawWeather(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Weather: 25C");
  display.display();
}

void drawSettings(){
  display.clearDisplay();
  drawStatusBar();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Settings Menu...");
  display.display();
}

void setup(){
  Wire.begin(21,22);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(JOY_DELETE, INPUT_PULLUP);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) while(true);
  drawKeyboardScreen();
}

void loop(){
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  bool btn = !digitalRead(JOY_BTN);
  bool delBtn = !digitalRead(JOY_DELETE);

  // клавиатура пока Wi-Fi не подключен
  if(WiFi.status()!=WL_CONNECTED){
    // навигация
    if(yVal<1000 && millis()-lastDebounce>150){ cursorY--; if(cursorY<0) cursorY=3; lastDebounce=millis();}
    if(yVal>3000 && millis()-lastDebounce>150){ cursorY++; if(cursorY>3) cursorY=0; lastDebounce=millis();}
    if(xVal<1000 && millis()-lastDebounce>150){ cursorX--; if(cursorX<0) cursorX=9; lastDebounce=millis();}
    if(xVal>3000 && millis()-lastDebounce>150){ cursorX++; if(cursorX>9) cursorX=0; lastDebounce=millis();}

    // выбор символа
    if(btn && !joyPressed){joyPressed=true; char c=keys[cursorY][cursorX]; if(inputMode) ssidInput+=c; else passInput+=c;}
    else if(!btn) joyPressed=false;

    // удаление
    if(delBtn && !delPressed){delPressed=true; if(inputMode && ssidInput.length()>0) ssidInput.remove(ssidInput.length()-1); else if(!inputMode && passInput.length()>0) passInput.remove(passInput.length()-1);}
    else if(!delBtn) delPressed=false;

    // переключение между SSID и паролем долгим нажатием
    static unsigned long pressStart=0;
    if(btn && pressStart==0) pressStart=millis();
    if(!btn && pressStart>0){if(millis()-pressStart>1000) inputMode=!inputMode; pressStart=0;}

    drawKeyboardScreen();

    // подключение к Wi-Fi
    if(ssidInput.length()>0 && passInput.length()>0 && !inputMode){
      display.clearDisplay(); display.setCursor(0,20); display.print("Connecting..."); display.display();
      WiFi.begin(ssidInput.c_str(), passInput.c_str());
      int timeout=0;
      while(WiFi.status()!=WL_CONNECTED && timeout<20){delay(500); timeout++;}
      if(WiFi.status()==WL_CONNECTED){
        timeClient.begin(); timeClient.update();
        drawMenu();
      } else {display.clearDisplay(); display.setCursor(0,20); display.print("Failed! Reset."); display.display(); while(true){}}
    }

    return;
  }

  timeClient.update();

  // навигация меню
  if(yVal<1000 && millis()-lastDebounce>200){ menuIndex--; if(menuIndex<0) menuIndex=menuSize-1; lastDebounce=millis();}
  if(yVal>3000 && millis()-lastDebounce>200){ menuIndex++; if(menuIndex>=menuSize) menuIndex=0; lastDebounce=millis();}

  // выбор приложения
  if(btn && !joyPressed){joyPressed=true;
    switch(menuIndex){
      case 0: drawBrowser(); break;
      case 1: drawHotspot(); break;
      case 2: drawClock(); break;
      case 3: drawCalculator(); break;
      case 4: drawStatus(); break;
      case 5: drawNotes(); break;
      case 6: drawWeather(); break;
      case 7: drawSettings(); break;
    }
  } else if(!btn) joyPressed=false;

  drawMenu();
  delay(50);
}
