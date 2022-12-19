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

unsigned int timeOfGateOpen = 0;

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
        switch (currentState) {
          case HIGH:
            return STATE_HIGH;
          case LOW:
            return STATE_LOW;
        }
      }
      lastReading = currentReading;
      return STATE_UNCHANGED;
    }
};

Button gateButton(GATE_SENSOR);

void setup() {
  pinMode(SPEAKER, OUTPUT);
  pinMode(GATE_SENSOR, INPUT);
  pinMode(ARMED_LIGHT, OUTPUT);
}

void loop() {
  unsigned int currentTime = millis();
  if (currentState == SILENT) {
    if (gateButton.getDebouncedSwitchState() == STATE_LOW) {
      currentState = WAITING;
      timeOfGateOpen = currentTime;
    }
  } else if (currentState == WAITING) {
    if (gateButton.getDebouncedSwitchState() == STATE_HIGH) {
      currentState = SILENT;
      currentLightState = HIGH;
      digitalWrite(ARMED_LIGHT, currentLightState);
    } else if (currentTime - timeOfGateOpen > 7000) {
      currentState = ALARM;
      currentLightState = HIGH;
      digitalWrite(ARMED_LIGHT, currentLightState);
    } else if ((currentTime - timeOfLastLightChange) > 1000) {
      currentLightState = !currentLightState;
      digitalWrite(ARMED_LIGHT, currentLightState);
      timeOfLastLightChange = currentTime;
    }
  } else if (currentState == ALARM) {
    if (gateButton.getDebouncedSwitchState() == STATE_HIGH) {
      currentState = SILENT;
    }
    tone(SPEAKER, 100, 100);
  }
}
