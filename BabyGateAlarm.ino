const int SPEAKER = 8;
const int GATE_SENSOR = 2;
const int ARMED_LIGHT = 7;

enum SensorState {
  Waiting,
  Silent,
  Alarm
};

class Logger {
  private:
    bool enabled = false;
  public:
    void log(char* str) {
      if (enabled) {
        Serial.println(str);
      }
    }

    void setup() {
      if (enabled) {
        Serial.begin(9600);
      }
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
  public:
    void turnOn() {
      currentLightState = HIGH;
      digitalWrite(ARMED_LIGHT, currentLightState);
    }

    void blink() {
      unsigned int currentTime = millis();
      if ((currentTime - timeOfLastLightChange) > 500) {
        currentLightState = !currentLightState;
        digitalWrite(ARMED_LIGHT, currentLightState);
        timeOfLastLightChange = currentTime;
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
  pinMode(ARMED_LIGHT, OUTPUT);
  Serial.begin(9600);
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
    }
    tone(SPEAKER, 100, 100);
  }
}
