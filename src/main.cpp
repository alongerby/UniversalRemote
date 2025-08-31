#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

const uint16_t kIrTxPin = 4;   // DAT pin
IRGreeAC ac(kIrTxPin);

String line;

void setup() {
  Serial.begin(115200);
  delay(2000); // let the port settle
  ac.begin();
  Serial.println("\nReady. Type ON or OFF, then press Enter.");
}

void sendState() {
  ac.send();
  Serial.println("IR sent.");
}

void processCmd(const String& s) {
  if (s == "ON") {
    ac.on();
    sendState();
  } else if (s == "OFF") {
    ac.off();
    sendState();
  } else {
    Serial.println("Unknown. Use ON or OFF.");
  }
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    Serial.print(c);
    if (c == '\r') {
      continue;
    }
    if(c == '\n'){
      line.trim();
      if(line == "ON"){
        Serial.print("HERE");
        Serial.println("ON");
        processCmd(line);
      }
      if(line == "OFF"){
        Serial.println("OFF");
        processCmd(line);
      }
      line = "";
    }else{
      line += c;
    }
  }
}
