#define meter 0
#define button 10
#define valve 6
#define led 9

unsigned long clickCount = 0;
unsigned long debounceDelay = 75;
unsigned long lastDebounceTime = 0;
int lastButtonState = 0;
int buttonState = 1;
unsigned long clickGoal = 3850; //2100clicks/4cups
bool buttonPressed = false;
int machineState = 0; // 0 = IDLE

//watchdog
int watchDogLength = 1000;
unsigned long watchDogTimer = 0;
unsigned long prevClickCount = 0;

void setup() {

  //Serial.begin(9600);
  pinMode(button,INPUT_PULLUP);
  pinMode(meter,INPUT_PULLUP);
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);
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
        digitalWrite(led,HIGH);
      }
      break;
    
    case 1:
      if (buttonPressed){
        buttonPressed = false;
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve,LOW);
        digitalWrite(led,LOW);
        break;
      }
      else if (clickCount >= clickGoal) {
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve,LOW);
        digitalWrite(led,LOW);
        break;
      }
      if (clickCount != prevClickCount){
        watchDogTimer = millis();
      }
      else if (millis() - watchDogTimer > watchDogLength){
        //no clicks longer than watchDogLength
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve,LOW);
        digitalWrite(led,LOW);
      }
      prevClickCount = clickCount;
      break;
  }
}

void clickDetected() {
  clickCount++;
}
