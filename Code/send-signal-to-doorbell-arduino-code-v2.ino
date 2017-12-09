   #include <Wire.h>
#include <NewPing.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define inArray(x) (sizeof(x) / sizeof(x[0]))
#define OLED_RESET 4
#define LIGHT 3
#define BUTTON 12 // Settings Button.
#define TRIGGER_PIN  6  // Trigger-Pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Echo-Pin on the ultrasonic sensor.
#define MAX_DISTANCE 350 // Maximum distance we want to ping for (in centimeters).
#define TRANS_PIN 2 // Transmit Pin
int echoArray[] = {200,200,200,200}; // define the array
int arrLength = (sizeof( echoArray ) / sizeof( int )); // Get the size of the array

Adafruit_SSD1306 display(OLED_RESET);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

byte counter = 0;
byte TRIGGER_AT_CM = 50;
byte triggered = 0;
byte avgDist;
byte currentSonar = 0;
byte resendCount = 2; // amount of resends.
int resendDelay = 6050; // in microseconds

const int intervalDisplayUpdate = 330;
const int intervalCheck = 100;
const long minDelay = 60000;
const long maxDelay = 660000;
const long sonarInterval = 50;

int currMode = 0;
int setupModeTimeLimit = 20000;
unsigned long countdown = 60000;
unsigned long previousDisplayMillis = 0;
unsigned long previousCheckMillis = 0;
unsigned long previousSonarMillis = 0;
//unsigned long timeArr[5] = {1000, 2000, 3000, 4000, 5000};
unsigned long timeArr[5] = {60000, 180000, 360000, 600000, 1200000};

// First
/*
byte packets[] = {
  0,0,0,1, 
  0,1,0,1, 
  0,0,0,1, 
  1,1,1,1, 
  0,1,0,1, 
  0,1,0,0, 
  0
};
*/

// Second
/*
byte packets[] = {
  0,0,0,1,
  0,1,0,0,
  0,0,0,1,
  1,1,1,1,
  0,1,0,1,
  0,1,0,0,
  0
};
*/



void setup()   {
  pinMode(LIGHT, OUTPUT);           
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  randomSeed(analogRead(A1)+analogRead(A4));  


  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("David Skoglund");
  display.setTextColor(BLACK, WHITE);
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("0xDOORBELL");
  display.display();
  delay(2000);
  display.clearDisplay();
}


void loop() {

    unsigned long currentMillis = millis();
    if (currentMillis - previousSonarMillis >= sonarInterval) {
      
          previousSonarMillis = currentMillis;
          
          measureSonar(currentSonar);
          currentSonar = currentSonar + 1;
          
          if(currentSonar >= arrLength){
            currentSonar = 0;
          }
    }


  // byte state = digitalRead(BUTTON);
  if(currMode == 0){
    setupMode();
  } else {
    if(digitalRead(BUTTON) == HIGH && triggered == 0){
      delay(300);
      if(digitalRead(BUTTON) == HIGH){
        delay(2000);
        currMode = 0; // go back to setup;
      } else {
        ringDoorbell();
        Serial.print(F("Signal sent to the doorbell."));
        display.clearDisplay();
        display.println("Signal sent to the doorbell.");
        display.display();
        delay(2000);
      }
    } else if(digitalRead(BUTTON) == HIGH && triggered == 1){
      display.clearDisplay();
      display.println("Trigger was aborted.");
      display.display();
      triggered=0;
      delay(2000);
    }
    checkAvg();
    refreshDisplay();

  }
}


void checkAvg(void){
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousCheckMillis >= intervalCheck) {
    
    previousCheckMillis = currentMillis;
    
    if(triggered == 0){
              
    
        long tmpDist = 0;
        
        for (int i=0; i < arrLength; i++)
        {
          tmpDist = tmpDist + echoArray[i];
        }
        
        avgDist = tmpDist / arrLength;
        
        if( avgDist < TRIGGER_AT_CM && avgDist > 0 )
        {
          triggered = 1;
          digitalWrite(LIGHT, HIGH);

          int tmp = random(0, 4);
          countdown = timeArr[tmp];

          Serial.print("Triggered with millis: ");
          Serial.println(countdown);
        }
    } else {
              Serial.println(countdown);
        if(countdown > 1){
            
            countdown = countdown - 100;
            if(countdown > 4000000000) countdown = 0;
            
        } else {
              ringDoorbell();

              counter=counter+1;
              countdown = 60000;
              Serial.print("Counter incrementet to: ");
              Serial.println(counter);
              triggered=0;
              digitalWrite(LIGHT, LOW);
        }
        
    }
  }
}


