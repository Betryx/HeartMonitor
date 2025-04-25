#include <PulseSensorPlayground.h>

const int PulseWire = A4; // Input for PulseSensor
const int ECGWire = A5;   // Input for AD8232
const int LED = LED_BUILTIN;
const int LOPlus = 10;  // Electrode detection +
const int LOMinus = 11; // Electrode detection -
int Threshold = 515;
int OperationMode = -1; // Selector for operation mode
PulseSensorPlayground pulseSensor;

unsigned long previousMillis = 0; // Time delay for ECG signal reading
const long interval = 50; // Interval for ECG reading (milliseconds)

const int sampleCount = 5; // Number of samples for averaging
int bpmArray[sampleCount]; 
int currentIndex = 0; 
int bpmSum = 0; 
int lastBPM = 0; 

void setup() {
  Serial.begin(1000000);
  pinMode(LOPlus, INPUT);
  pinMode(LOMinus, INPUT);

  pulseSensor.begin();
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED);
  pulseSensor.setThreshold(Threshold);

  showHomeMenu();

  while (true) {
    if (Serial.available()) { // Check for available serial interface through which the development board communicates with the computer
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toUpperCase();

      if (command == "BPM") {
        OperationMode = 0; 
        Serial.println("Entering BPM mode...");
        break;
      } else if (command == "EKG") {
        OperationMode = 1; 
        Serial.println("Entering EKG mode...");
        break;
      } else if (command == "HOME") {
        OperationMode = -1;  
        showHomeMenu();
        break;
      } else {
        Serial.println("Invalid command. Enter 'BPM' for BPM mode, 'EKG' for EKG mode, or 'HOME'.");
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

// Home menu
void showHomeMenu() {
  Serial.println("\n------ Home Menu ------");
  Serial.println("Enter 'BPM' to measure heart rate");
  Serial.println("Enter 'EKG' to display ECG signal graph");
  Serial.println("Enter 'HOME' to return to this menu");
  Serial.println("-------------------------");
}

// Heart rate measurement mode
void showBPMMode() {
  static bool printedHeader = false; 

  if (!printedHeader) {
    Serial.println("\n------ In BPM Mode ------");
    Serial.println("Measuring heart rate...");
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
        Serial.print("Heart rate (BPM): ");
        Serial.println(averageBPM);
        lastBPM = averageBPM; 
      }
    }
  }
}

// Electrocardiogram mode
void showEKGMode() {
  static bool printedHeader = false; 
  static bool warningDisplayed = false;

  // Check for electrodes
  if (digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH) {
    Serial.println("\n❌ No electrode group detected: attach the electrode group and try again.");
    OperationMode = -1; 
    showHomeMenu();
    return;
  }

  if (!warningDisplayed) {
    Serial.println("\n⚠️ Electrode group detected");
    Serial.println("📊 ECG is displayed in the serial plotter");
    warningDisplayed = true;
  }

  if (!printedHeader) {
    Serial.println("\n------ In EKG Mode ------");
    Serial.println("Displaying ECG signal graph...");
    printedHeader = true;
  }

  // Plot ECG signal
  int ecgSignal = analogRead(ECGWire);
  Serial.println(-1*ecgSignal);
  delay(40);
}
