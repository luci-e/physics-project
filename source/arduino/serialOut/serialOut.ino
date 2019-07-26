// These constants won't change. They're used to give names to the pins used:
const int analogInPin = 14;  // Analog input pin that the potentiometer is attached to

int sensorValue = 0;        // value read from the pot
char buffer[32];

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  analogReference(EXTERNAL);
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);

  // print the results to the Serial Monitor:
  sprintf( buffer, "%4d\n", sensorValue);
  Serial.print(buffer);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(1);
}
