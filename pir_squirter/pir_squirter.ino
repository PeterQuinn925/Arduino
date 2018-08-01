/*PIR Cat sprayer
   detect cat
   yes, there is a cat
      wait 10 sec. Is it still there?
         Yes, squirt it
         No, do nothing
   wait 15 sec
   end of loop

   squirt it
   fire squirter twice
*/
#include <Servo.h>
Servo myservo;
int calibrationTime = 30;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 500;

boolean takeLowTime;

int pirPin = 6;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;

void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  delay(1000);
  myservo.write(0);
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  for (int i = 0; i < calibrationTime; i++) {
    digitalWrite(ledPin,HIGH);
    Serial.print(".");
    digitalWrite(ledPin,LOW);
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
}

void loop() {
  delay(10);
  Serial.println("main loop");
  if (digitalRead(pirPin) == HIGH) {
    Serial.println("---");
    Serial.print("motion detected at ");
    Serial.print(millis() / 1000);
    Serial.println(" sec");
    digitalWrite(ledPin,HIGH);
    digitalWrite(pirPin, LOW);
    delay(10000);//wait 10 sec to see if it's still there
    if (digitalRead(pirPin) == HIGH) {//still high. time to squirt
      Serial.println("still there");
      squirt_it();
    }
    digitalWrite(ledPin,LOW);
  }
}

void squirt_it() {
  Serial.println("squirt");
  for (int i = 0; i <= 1; i++) {//squirt twice
    myservo.write(180);              // tell servo to go to position in variable 'pos'
    Serial.println(180);
    digitalWrite(ledPin,HIGH);
    delay(1000);                       // waits 15ms for the servo to reach the position
    myservo.write(0);
    Serial.println(0);
    digitalWrite(ledPin,LOW);
    delay(1000);                       // waits 15ms for the servo to reach the position
    myservo.write(180);              // tell servo to go to position in variable 'pos'
    Serial.println(180);
     myservo.write(0);
    Serial.println(0);
    digitalWrite(ledPin,HIGH);
    delay(2000);
  }
}