void setupMode(void){
    unsigned long currentMillis = millis();
  if (currentMillis - previousDisplayMillis >= intervalCheck) {
      previousDisplayMillis = currentMillis;
        long tmpDist = 0;
        
        for (int i=0; i < arrLength; i++)
        {
          tmpDist = tmpDist + echoArray[i];
        }
        
        avgDist = tmpDist / arrLength;
    Serial.print(avgDist);
    Serial.print(F("cm/avg source("));

    for (int i=0; i < arrLength; i++)
    {
      Serial.print(i);
      Serial.print("=");
      Serial.print(echoArray[i]);
      if(i < arrLength-1) Serial.print(", ");
    }  
    Serial.println(")");  
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.println("  *  Setup Mode   *  ");
      display.setCursor(0,10);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.println("Trigger Distance:");
      display.setTextSize(2);
      display.println(avgDist);
      display.setTextSize(1);
      display.println("Press Ok to set");
      display.display();
      
      setupModeTimeLimit = setupModeTimeLimit - 100;
      
      if(setupModeTimeLimit == 0 || digitalRead(BUTTON) == HIGH){
        TRIGGER_AT_CM = avgDist;
        currMode = 1;
          display.clearDisplay();
          display.setCursor(0,10);
          display.println("Live in a few sec!");
          display.display();
          delay(3000);
      }
  }

}

/***
 * measureSonar function
 * does the measuring.
 */
void measureSonar(byte current){
          unsigned int uS = sonar.ping();
          int currValue = uS / US_ROUNDTRIP_CM;
          echoArray[current] = currValue;
}



void refreshDisplay(void){
    unsigned long currentMillis = millis();
  if (currentMillis - previousDisplayMillis >= intervalDisplayUpdate) {
      previousDisplayMillis = currentMillis;
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.println("  *  Pling Plong  *  ");
      display.setCursor(0,10);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.print("Trig:");
      display.print(TRIGGER_AT_CM);
      display.print(" Dist:");
      display.println(avgDist);
      display.println("Target: David");
      display.print("Counter: ");
      display.println(counter);
      if(triggered == 1){
        display.setTextColor(BLACK, WHITE);
        display.println("TRIGGERED");
        display.setTextColor(WHITE);
        display.print("Countdown: ");
        int sec = countdown/1000;
        if(sec > 60){
          byte tmpMin = sec/60;
          display.print(tmpMin);
          display.print("m ");
          display.print(sec-(60*tmpMin));
          display.println("s");
        } else {
          display.print(sec);
          display.println("s");
        }

      }
      display.display();
  }
}


void longPulse(){
  digitalWrite(TRANS_PIN, HIGH);
  delayMicroseconds(670);
  digitalWrite(TRANS_PIN,LOW);
  delayMicroseconds(190);
}

void shortPulse(){
  digitalWrite(TRANS_PIN, HIGH);
  delayMicroseconds(240);
  digitalWrite(TRANS_PIN, LOW);
  delayMicroseconds(620);
}




void ringDoorbell(){

  for (byte id = 0x00; id <=0x1F; id++)
  {
  
    for(byte i=1; i<= resendCount; i++){
      
        // Initialize Packet with 3 zeros
        for(byte i=0; i<3; i++){
          shortPulse();
        }
        
        // Custom doorbell Id
        for (byte mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
          if (id & mask)
            longPulse();
          else
            shortPulse();
        }

        // Send The DoorBell Signal
        byte e=1;
        for (int Tmask = 00000000000001; Tmask>0; Tmask <<= 1) { //iterate through bit mask
          if(e <= 13){  
            if (0x55F & Tmask)
              longPulse();
            else
              shortPulse();
          }
          e++;
        }
        delayMicroseconds(resendDelay);
    }
  }
  delayMicroseconds(10000);
  
}



