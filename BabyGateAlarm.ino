const int SPEAKER = 8;
const int GATE_SENSOR = 2;
const int ARM_TOGGLE = 4;
const int ARMED_LIGHT = 7;

bool isArmed = true;

int lastArmToggleReading = LOW;
int timeOfLastArmToggleChange = 0;
bool readButtonPress = false;
int currentArmToggleState = LOW;
const int DEBOUNCE_DELAY = 50;

void setup() {
  pinMode(SPEAKER, OUTPUT);
  pinMode(GATE_SENSOR, INPUT);
  pinMode(ARM_TOGGLE, INPUT);
  pinMode(ARMED_LIGHT, OUTPUT);
}

void loop() {
  int currentArmToggleReading = digitalRead(ARM_TOGGLE);
  if (currentArmToggleReading != lastArmToggleReading) {
    timeOfLastArmToggleChange = millis();
  } else if ((millis() - timeOfLastArmToggleChange) > DEBOUNCE_DELAY && currentArmToggleReading != currentArmToggleState) {
    currentArmToggleState = currentArmToggleReading;
    if (currentArmToggleState == HIGH) {
      isArmed = !isArmed;
    }
  }
  lastArmToggleReading = currentArmToggleReading;

  if (isArmed) {
    digitalWrite(ARMED_LIGHT, HIGH);
  } else {
    digitalWrite(ARMED_LIGHT, LOW);
  }

  if (isArmed && digitalRead(GATE_SENSOR) == LOW) {
    tone(SPEAKER, 100, 1000);
  }
}
