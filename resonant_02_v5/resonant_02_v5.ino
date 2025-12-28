#include <DFRobot_DF1201S.h>

#define DF1_TX 0
#define DF2_TX 8
#define DF1_RX 1
#define DF2_RX 9

DFRobot_DF1201S dfPlayer1;
DFRobot_DF1201S dfPlayer2;

enum State {
  IDLE,
  PLAYING,
  COOLDOWN
};

State currentState = IDLE;

String AudioFile = "a2.mp3";
String HapticFile = "h2.wav";
// Global variables
unsigned long lastSilentLoopTime = 0;
unsigned long silentFileDurationMs = 0;
bool silentLoopStarted = false;
const String SilentFile = "silent.mp3";  // Or whatever the filename is

bool useSecondPlayer = true;
bool silentLoopActive = false;

unsigned long lastPlayTime = 0;
const unsigned long cooldownAfterPlaybackMs = 5000;
unsigned long playbackStartMillis = 0;
unsigned long expectedPlaybackDuration = 0;

int hapticVolNormal = 18;
int hapticVolIntense = 25;
int audioVol = 25;

#define TRIGGER_PIN_1 A1
#define TRIGGER_PIN_2 A2

// Ultra-fast trigger detection
bool lastTrig1 = LOW;
bool lastTrig2 = LOW;
unsigned long lastTriggerTime = 0;
const unsigned long triggerCooldownMs = 400; // Very short cooldown

bool readyToTrigger = true;


void setup() {
  Serial.begin(115200);
  delay(200);

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

  dfPlayer1.setVol(5);
  dfPlayer1.switchFunction(dfPlayer1.MUSIC);
  dfPlayer2.switchFunction(dfPlayer2.MUSIC);

  dfPlayer1.playSpecFile("/" + AudioFile);
  delay(500);

  expectedPlaybackDuration = dfPlayer1.getTotalTime() * 1000UL;
  Serial.print("Audio file duration (ms): ");
  Serial.println(expectedPlaybackDuration);

  dfPlayer1.pause();
  dfPlayer1.setVol(0);
  dfPlayer1.setPlayMode(dfPlayer1.SINGLE);

dfPlayer2.setPlayMode(dfPlayer2.SINGLECYCLE);
dfPlayer2.setVol(0);
dfPlayer2.playSpecFile("/" + SilentFile);

// Wait until file info is available (you might already be doing this)
delay(500);
silentFileDurationMs = dfPlayer2.getTotalTime()* 1000UL;  // This reads duration in ms
lastSilentLoopTime = millis();
silentLoopStarted = true;

Serial.print("Silent file duration (ms): ");
Serial.println(silentFileDurationMs);


  dfPlayer2.setPlayMode(dfPlayer2.SINGLE);
  dfPlayer2.playSpecFile("/" + HapticFile);
  dfPlayer2.setVol(0);
  dfPlayer2.start();
  //loopFileNumPlayer2 = dfPlayer2.getCurFileNumber();

  pinMode(TRIGGER_PIN_1, INPUT);
  pinMode(TRIGGER_PIN_2, INPUT);

  delay(3000);
}

void loop() {
  unsigned long currentMillis = millis();

  // Read triggers
  bool trig1 = digitalRead(TRIGGER_PIN_1);
  bool trig2 = digitalRead(TRIGGER_PIN_2);
  bool anyHigh = trig1 || trig2;
  bool bothHigh = trig1 && trig2;

  // Re-arm trigger if all inputs are LOW
  if (!anyHigh && (currentMillis - lastTriggerTime > triggerCooldownMs)&& dfPlayer2.isPlaying()) {
    readyToTrigger = true;
  }

  switch (currentState) {
    case IDLE:
      if (anyHigh && readyToTrigger && (currentMillis - lastTriggerTime > triggerCooldownMs)) {
        readyToTrigger = false;  // lock re-trigger until full release

        // Haptic + audio start
        dfPlayer2.pause();
        delay(100);
        dfPlayer2.setVol(bothHigh ? hapticVolIntense : hapticVolNormal);
        delay(100);
        dfPlayer2.setPlayTime(1);
        dfPlayer2.playSpecFile("/" + HapticFile);
        delay(400);

        dfPlayer1.setVol(audioVol);
        dfPlayer1.playSpecFile("/" + AudioFile);

        playbackStartMillis = currentMillis;
        currentState = PLAYING;
        lastTriggerTime = currentMillis;
        Serial.println("Playback started INSTANTLY");
      }
      break;

    case PLAYING:
      if (bothHigh) {
        dfPlayer2.setVol(hapticVolIntense);
      } else if (anyHigh) {
        dfPlayer2.setVol(hapticVolNormal);
      }

      if (currentMillis - playbackStartMillis >= expectedPlaybackDuration) {
        dfPlayer2.setPlayMode(dfPlayer2.SINGLECYCLE);
        delay(100);
        dfPlayer2.playSpecFile("/" + SilentFile);
        dfPlayer1.setVol(0);
        dfPlayer2.setVol(0);
        currentState = COOLDOWN;
        lastPlayTime = currentMillis;
        Serial.println("Playback ended");
       
      }
      break;

    case COOLDOWN:
      if (currentMillis - lastPlayTime >= cooldownAfterPlaybackMs) {
        currentState = IDLE;
        dfPlayer2.setPlayMode(dfPlayer2.SINGLECYCLE);
        Serial.println("Cooldown ended");
        dfPlayer2.playSpecFile("/" + SilentFile);
          delay(500);

      }
      break;
  }

  // Ensure silent loop during IDLE
// Only restart silent file if it's *not already playing*
if (currentState == IDLE && !dfPlayer1.isPlaying()) {
  unsigned long currentMillis = millis();

  // Time to restart silent loop?
  if (silentLoopStarted && (currentMillis - lastSilentLoopTime > silentFileDurationMs + 200)) {
    dfPlayer2.setPlayMode(dfPlayer2.SINGLECYCLE);
    dfPlayer2.setVol(0);
    dfPlayer2.playSpecFile("/" + SilentFile);
    lastSilentLoopTime = currentMillis;
    Serial.println("Restarting silent loop");
  }
}



}
