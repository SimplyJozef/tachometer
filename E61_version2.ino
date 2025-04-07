#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

#include <SPI.h>
#include <TFT_eSPI.h> 
#include <XPT2046_Touchscreen.h>


#define SCREEN_HEIGHT 240  

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 13  // T_DIN
#define XPT2046_MISO 12  // T_OUT
#define XPT2046_CLK 14   // T_CLK
#define XPT2046_CS 33    // T_CS


#define nameColor 0xFB00    
#define progressColor 0xFB00     
#define unitColor 0xFB00    

SPIClass touchscreenSPI = SPIClass(HSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

uint8_t address[6] = {0x1C, 0xA1, 0x35, 0x69, 0x8D, 0xC5};

bool menu = true;
int simulation_value = 50;



int prev_rectWidth1 = 0;
int prev_rectWidth2 = 0;
int prev_rectWidth3 = 0;

const int graphWidth = 320;  
const int graphHeight = 66;  
const int barWidth = 1;     

int focusIndex = 0;

const int SHORT_PRESS_TIME = 200; 
const int LONG_PRESS_TIME = 1000;  

bool touchActive = false;
unsigned long touchStartTime = 0;
unsigned long touchEndTime = 0;

const int btn1 = 22; 


int currentSection = 0; 
int currentVarTop = 0;
int currentVarMiddle = 1;
int currentVarBottom = 2;
const int NUM_VARS = 7; 

int prevDigitCount0 = 0;
int prevDigitCount80 = 0;
int prevDigitCount160 = 0;




void setup() {
  Serial.begin(115200);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_BLACK);

  pinMode(btn1, INPUT_PULLUP);

  if (!menu) {
    if (!SerialBT.begin("ArduHUD", true)) {
      Serial.println("Failed to initialize Bluetooth!");
      while (1);
    }

    if (!SerialBT.setPin("1234", 4)) {
      Serial.println("Failed to set Bluetooth PIN!");
      while (1);
    } 

    tft.setTextColor(progressColor, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(0,0);
    tft.print("Connecting...");
    Serial.println("Connecting");

    if (!SerialBT.connect(address)) {
      tft.print("Faild to connect");
      Serial.println("Failed to connect to ELM327.");
      while (1);
    } else {
      Serial.println("Connected to ELM327!");
      tft.print("Connected");
    }


    

    SerialBT.print("AT Z\r");  
    delay(1000);
    SerialBT.print("AT E0\r");   
    SerialBT.print("AT SP 0\r"); 
    delay(500);
  }
}

int drawStyle1 = 0;
int drawStyle2 = 1;
int drawStyle3 = 0;

bool btnActive = false;
unsigned long pressStartTime = 0;

void loop() {
  if (simulation_value > 7000) {
    simulation_value = 0;
  } else {
    simulation_value += 50;
  }

  bool btnState = digitalRead(btn1);
  
  
  if (btnState == LOW && !btnActive) {
    pressStartTime = millis();
    btnActive = true;
  }
  
  if (btnState == HIGH && btnActive) {
    btnActive = false;
    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration < SHORT_PRESS_TIME) {
      Serial.println("short fyz");
      changeSectionDraw(focusIndex);
    }
  }


  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    if (!touchActive) {
      touchStartTime = millis();
      touchActive = true;
    }
  } else if (touchActive) {
    
    touchActive = false;
    touchEndTime = millis();

    long pressDuration = touchEndTime - touchStartTime;

    if (pressDuration < SHORT_PRESS_TIME) {
      
      TS_Point p = touchscreen.getPoint();
      int touchY = map(p.x, 200, 3800, 1, SCREEN_HEIGHT);
      
      tft.drawRect(0, 0, 320, 80, TFT_BLACK);
      tft.drawRect(0, 80, 320, 80, TFT_BLACK);
      tft.drawRect(0, 160, 320, 80, TFT_BLACK);

      if (touchY >= 0 && touchY < 80) {
        if (focusIndex == 0) {
          changeSectionVar(0);
        } else {
          focusIndex = 0;
        }
      } else if (touchY >= 80 && touchY < 160) {
        if (focusIndex == 1) {
          changeSectionVar(1);
        } else {
          focusIndex = 1;
        }
      } else if (touchY >= 160 && touchY < 240) {
        if (focusIndex == 2) {
          changeSectionVar(2);
        } else {
          focusIndex = 2;
        }
      }

      Serial.print("short touch");
    
    } else if (pressDuration > LONG_PRESS_TIME) {
      menu = !menu;
      tft.fillRect(0, 0, 320, 240, TFT_BLACK);
      Serial.println("long touch");
    }
  }

  // top
  switch(currentVarTop % NUM_VARS) {
    case 0: getEngineRPM(0, drawStyle1); break;
    case 1: getVehicleSpeed(0, drawStyle1); break;
    case 2: getCoolantTemp(0, drawStyle1); break;
    case 3: getIntakeAirTemp(0, drawStyle1); break;
    case 4: getEngineLoad(0, drawStyle1); break;
    case 5: getIntakeMAP(0, drawStyle1); break;
    case 6: getMAFFlowRate(0, drawStyle1); break;
  }

  // middle 
  switch(currentVarMiddle % NUM_VARS) {
    case 0: getEngineRPM(80, drawStyle2); break;
    case 1: getVehicleSpeed(80, drawStyle2); break;
    case 2: getCoolantTemp(80, drawStyle2); break;
    case 3: getIntakeAirTemp(80, drawStyle2); break;
    case 4: getEngineLoad(80, drawStyle2); break;
    case 5: getIntakeMAP(80, drawStyle2); break;
    case 6: getMAFFlowRate(80, drawStyle2); break;
  }

  // bottom 
  switch(currentVarBottom % NUM_VARS) {
    case 0: getEngineRPM(160, drawStyle3); break;
    case 1: getVehicleSpeed(160, drawStyle3); break;
    case 2: getCoolantTemp(160, drawStyle3); break;
    case 3: getIntakeAirTemp(160, drawStyle3); break;
    case 4: getEngineLoad(160, drawStyle3); break;
    case 5: getIntakeMAP(160, drawStyle3); break;
    case 6: getMAFFlowRate(160, drawStyle3); break; 
  }

  clearBluetoothBuffer();
  delay(50);
}

