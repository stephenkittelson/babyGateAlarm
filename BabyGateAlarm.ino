const int SPEAKER = 8;
const int GATE_SENSOR = 2;
const int ARMED_LIGHTS[] = {5, 13, 12, 4, 7, 11, 10, 9, 6};
const int NUM_ARMED_LIGHTS = 9;

#define LOG_ENABLED 0

enum SensorState {
  Waiting,
  Silent,
  Alarm
};

class Logger {
public:
  void log(char* str) {
#if LOG_ENABLED
    Serial.println(str);
#endif
  }

  void setup() {
#if LOG_ENABLED
    Serial.begin(9600);
#endif
  }
};

class Button {
private:
  unsigned int timeOfLastButtonPush = 0;
  int lastReading = LOW;
  int timeOfLastChange = 0;
  int currentState = LOW;
  int pin;
  const int DEBOUNCE_DELAY = 50;

public:
  Button(int pin) {
    this->pin = pin;
  }

  int getDebouncedSwitchState() {
    int currentReading = digitalRead(pin);
    if (currentReading != lastReading) {
      timeOfLastChange = millis();
    } else if ((millis() - timeOfLastChange) > DEBOUNCE_DELAY && currentReading != currentState) {
      currentState = currentReading;
    }
    lastReading = currentReading;
    return currentState;
  }
};

class Light {
private:
  unsigned int timeOfLastLightChange = 0;
  int currentLightState = LOW;
  int blinkTime = 0;
  unsigned int timeOfLastAlarmFlash = 0;
  bool alarmFlashPhase = false;
  const unsigned int ALARM_FLASH_INTERVAL = 200;

public:
  void turnOn() {
    currentLightState = HIGH;
    for (int i = 0; i < NUM_ARMED_LIGHTS; i++) {
      digitalWrite(ARMED_LIGHTS[i], currentLightState);
    }
  }

  void blink() {
    unsigned int currentTime = millis();
    if ((currentTime - timeOfLastLightChange) > 50) {
      unsigned short lightStates = 0b0111 << blinkTime;
      lightStates = (lightStates | ((lightStates & 0xFE00) >> NUM_ARMED_LIGHTS)) & 0x01FF;
      blinkTime = (blinkTime + 1) % NUM_ARMED_LIGHTS;
      for (int i = 0; i < NUM_ARMED_LIGHTS; i++) {
        int lightState;
        if (lightStates & (1 << i)) {
          lightState = HIGH;
        } else {
          lightState = LOW;
        }
        digitalWrite(ARMED_LIGHTS[i], lightState);
      }
      timeOfLastLightChange = currentTime;
    }
  }

  // Alternates every-other light on/off, swapping which set is lit
  // every ALARM_FLASH_INTERVAL (200ms). Call this repeatedly while
  // the alarm is firing.
  void flashAlternate() {
    unsigned int currentTime = millis();
    if ((currentTime - timeOfLastAlarmFlash) > ALARM_FLASH_INTERVAL) {
      alarmFlashPhase = !alarmFlashPhase;
      for (int i = 0; i < NUM_ARMED_LIGHTS; i++) {
        bool isEvenIndex = (i % 2 == 0);
        int lightState = (isEvenIndex == alarmFlashPhase) ? HIGH : LOW;
        digitalWrite(ARMED_LIGHTS[i], lightState);
      }
      timeOfLastAlarmFlash = currentTime;
    }
  }
};

Light light;
Button gateButton(GATE_SENSOR);
Logger logger;

SensorState currentState = Silent;
unsigned int timeOfGateOpen = 0;

void setup() {
  pinMode(SPEAKER, OUTPUT);
  pinMode(GATE_SENSOR, INPUT);
  for (int i = 0; i < NUM_ARMED_LIGHTS; i++) {
    pinMode(ARMED_LIGHTS[i], OUTPUT);
  }
  logger.setup();
  light.turnOn();
}

void loop() {
  unsigned int currentTime = millis();
  char str[100];
  sprintf(str, "time: %u, currentState: %d, switch state: %d", currentTime, currentState, gateButton.getDebouncedSwitchState());
  logger.log(str);

  if (currentState == Silent) {
    if (gateButton.getDebouncedSwitchState() == LOW) {
      currentState = Waiting;
      timeOfGateOpen = currentTime;
    }
  } else if (currentState == Waiting) {
    if (gateButton.getDebouncedSwitchState() == HIGH) {
      currentState = Silent;
      light.turnOn();
    } else if (currentTime - timeOfGateOpen > 7000) {
      currentState = Alarm;
      light.turnOn();
    } else {
      light.blink();
    }
  } else if (currentState == Alarm) {
    if (gateButton.getDebouncedSwitchState() == HIGH) {
      currentState = Silent;
      light.turnOn();
    }
    light.flashAlternate();
    tone(SPEAKER, 100, 100);
  }
}
