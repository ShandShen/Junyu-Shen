const int ledPin = 13;

const int ldrPin = A0;

void setup() {

Serial.begin(9600);

pinMode(ledPin, OUTPUT);

pinMode(ldrPin, INPUT);

}

void loop() {

int ldrStatus = analogRead(ldrPin);

if (ldrStatus <=200) {

digitalWrite(ledPin, HIGH);

Serial.println( ldrStatus );
Serial.print("led on");

}

else {

digitalWrite(ledPin, LOW);

Serial.println( ldrStatus );
Serial.print("led off");
}

}