void clearBluetoothBuffer() {
  while (SerialBT.available()) {
    SerialBT.read();
  }
}

int countDigit(int n) { 
  return floor(log10(n) + 1); 
}

void drawBar(int y, const char* name, int value, const char* unit, int minVal, int maxVal) {
  int rectWidth = map(value, minVal, maxVal, 0, 320);

  switch(y) {
    case 0:
      if (rectWidth != prev_rectWidth1) {
        tft.fillRect(0 + prev_rectWidth1, y + 25, 320 - prev_rectWidth1, 55, TFT_BLACK);
        tft.fillRect(0, y + 25, rectWidth, 55, progressColor);
        prev_rectWidth1 = rectWidth; 
      } 
      break;
    
    case 80:
      if (rectWidth != prev_rectWidth2) {
        tft.fillRect(0 + prev_rectWidth2, y + 25, 320 - prev_rectWidth2, 55, TFT_BLACK);
        tft.fillRect(0, y + 25, rectWidth, 55, progressColor);
        prev_rectWidth2 = rectWidth;
      } 
      break;

    case 160:
      if (rectWidth != prev_rectWidth3) {
        tft.fillRect(0 + prev_rectWidth3, y + 25, 320 - prev_rectWidth3, 55, TFT_BLACK);
        tft.fillRect(0, y + 25, rectWidth, 55, progressColor);
        prev_rectWidth3 = rectWidth; 
      } 
      break;
  }

  switch(y) {
    case 0:
      if (countDigit(prevDigitCount0) != countDigit(value)) {
        tft.fillRect(0, 0, 320, 25, TFT_BLACK); 
      } 
      break;
    
    case 80:
      if (countDigit(prevDigitCount80) != countDigit(value)) {
        tft.fillRect(0, 80, 320, 25, TFT_BLACK); 
      } 
      break;

    case 160:
      if (countDigit(prevDigitCount160) != countDigit(value)) {
        tft.fillRect(0, 160, 320, 25, TFT_BLACK); 
      } 
      break;
  }

  switch(y) {
    case 0: prevDigitCount0 = value; break;
    case 80: prevDigitCount80 = value; break;
    case 160: prevDigitCount160 = value; break;
  }

  // name
  tft.setTextColor(nameColor, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, y + 5);
  tft.print(name);
 
  // value
  tft.setTextColor(progressColor, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10 + tft.textWidth(String(name).c_str(), 2), y + 5);
  tft.print(value);

  // unit
  tft.setTextColor(unitColor, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10 + tft.textWidth(String(value).c_str(), 2) + tft.textWidth(String(name).c_str(), 2), y + 5);
  tft.print(unit);

  if (menu) {
    if (focusIndex == 0) {
      tft.drawRect(0, 0, 320, 80, progressColor);
    }

    if (focusIndex == 1) {
      tft.drawRect(0, 80, 320, 80, progressColor);
    }
    if (focusIndex == 2) {
      tft.drawRect(0, 160, 320, 80, progressColor);
    }
  }
}

