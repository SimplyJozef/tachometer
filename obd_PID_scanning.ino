#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
// ELM327 MAC address
//uint8_t address[6] = {0x1C, 0xA1, 0x35, 0x69, 0x8D, 0xC5};

uint8_t address[6] = {0x00, 0x1D, 0xA5, 0x68, 0x98, 0x8A};

// List of OBD-II standard PIDs
const char* obdPIDs[] = {
  "0100", "0101", "0102", "0103", "0104", "0105", "0106", "0107", "0108", "0109", "010A", "010B", "010C", "010D", "010E", "010F",
  "0110", "0111", "0112", "0113", "0114", "0115", "0116", "0117", "0118", "0119", "011A", "011B", "011C", "011D", "011E", "011F"
};
const int obdPIDCount = sizeof(obdPIDs) / sizeof(obdPIDs[0]);

const char* extendedOBDPIDs[] = {
  // Mode 01 PIDs (Extended)
  "0120", "0121", "0122", "0123", "0124", "0125", "0126", "0127", "0128", "0129", "012A", "012B", "012C", "012D", "012E", "012F",
  "0130", "0131", "0132", "0133", "0134", "0135", "0136", "0137", "0138", "0139", "013A", "013B", "013C", "013D", "013E", "013F",
  "0140", "0141", "0142", "0143", "0144", "0145", "0146", "0147", "0148", "0149", "014A", "014B", "014C", "014D", "014E", "014F",
  "0150", "0151", "0152", "0153", "0154", "0155", "0156", "0157", "0158", "0159", "015A", "015B", "015C", "015D", "015E", "015F",
  "0160", "0161", "0162", "0163", "0164", "0165", "0166", "0167", "0168", "0169", "016A", "016B", "016C", "016D", "016E", "016F",
  "0170", "0171", "0172", "0173", "0174", "0175", "0176", "0177", "0178", "0179", "017A", "017B", "017C", "017D", "017E", "017F",
  "0180", "0181", "0182", "0183", "0184", "0185", "0186", "0187", "0188", "0189", "018A", "018B", "018C", "018D", "018E", "018F",
  "0190", "0191", "0192", "0193", "0194", "0195", "0196", "0197", "0198", "0199", "019A", "019B", "019C", "019D", "019E", "019F",
  "01A0", "01A1", "01A2", "01A3", "01A4", "01A5", "01A6", "01A7", "01A8", "01A9", "01AA", "01AB", "01AC", "01AD", "01AE", "01AF",
  "01B0", "01B1", "01B2", "01B3", "01B4", "01B5", "01B6", "01B7", "01B8", "01B9", "01BA", "01BB", "01BC", "01BD", "01BE", "01BF",
  "01C0", "01C1", "01C2", "01C3", "01C4", "01C5", "01C6", "01C7", "01C8", "01C9", "01CA", "01CB", "01CC", "01CD", "01CE", "01CF",
  "01D0", "01D1", "01D2", "01D3", "01D4", "01D5", "01D6", "01D7", "01D8", "01D9", "01DA", "01DB", "01DC", "01DD", "01DE", "01DF",
  "01E0", "01E1", "01E2", "01E3", "01E4", "01E5", "01E6", "01E7", "01E8", "01E9", "01EA", "01EB", "01EC", "01ED", "01EE", "01EF",
  "01F0", "01F1", "01F2", "01F3", "01F4", "01F5", "01F6", "01F7", "01F8", "01F9", "01FA", "01FB", "01FC", "01FD", "01FE", "01FF"
};
const int extendedOBDCount = sizeof(extendedOBDPIDs) / sizeof(extendedOBDPIDs[0]);

// List of BMW-specific PIDs
const char* bmwPIDs[] = {
  "2101", "2102", "2103", "2141", "2142", "2143", "2201", "2202", "221E", "2233", "225B",
  "2104", "2105", "2106", "2107", "2108", "2109", "2110", "2111", "2112", "2113", "2114",
  "2115", "2116", "2117", "2118", "2119", "2120", "2121", "2122", "2123"
};
const int bmwPIDCount = sizeof(bmwPIDs) / sizeof(bmwPIDs[0]);

void setup() {
  Serial.begin(115200);
  
  // Initialize Bluetooth
  if (!SerialBT.begin("ArduHUD", true)) {
    Serial.println("Failed to initialize Bluetooth!");
    while (1);
  }

  // Set Bluetooth PIN
  if (!SerialBT.setPin("1234", 4)) {
    Serial.println("Failed to set Bluetooth PIN!");
    while (1);
  }

  // Connect to ELM327
  if (!SerialBT.connect(address)) {
    Serial.println("Failed to connect to ELM327.");
    while (1);
  } else {
    Serial.println("Connected to ELM327!");
  }

  // Set OBD protocol to auto-detect
  SerialBT.print("AT SP 0\r");
  delay(500);
  SerialBT.print("AT DP\r");
  delay(500);
  Serial.print("Current Protocol: ");
  Serial.println(SerialBT.readStringUntil('\r'));
  
  // Disable command echo
  SerialBT.print("AT E0\r");
  delay(500);
}

void loop() {
  if (!SerialBT.connected()) {
    Serial.println("Bluetooth connection lost. Reconnecting...");
    if (!SerialBT.connect(address)) {
      Serial.println("Failed to reconnect to ELM327.");
      delay(1000);
      return;
    }
  }

  testPIDs(obdPIDs, obdPIDCount, "Standard OBD-II PIDs");
  testPIDs(bmwPIDs, bmwPIDCount, "BMW-Specific PIDs");
  testPIDs(extendedOBDPIDs, extendedOBDCount, "Extended PIDs");
  
  delay(5000); // Wait before repeating
}

void testPIDs(const char* pids[], int count, const char* category) {
  Serial.println(category);
  for (int i = 0; i < count; i++) {
    SerialBT.print(String(pids[i]) + "\r");
    delay(100);
    String response = readELMResponse();
    Serial.print(pids[i]);
    Serial.print(" Response: ");
    Serial.println(response);
  }
}

String readELMResponse() {
  unsigned long startTime = millis();
  String response = "";
  while (millis() - startTime < 500) {
    if (SerialBT.available()) {
      char c = SerialBT.read();
      response += c;
      if (c == '\r') {
        break;
      }
    }
  }
  return response;
}
