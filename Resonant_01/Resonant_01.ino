#include <DFRobot_DF1201S.h>

#define DF1_TX 0  // TX pin for DFPlayer 1
#define DF2_TX 8  // TX pin for DFPlayer 2
#define DF1_RX 1  // TX pin for DFPlayer 1
#define DF2_RX 9  // TX pin for DFPlayer 2

#define BUTTON_PIN 2

DFRobot_DF1201S dfPlayer1; //music
DFRobot_DF1201S dfPlayer2; //Haptic

String AudioFile = "a1.mp3";
String HapticFile = "h1.mp3";

bool hasPlayed = false;

bool useSecondPlayer = true; // Toggle to enable second DFPlayer

unsigned long lastPlayTime = 0;
bool isPlaybackBlocked = false;
const unsigned long cooldownDelay = 3000;  // Cooldown in milliseconds after playback ends

// Define pins
#define MODE_SELECT_PIN A0   // Analog pin to select mode
#define ANALOG_TRIGGER_PIN A2  // Analog sensor input
#define THRESHOLD 600          // Threshold for analog trigger

// Define trigger pins
#define TRIGGER_PIN_4 D2
#define TRIGGER_PIN_2 D3
#define TRIGGER_PIN_3 D6 // D9 -disable for secondary player
#define TRIGGER_PIN_1 D7 // D10 - disable for secondary player

// Serial communication with DFPlayer Pro


// Variables for tracking state

// Track previous states for edge detection
bool prevState[4] = {true, true, true, true};  // Assume initial state is HIGH (not triggered)

void setup() {
    Serial.begin(115200);
     delay(200);
   Serial.begin(115200);
    delay(100);

    Serial1.begin(115200);
      delay(100);
  Serial2.begin(115200);

  while (!dfPlayer1.begin(Serial1)) {
        Serial.println("DFPlayer1 Init failed!");
        delay(1000);
    }

 if (useSecondPlayer) {
        while (!dfPlayer2.begin(Serial2)) {
            Serial.println("DFPlayer2 Init failed!");
            delay(1000);
        }
   
    }

    dfPlayer1.setVol(0);
    dfPlayer1.switchFunction(dfPlayer1.MUSIC);
    if (useSecondPlayer) {
        dfPlayer2.setVol(0);
        dfPlayer2.switchFunction(dfPlayer2.MUSIC);
    }

    delay(1000);
    dfPlayer1.setPlayMode(dfPlayer1.SINGLE);
    if (useSecondPlayer) {
        dfPlayer2.setPlayMode(dfPlayer2.SINGLE);
    }

    pinMode(MODE_SELECT_PIN, INPUT);
    pinMode(ANALOG_TRIGGER_PIN, INPUT);
    pinMode(TRIGGER_PIN_1, INPUT_PULLUP);
    pinMode(TRIGGER_PIN_2, INPUT_PULLUP);
  //  pinMode(TRIGGER_PIN_3, INPUT_PULLUP);
  //  pinMode(TRIGGER_PIN_4, INPUT_PULLUP);

    delay(3000);
    dfPlayer1.pause();
    if (useSecondPlayer) dfPlayer2.pause();

    dfPlayer1.setVol(0);
    if (useSecondPlayer) dfPlayer2.setVol(0);
}


void loop() {
   
    // Unblock if both players are done + cooldown passed
    if (isPlaybackBlocked && !dfPlayer1.isPlaying() &&
        (!useSecondPlayer || !dfPlayer2.isPlaying())) {
        if (millis() - lastPlayTime > cooldownDelay) {
            isPlaybackBlocked = false;
        }
    }

    if (isPlaybackBlocked) return;

    for (int i = 0; i < 4; i++) {
        bool currentState = digitalRead(TRIGGER_PIN_1);

        if (currentState == HIGH && prevState[i] == LOW) {
           
                dfPlayer1.setVol(25);
                dfPlayer1.playSpecFile("/a1.mp3");
                if (useSecondPlayer) {
                    dfPlayer2.setVol(25);
                    dfPlayer2.playSpecFile("/h1.mp3");
                }
                lastPlayTime = millis();
                isPlaybackBlocked = true;
                break;
            
        }

        prevState[i] = currentState;
    }

    int sensorValue = analogRead(ANALOG_TRIGGER_PIN);
    if (sensorValue > THRESHOLD && !isPlaybackBlocked) {
        dfPlayer1.setVol(25);
        dfPlayer1.playSpecFile("/a1.mp3");
        if (useSecondPlayer) {
            dfPlayer2.setVol(25);
            dfPlayer2.playSpecFile("/h1.mp3");
        }
        lastPlayTime = millis();
        isPlaybackBlocked = true;
    }
}



