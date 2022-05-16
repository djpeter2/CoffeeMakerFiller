
const byte meter = 2;
const byte button = 9;
const byte valve = 6;
unsigned long clickCount = 0;
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;
int lastButtonState = 1;
int buttonState = 1;
unsigned long clickGoal = 40000;
bool buttonPressed = false;
int machineState = 0; // 0 = IDLE

void setup() {
  // put your setup code here, to run once:
  pinMode(button,INPUT_PULLUP);
  pinMode(meter,INPUT_PULLUP);
  pinMode(valve,OUTPUT);
  digitalWrite(valve,LOW);
  attachInterrupt(digitalPinToInterrupt(meter), clickDetected, RISING);
}

void loop() {
  // debounce button:
  int buttonReading = digitalRead(button);
  if (buttonReading != lastButtonState){
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonReading != buttonState){
      buttonState = buttonReading;
      if (buttonState = LOW) buttonPressed = true;
    }
  }
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
  }


}

void clickDetected() {
  clickCount++;
}
