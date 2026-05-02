#define LED_PIN 2
#define BTN_PIN 18
#define POT_PIN 34

volatile int estado = 0;
volatile bool tick = false;
volatile bool ledState = false;
volatile int conteo = 0;

int lastBtnState = HIGH;

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  tick = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);
}

void loop() {

  int btnState = digitalRead(BTN_PIN);

  if (btnState == LOW && lastBtnState == HIGH) {
    estado++;
    if (estado > 2) estado = 0;

    if (estado == 2) {
      conteo   = 0;
      ledState = false;
      ledcWrite(LED_PIN, 0);
    }
  }
  lastBtnState = btnState;

  if (estado == 2 && tick) {
    portENTER_CRITICAL(&timerMux);
    tick = false;
    portEXIT_CRITICAL(&timerMux);

    ledState = !ledState;
    ledcWrite(LED_PIN, ledState ? 255 : 0);

    if (ledState == false) {
      conteo++;
      Serial.print("Parpadeo #");
      Serial.println(conteo);
    }

    if (conteo >= 5) {
      estado    = 0;
      conteo    = 0;
      ledState  = false;
      ledcWrite(LED_PIN, 0);
      Serial.println("Secuencia completa → Estado 0");
    }
  }

  switch (estado) {

    case 0:
      ledcWrite(LED_PIN, 0);
      break;

    case 1: {
      int valorADC = analogRead(POT_PIN);
      int pwmValue = map(valorADC, 0, 4095, 0, 255);
      ledcWrite(LED_PIN, pwmValue);
      break;
    }

    case 2:
      break;
  }
}