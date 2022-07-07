#define meter 0
#define button 10
#define valve 6
unsigned long clickCount = 0;
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;
int lastButtonState = 0;
int buttonState = 1;
unsigned long clickGoal = 3900; //2100clicks/4cups
bool buttonPressed = false;
int machineState = 0; // 0 = IDLE
int ledState = LOW;

//watchdog
int watchDogLength = 1000;
unsigned long watchDogTimer = 0;
unsigned long prevClickCount = 0;

void setup() {

  //Serial.begin(9600);
  pinMode(button,INPUT_PULLUP);
  pinMode(meter,INPUT_PULLUP);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  pinMode(valve,OUTPUT);
  digitalWrite(valve,LOW);
  attachInterrupt(digitalPinToInterrupt(meter), clickDetected, FALLING);
}

void loop() {

  int reading = digitalRead(button);
  if (reading != lastButtonState){
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState){
      buttonState = reading;
      if (buttonState == LOW){
        buttonPressed = true;
      }
    }
  }
  
  lastButtonState = reading;
  
  switch (machineState){
    case 0:
      delay(10);
      if (buttonPressed){
        buttonPressed = false;
        machineState = 1;
        clickCount = 0;
        watchDogTimer = millis();
        digitalWrite(valve,HIGH);
      }
      break;
    
    case 1:
      if (buttonPressed){
        buttonPressed = false;
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve,LOW);
      }
      else if (clickCount >= clickGoal) {
        machineState =0;
        clickCount = 0;
        digitalWrite(valve,LOW);
      }
      if (clickCount != prevClickCount){
        watchDogTimer = millis();
      }
      else if (millis() - watchDogTimer > watchDogLength){
        //no clicks longer than watchDogLength
        machineState =0;
        clickCount = 0;
        digitalWrite(valve,LOW);
      }
      prevClickCount = clickCount;
      break;
  }
}

void clickDetected() {
  clickCount++;
}
