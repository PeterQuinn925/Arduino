//ver 1.1 - use switch state instead of state variable to determine what to do
//ver 1.2 -  add error recovery routine and update logic for error states

int Light_Input = A0;
int Switch_Open = 6;//blue wire
int Switch_Closed = 5; //pin 7 not available on trinket use 5 instead. Green wire
int M_Reverse = 9; //green wire
int M_Forward = 10;//green wire
int M_On = 11;//yellow wire
int LED_pin = 13;//use built in LED on arduino
int Error_delay = 120 ; //seconds to wait to clear error
const String S_Open = "Open";
const String S_Closed = "Closed";
const String S_Opening = "Opening";
const String S_Closing = "Closing";
const String S_Error = "Error";
String State;
int Open_SetP = 25;//light level to open at 25 is good
int Close_SetP = 25;//light level to close
long Door_Timeout = 6.5 * 60 * 1000L; // Slower now
int Light_Level = 0;

void setup() {
  Serial.begin(9600);
  pinMode(Switch_Open, INPUT_PULLUP);
  pinMode(Switch_Closed, INPUT_PULLUP);
  pinMode(M_Reverse, OUTPUT);
  pinMode(M_Forward, OUTPUT);
  pinMode(M_On, OUTPUT);
  pinMode(LED_pin, OUTPUT); // blink for error. on for open, off for closed

  Light_Level = analogRead(Light_Input);//get initial light level

  //Determine initial state
  if (Switch_Check(Switch_Open) == HIGH) {
    State = S_Open;
    digitalWrite(LED_pin, HIGH);
  }
  else if (Switch_Check(Switch_Closed) == HIGH) {
    State = S_Closed;
    digitalWrite(LED_pin, LOW);
  }
  else if (Light_Level > Open_SetP) {//if the switches aren't set then open or close the door as needed
    Open_Door();
  }
  else if (Light_Level < Close_SetP) {
    Close_Door();
  }
  else {//should never get here unless the light level is between the open and closed setpoints.
    State = S_Error;
  }
}

void loop() {
  Serial.println(State);
  int i = 0;
  if (State == S_Error) {
    i = 0;
    do
    { // blink LED
      digitalWrite(LED_pin, HIGH);
      delay(500);
      digitalWrite(LED_pin, LOW);
      delay (500);
      i++;
    } while (i < Error_delay); //If it's the error state, wait n  seconds and try to clear
    Error_Recover();
  }
  
  Light_Level = analogRead(Light_Input);
  Serial.print("Light: ");
  Serial.println(Light_Level);
  // Bright enough to close or open the door?
  if (digitalRead(Switch_Open) == HIGH) { //Door is open, is it dark now?
    if (Light_Level < Close_SetP) Close_Door();
  }
  if (Switch_Check(Switch_Closed) == HIGH)
  { //Door is closed, bright enough to open?
    if (Light_Level > Open_SetP) Open_Door();
  }
  if (Switch_Check(Switch_Closed) == LOW && Switch_Check(Switch_Open) == LOW)//error recovery state. Door is partially open
  {
    if (Light_Level > Open_SetP)
    {
      Open_Door();
    }
    else
    {
      Close_Door();
    }
  }

  //delay(2 * 60 * 1000L); //wait 2 minutes see if brightness changes
  delay(15 * 1000L); //debugging and testing. wait 15 sec

}

void Open_Door() {
  State = S_Opening;
  Serial.println(State);
  digitalWrite(LED_pin, HIGH);//indicate the door is open
  long time1 = millis();
  long time2 = 0;
  analogWrite(M_On, 255); //turn on motor
  digitalWrite(M_Forward, true); //forward direction
  digitalWrite(M_Reverse, false);
  do
  {
    time2 = millis(); //check for timeout
   if (time2 - time1 > Door_Timeout) {
      State = S_Error;
      break;
    }
  } while (Switch_Check(Switch_Open) != HIGH);
  if (State != S_Error) State = S_Open;
  analogWrite(M_On, 0); //Stop the motor
  Serial.println(State);
}

void Close_Door() {
  State = S_Closing;
  Serial.println(State);
  digitalWrite(LED_pin, LOW); //indicate that the door is closed
  long time1 = millis();
  long time2 = 0;
  analogWrite(M_On, 255); //turn on motor
  digitalWrite(M_Forward, false); //reverse direction
  digitalWrite(M_Reverse, true);
  do
  {
    time2 = millis(); //check for timeout
    if (time2 - time1 > Door_Timeout) {
      State = S_Error;
      break;
    }
  } while (Switch_Check(Switch_Closed) != HIGH);
  if (State != S_Error) State = S_Closed;
  analogWrite(M_On, 0); //Stop the motor
  Serial.println(State);
}
bool Switch_Check(int Pinno)//debounce noisy switch
{
  bool switch_value1 = HIGH;
  bool switch_value2 = HIGH;
  do
  {
    switch_value1 = digitalRead(Pinno);
    delay(2); //wait 2 MS
    switch_value2 = digitalRead(Pinno);
  } while (switch_value1 != switch_value2);
  return switch_value1;
}

void Error_Recover()
{
  analogWrite(M_On, 32767); //turn on motor
  digitalWrite(M_Forward, true); //forward direction
  digitalWrite(M_Reverse, false);
  delay(10 * 1000); //10 seconds
  digitalWrite(M_Forward, false); //reverse direction
  digitalWrite(M_Reverse, true);
  delay(10 * 1000); //10 seconds
  analogWrite(M_On, 0); //Stop the motor
}