void drawParameter(int y, const char* name, int value, const char* unit, int minVal, int maxVal) {
  switch(y) {
    case 0:
      if (countDigit(prevDigitCount0) != countDigit(value)) {
        tft.fillRect(0, 0, 320, 80, TFT_BLACK); 
      } 
      break;
    
    case 80:
      if (countDigit(prevDigitCount80) != countDigit(value)) {
        tft.fillRect(0, 80, 320, 80, TFT_BLACK); 
      } 
      break;

    case 160:
      if (countDigit(prevDigitCount160) != countDigit(value)) {
        tft.fillRect(0, 160, 320, 80, TFT_BLACK); 
      } 
      break;
  }

  switch(y) {
    case 0: prevDigitCount0 = value; break;
    case 80: prevDigitCount80 = value; break;
    case 160: prevDigitCount160 = value; break;
  }
  
  // name
  tft.setTextColor(nameColor, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, y + 10);
  tft.print(name);
  
  // value
  tft.setTextColor(progressColor, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(10, y + 35);
  tft.print(value);
  
  // unit
  tft.setTextColor(unitColor, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10 + tft.textWidth(String(value).c_str(), 4) + 5, y + 45);
  tft.print(unit);

  if (menu) {
    if (focusIndex == 0) {
      tft.drawRect(0, 0, 320, 80, progressColor);
    }

    if (focusIndex == 1) {
      tft.drawRect(0, 80, 320, 80, progressColor);
    }
    if (focusIndex == 2) {
      tft.drawRect(0, 160, 320, 80, progressColor);
    }
  }
}

String sendELMCommand(String command) {
  SerialBT.print(command + "\r");
  delay(50); 
  
  unsigned long startTime = millis();
  String response = "";
  bool promptReceived = false;
  
  while (millis() - startTime < 1000 && !promptReceived) {
    while (SerialBT.available()) {
      char c = SerialBT.read();
      response += c;
      
      if (c == '>') {
        promptReceived = true;
        break;
      }
    }
  }

  response.trim();
  response.replace("\r", "");
  response.replace("\n", "");
  response.replace(" ", "");
  response.replace(">", "");
  
  response = response.substring(4);
  
  return response;
}

void getCoolantTemp(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Coolant", simulation_value, "C", -40, 120);
    } else {
      drawBar(y, "Coolant", simulation_value, "C", -40, 120);
    }
  } else {
    String coolantResponse = sendELMCommand("0105");
    if (coolantResponse.startsWith("4105")) {
      String hexValue = coolantResponse.substring(4, 6);
      int coolantTemp = strtol(hexValue.c_str(), NULL, 16) - 40;
      if (drawStyle == 0) {
        drawParameter(y, "Coolant", simulation_value, "C", -40, 120);
      } else {
        drawBar(y, "Coolant", simulation_value, "C", -40, 120);
      }
    }
  }
}

void getEngineLoad(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Engine Load", simulation_value, "%", 0, 100);
    } else {
      drawBar(y, "Engine Load", simulation_value, "%", 0, 100);
    }
  } else {
    String loadResponse = sendELMCommand("0104");
    if (loadResponse.startsWith("4104")) {
      String hexValue = loadResponse.substring(4, 6);
      int loadValue = strtol(hexValue.c_str(), NULL, 16) * 100 / 255.0;
      if (drawStyle == 0) {
        drawParameter(y, "Engine Load", loadValue, "%", 0, 100);
      } else {
        drawBar(y, "Engine Load", loadValue, "%", 0, 100);
      }
    }
  }
}

void getIntakeMAP(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Intake MAP", simulation_value, "kPa", 0, 255);
    } else {
      drawBar(y, "Intake MAP", simulation_value, "kPa", 0, 255);
    }
  } else {
    String mapResponse = sendELMCommand("010B");
    if (mapResponse.startsWith("410B")) {
      String hexValue = mapResponse.substring(4, 6);
      int mapValue = strtol(hexValue.c_str(), NULL, 16);
      if (drawStyle == 0) {
        drawParameter(y, "Intake MAP", mapValue, "kPa", 0, 255);
      } else {
        drawBar(y, "Intake MAP", mapValue, "kPa", 0, 255);
      }
    }
  }
}

