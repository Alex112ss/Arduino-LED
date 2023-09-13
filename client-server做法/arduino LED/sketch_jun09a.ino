const int ledPin = 11;
int brightness = 160;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();
    if (receivedChar == '1') {
      analogWrite(ledPin, brightness);
    } else if (receivedChar == '0') {
      analogWrite(ledPin, 0);
    } else if (receivedChar == '+') {
      if (brightness < 255) {
        brightness += 40;
      }
      analogWrite(ledPin, brightness);
    } else if (receivedChar == '-') {
      if (brightness > 0) {
        brightness -= 40;
      }
      analogWrite(ledPin, brightness);
    }
  }
}
