#include <TM74HC595Display.h>

#define meter 0
#define valve 18
#define fullSwitch 13
#define emptySwitch 12

//rotary encoder
#define CLK 2
#define DT 3
#define SW 5

//LED segment display
#define SCLK 6 
#define RCLK 9
#define DIO 10

int thisState = 0;
int oldState = 0;
int knobPosition = 0;
int counter = 0;

const int8_t KNOBDIR[] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

unsigned long clickCount = 0;
unsigned long debounceDelay = 75;
unsigned long lastDebounceTimes[3];
int inputs[] = {SW, fullSwitch, emptySwitch};
int lastInputStates[3];
int inputStates[3];
int lastLEDstate = 0;
unsigned long clickGoal = 3850; //2100clicks/4cups
int cupGoal = 12;

bool buttonPressed = false;
int machineState = 0; // 0 = IDLE

/**
bit - segment
1   - decimal point
2   - CENTER
3   - upper LEFT
4   - lower LEFT
5   - BOTTOM
6   - lower RIGHT
7   - upper RIGHT
8.  - TOP
0b.D.C.UL.LL.B.LR.UR.T
*/

TM74HC595Display disp(SCLK, RCLK, DIO);
unsigned char led[] = {
0xC0, //0
0xF9, //1
0xA4, //2
0xB0, //3
0x99, //4
0x92, //5
0x82, //6
0xF8, //7
0x80, //8
0x90, //9
};
unsigned char decimals[] = {
  0b01000000, //0
  0b01111001, //1
  0b00100100, //2
  0b00110000, //3
  0b00011001, //4
  0b00010010, //5
  0b00000010, //6
  0b01111000, //7
  0b00000000, //8
  0b00011000  //9
};
//watchdog
int watchDogLength = 1000;
unsigned long watchDogTimer = 0;
unsigned long prevClickCount = 0;

void setup() {
  pinMode(meter, INPUT_PULLUP);
  pinMode(fullSwitch, INPUT_PULLUP);
  pinMode(emptySwitch, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(valve, OUTPUT);
  digitalWrite(valve, LOW);
  attachInterrupt(digitalPinToInterrupt(meter), clickDetected, FALLING);
}

void loop() {
  
  for (int i = 0; i < 3; i++ ) {   
    int reading = digitalRead(inputs[i]);
    if (reading != lastInputStates[i]){
      lastDebounceTimes[i] = millis();
    }
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != inputStates[i]){
        inputStates[i] = reading;
        if (i == 0 && inputStates[i] == LOW){ // i = 0 for button
          buttonPressed = true;
        }
      }
    }
    lastInputStates[i] = reading;
  }

  switch (machineState){
    case 0: // Idle
      int sig1 = digitalRead(DT);
      int sig2 = digitalRead(CLK);
      thisState = sig1 | (sig2 << 1);
      int index = thisState | (oldState<<2);
      oldState = thisState;
      knobPosition += KNOBDIR[index];
      if (thisState == 3){
        cupGoal = knobPosition>>2;
      }
       cupGoal *= 5;
      if ( 120 < cupGoal) {
        cupGoal = 120;
        knobPosition = 96;
      } else if ( cupGoal < 0) {
        cupGoal = 0;
        knobPosition = 0;
      }
      digit3(cupGoal);

      if (buttonPressed){
        buttonPressed = false;
        machineState = 1;
        clickCount = 0;
        clickGoal = cupGoal * (4 / 2100);
        digit3(clickCount);
        watchDogTimer = millis();
        digitalWrite(valve, HIGH);
      }
      break;
    
    case 1: // Fill
      if (buttonPressed){
        buttonPressed = false;
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve, LOW);
        break;
      }
      else if (clickCount >= clickGoal) {
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve, LOW);
        break;
      }
      else if (fullSwitch) {
        machineState = 2;
        digitalWrite(valve, LOW);
        lastLEDstate = HIGH;
        break;
      }
      if (clickCount != prevClickCount){
        watchDogTimer = millis();
      }
      else if (millis() - watchDogTimer > watchDogLength){
        //no clicks longer than watchDogLength
        machineState = 0;
        clickCount = 0;
        digitalWrite(valve, LOW);
      }
      prevClickCount = clickCount;
      int fillVolume = clickCount * (4 / 2100);
      digit3(fillVolume);
      break;

    case 2: // Full
      
      display(0b10001110, 0b1000); //F
      display(0b11000001, 0b0100); //U
      display(0b11000111, 0b0010); //L
      display(0b11000111, 0b0001); //L

      if (fullSwitch && !emptySwitch){
        machineState = 0;
        break;
      }
      if (buttonPressed){
        buttonPressed = false;
        machineState = 0;
        clickCount = 0;
        break;
      }
      break;
  }
}

void clickDetected() {
  clickCount++;
}

void send(unsigned char X){
  for (int i = 8; i >= 1; i--){
    if (X & 0x80){
      digitalWrite(DIO, HIGH);
    }
    else{
      digitalWrite(DIO, LOW);
    }
    X <<= 1;
    digitalWrite(SCLK, LOW);
    digitalWrite(SCLK, HIGH);  
  }
}

void display(unsigned char X, unsigned char port){
  send(X);
  send(port);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
}

void digit3(int n){
  int n1, n2, n3, n4;
  n1 = (int)  n % 10;
  n2 = (int) ((n % 100) - n1)/10;
  n3 = (int) ((n % 1000) - n2 - n1) / 100;
  n4 = (int) ((n % 10000) - n3 - n2 - n1) / 1000;
 
	display(led[n1], 0b0001);
  if(n>9)display(decimals[n2], 0b0010);
  else display(decimals[0], 0b0010);
  if(n>99)display(led[n3], 0b0100);
  if(n>999)display(led[n4], 0b1000);
 
}