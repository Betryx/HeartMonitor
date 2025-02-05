#include <PulseSensorPlayground.h>

const int PulseWire = A4; // Вход за PulseSensor
const int ECGWire = A5;   // Вход за AD8232
const int LED = LED_BUILTIN;
const int LOPlus = 10;  // Детекция на електроди +
const int LOMinus = 11; // Детекция на електроди  -
int Threshold = 515;
int OperationMode = -1; // Селектор за режим на работа
PulseSensorPlayground pulseSensor;

unsigned long previousMillis = 0; // Време за забавяне на четене на ЕКГ сигнала
const long interval = 50; // Интервал за четене на ЕКГ (милисекунди)

const int sampleCount = 5; // Брой проби за усредняване
int bpmArray[sampleCount]; 
int currentIndex = 0; 
int bpmSum = 0; 
int lastBPM = 0; 

void setup() {
  Serial.begin(115200);
  pinMode(LOPlus, INPUT);
  pinMode(LOMinus, INPUT);

  pulseSensor.begin();
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED);
  pulseSensor.setThreshold(Threshold);

  showHomeMenu();

  while (true) {
    if (Serial.available()) { // Проверка за наличен сериен интерфейс посредством който развойната платка комуникира с компютъра
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toUpperCase();

      if (command == "BPM") {
        OperationMode = 0; 
        Serial.println("Влизате в режим BPM...");
        break;
      } else if (command == "EKG") {
        OperationMode = 1; 
        Serial.println("Влизате в режим EKG...");
        break;
      } else if (command == "HOME") {
        OperationMode = -1;  
        showHomeMenu();
        break;
      } else {
        Serial.println("Невалидна команда. Въведете 'BPM' за BPM режим, 'EKG' за EKG режим или 'HOME'.");
      }
    }
  }
}

void loop() {
  if (OperationMode == 0) {
    showBPMMode();  
  } else if (OperationMode == 1) {
    showEKGMode();  
  }
}

// Начално меню
void showHomeMenu() {
  Serial.println("\n------ Начално меню ------");
  Serial.println("Въведете 'BPM' за измерване на пулс");
  Serial.println("Въведете 'EKG' за графика на ЕКГ сигнал");
  Serial.println("Въведете 'HOME' за връщане в това меню");
  Serial.println("-------------------------");
}

// Режим измерване на пулс
void showBPMMode() {
  static bool printedHeader = false; 

  if (!printedHeader) {
    Serial.println("\n------ В режим BPM ------");
    Serial.println("Измерваме сърдечния ритъм...");
    printedHeader = true;
  }

  if (pulseSensor.sawStartOfBeat()) {
    int currentBPM = pulseSensor.getBeatsPerMinute();

    if (currentBPM > 40 && currentBPM < 220) {
      bpmSum -= bpmArray[currentIndex];  
      bpmArray[currentIndex] = currentBPM; 
      bpmSum += currentBPM; 
      currentIndex = (currentIndex + 1) % sampleCount; 

      int averageBPM = bpmSum / sampleCount; 
      if (lastBPM != averageBPM) { 
        Serial.print("Пулс (BPM): ");
        Serial.println(averageBPM);
        lastBPM = averageBPM; 
      }
    }
  }
}

// Режим Електрокардиограма
void showEKGMode() {
  static bool printedHeader = false; 
  static bool warningDisplayed = false;

  // Проверка за електроди
  if (digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH) {
    Serial.println("\n❌ Липса електродна група: закачете електродна група и опитайте отново.");
    OperationMode = -1; 
    showHomeMenu();
    return;
  }

  if (!warningDisplayed) {
    Serial.println("\n⚠️ Електродна група засечена");
    Serial.println("📊 ЕКГ се визуализира в сериен плотер");
    warningDisplayed = true;
  }

  if (!printedHeader) {
    Serial.println("\n------ В режим EKG ------");
    Serial.println("Показваме графика на ЕКГ сигнала...");
    printedHeader = true;
  }

  // Плотване на ЕКГ сигнал
 int ecgSignal = analogRead(ECGWire);
    Serial.println(ecgSignal);
    delay(150);
}
