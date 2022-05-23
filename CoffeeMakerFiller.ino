int meter = 0;
int button = 10;
int valve = 6;
unsigned long clickCount = 0;
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;
int lastButtonState = 0;
int buttonState;
unsigned long clickGoal = 100000;
bool buttonPressed = false;
int machineState = 0; // 0 = IDLE
int ledState = LOW;

void setup() {

  //Serial.begin(9600);
  pinMode(10,INPUT_PULLUP);
  pinMode(meter,INPUT_PULLUP);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  pinMode(valve,OUTPUT);
  digitalWrite(valve,LOW);
  attachInterrupt(digitalPinToInterrupt(meter), clickDetected, FALLING);
}

void loop() {

  int reading = digitalRead(10);
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
      if (buttonPressed){
        buttonPressed = false;
        machineState = 1;
        clickCount = 0;
        digitalWrite(valve,HIGH);
      }
      break;
    case 1:
      if (buttonPressed){
        buttonPressed = false;
        machineState = 0;
        digitalWrite(valve,LOW);
        
      }
      if (clickCount >= clickGoal) {
        machineState =0;
        digitalWrite(valve,LOW);
        
      }
      break;
  }
  
}

void clickDetected() {
  clickCount++;
  ledState = !ledState;
  digitalWrite(13,ledState);
}
