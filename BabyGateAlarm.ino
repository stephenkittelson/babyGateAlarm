const int SPEAKER = 8;
const int GATE_SENSOR = 2;
const int ARMED_LIGHT = 7;

const int STATE_HIGH = 5;
const int STATE_LOW = 6;
const int STATE_UNCHANGED = 7;

const int WAITING = 10;
const int SILENT = 11;
const int ALARM = 12;

int currentState = SILENT;

bool isArmed = true;
unsigned int timeOfLastDisarmPush = 0;

unsigned int timeOfLastLightChange = 0;
int currentLightState = LOW;

void setup() {
  pinMode(SPEAKER, OUTPUT);
  pinMode(GATE_SENSOR, INPUT);
  pinMode(ARMED_LIGHT, OUTPUT);
}

void loop() {
  unsigned int currentTime = millis();
  /*
  three states:
    silent
    waiting
    alarm

  if in silent state:
    if gate switch is not pressed:
      switch to waiting state
  else if in waiting state:
    if gate switch is pressed:
      switch to silent state
    else if time has elapsed:
      switch to alarm state
    blink the light
  else if in alarm state:
    if gate switch is pressed:
      switch to silent state
    alarm


  */
  if (getDebouncedDisarmSwitchState() == STATE_HIGH) {
    timeOfLastDisarmPush = currentTime;
    isArmed = false;
  }
  if (!isArmed && (currentTime - timeOfLastDisarmPush) > 30000) {
      isArmed = true;
  }

  if (isArmed && currentLightState == LOW) {
    currentLightState = HIGH;
    digitalWrite(ARMED_LIGHT, currentLightState);
  } else if (!isArmed) {
    if ((currentTime - timeOfLastLightChange) > 1000) {
      currentLightState = !currentLightState;
      digitalWrite(ARMED_LIGHT, currentLightState);
      timeOfLastLightChange = currentTime;
    }
  }

  if (isArmed && digitalRead(GATE_SENSOR) == LOW) {
    // tone(SPEAKER, 100, 100);
  }
}

int lastDisarmReading = LOW;
int timeOfLastDisarmChange = 0;
int currentDisarmState = LOW;
const int DEBOUNCE_DELAY = 50;

int getDebouncedDisarmSwitchState() {
  int currentDisarmReading = digitalRead(DISARM);
  if (currentDisarmReading != lastDisarmReading) {
    timeOfLastDisarmChange = millis();
  } else if ((millis() - timeOfLastDisarmChange) > DEBOUNCE_DELAY && currentDisarmReading != currentDisarmState) {
    currentDisarmState = currentDisarmReading;
    switch (currentDisarmState) {
      case HIGH:
        return STATE_HIGH;
      case LOW:
        return STATE_LOW;
    }
  }
  lastDisarmReading = currentDisarmReading;
  return STATE_UNCHANGED;
}