void getEngineRPM(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Engine speed", simulation_value, "RPM", 0, 8000);
    } else {
      drawBar(y, "Engine speed", simulation_value, "RPM", 0, 8000);
    }
  } else {
    String rpmResponse = sendELMCommand("010C");
    if (rpmResponse.startsWith("410C")) {
      String hexValue = rpmResponse.substring(4, 8);
      int rpmValue = strtol(hexValue.c_str(), NULL, 16) / 4;
      if (drawStyle == 0) {
        drawParameter(y, "Engine speed", rpmValue, "RPM", 0, 8000);
      } else {
        drawBar(y, "Engine speed", rpmValue, "RPM", 0, 8000);
      }
    }
  }
}

void getVehicleSpeed(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Vehicle Speed", simulation_value, "km/h", 0, 200);
    } else {
      drawBar(y, "Vehicle Speed", simulation_value, "km/h", 0, 200);
    }
  } else {
    String speedResponse = sendELMCommand("010D");
    if (speedResponse.startsWith("410D")) {
      String hexValue = speedResponse.substring(4, 6);
      int speedValue = strtol(hexValue.c_str(), NULL, 16);
      if (drawStyle == 0) {
        drawParameter(y, "Vehicle Speed", speedValue, "km/h", 0, 200);
      } else {
        drawBar(y, "Vehicle Speed", speedValue, "km/h", 0, 200);
      }
    }
  }
}

void getIntakeAirTemp(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "Intake Air Temp", simulation_value, "°C", -40, 100);
    } else {
      drawBar(y, "Intake Air Temp", simulation_value, "°C", -40, 100);
    }
  } else {
    String iatResponse = sendELMCommand("010F");
    if (iatResponse.startsWith("410F")) {
      String hexValue = iatResponse.substring(4, 6);
      int iatValue = strtol(hexValue.c_str(), NULL, 16) - 40;
      if (drawStyle == 0) {
        drawParameter(y, "Intake Air Temp", iatValue, "°C", -40, 100);
      } else {
        drawBar(y, "Intake Air Temp", iatValue, "°C", -40, 100);
      }
    }
  }
}

void getMAFFlowRate(int y, int drawStyle) {
  if (menu) {
    if (drawStyle == 0) {
      drawParameter(y, "MAF Flow Rate", simulation_value, "g/s", 0, 500);
    } else {
      drawBar(y, "MAF Flow Rate", simulation_value, "g/s", 0, 500);
    }
  } else {
    String mafResponse = sendELMCommand("0110");
    if (mafResponse.startsWith("4110")) {
      String hexValue = mafResponse.substring(4, 8);
      int mafValue = strtol(hexValue.c_str(), NULL, 16) / 100.0;
      if (drawStyle == 0) {
        drawParameter(y, "MAF Flow Rate", mafValue, "g/s", 0, 500);
      } else {
        drawBar(y, "MAF Flow Rate", mafValue, "g/s", 0, 500);
      }
    }
  }
}

void changeSectionDraw(int currentFocus) {
  if (currentFocus == 0) {
    tft.fillRect(0, 0, 320, 80, TFT_BLACK);
    drawStyle1 = (drawStyle1 + 1) % 2;
  } else if (currentFocus == 1) {
    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    drawStyle2 = (drawStyle2 + 1) % 2;
  } else if (currentFocus == 2) {
    tft.fillRect(0, 160, 320, 80, TFT_BLACK);
    drawStyle3 = (drawStyle3 + 1) % 2;
  }
}

void changeSectionVar(int currentFocus) {
  if (currentFocus == 0) {
    tft.fillRect(0, 0, 320, 80, TFT_BLACK);
    currentVarTop = (currentVarTop + 1) % NUM_VARS;
  } else if (currentFocus == 1) {
    tft.fillRect(0, 80, 320, 80, TFT_BLACK);
    currentVarMiddle = (currentVarMiddle + 1) % NUM_VARS;
  } else if (currentFocus == 2) {
    tft.fillRect(0, 160, 320, 80, TFT_BLACK);
    currentVarBottom = (currentVarBottom + 1) % NUM_VARS;
  }
}