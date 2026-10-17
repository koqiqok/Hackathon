
const int buttonPin = 2;

const int redLED = 13;
const int yellowLED = 11;
const int greenLED = 12;

int clickCount = 0;

bool lastReading = HIGH;
bool buttonState = HIGH;

unsigned long lastDebounce = 0;
unsigned long lastClick = 0;

const unsigned long debounceDelay = 40;
const unsigned long clickDelay = 700;

void allLEDsOff() {
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  allLEDsOff();

  Serial.begin(9600);

  Serial.println("AWARE LED TEST");
  Serial.println("1 Click  = RED");
  Serial.println("2 Clicks = YELLOW");
  Serial.println("3 Clicks = GREEN");
  Serial.println("6 Clicks = RESET");
}

void loop() {
  bool reading = digitalRead(buttonPin);

  // Debounce button
  if (reading != lastReading) {
    lastDebounce = millis();
    lastReading = reading;
  }

  if (millis() - lastDebounce > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        clickCount++;
        lastClick = millis();
      }
    }
  }

  // Execute after clicking stops
  if (clickCount > 0 &&
      buttonState == HIGH &&
      millis() - lastClick > clickDelay) {

    if (clickCount == 1) {
      allLEDsOff();
      digitalWrite(redLED, HIGH);
      Serial.println("RED LED ON");
    }

    else if (clickCount == 2) {
      allLEDsOff();
      digitalWrite(yellowLED, HIGH);
      Serial.println("YELLOW LED ON");
    }

    else if (clickCount == 3) {
      allLEDsOff();
      digitalWrite(greenLED, HIGH);
      Serial.println("GREEN LED ON");
    }

    else if (clickCount == 6) {
      allLEDsOff();
      Serial.println("ALERT RESET - ALL LEDs OFF");
    }

    else {
      Serial.println("No action for this click count");
    }

    // Clear click counter after every sequence
    clickCount = 0;
  }
}
