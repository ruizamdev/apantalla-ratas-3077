/*
 * Motorcycle Anti-Theft System: Apantalla-Ratas™ 3077
 * 
 * This system implements a motorcycle security solution using a PIR motion sensor.
 * Key features and functionality:
 * - PIR sensor for motion detection with false positive prevention
 * - LCD display for system status and alerts
 * - Toggle button for arming/disarming the system
 * - Status LED indicating system state:
 *   * OFF: System disarmed
 *   * Blinking: System armed and monitoring
 *   * Solid ON: Alarm triggered
 * - Multiple warning LEDs that flash in sequence when alarm is triggered
 * - Buzzer for audible alarm
 * - Alarm can be cancelled by pressing the toggle button
 * - Debounce protection for reliable button operation
 * 
 * Operation:
 * 1. Press button to arm/disarm the system
 * 2. When armed, the status LED will blink to indicate active monitoring
 * 3. If motion is detected, system confirms it with multiple readings to avoid false alarms
 * 4. When intruder is detected, the system:
 *    - Activates the buzzer
 *    - Displays warning message on LCD
 *    - Flashes warning LEDs in sequence
 *    - Keeps alarm active for a set duration or until manually disabled
 * 
 * =============== Español ===============
 * 
 * Sistema Anti-Robo para Motocicleta: Apantalla-Ratas™ 3077
 * 
 * Este sistema implementa una solución de seguridad para motocicletas usando un sensor de movimiento PIR.
 * Características y funcionalidad principales:
 * - Sensor PIR para detección de movimiento con prevención de falsos positivos
 * - Pantalla LCD para estado del sistema y alertas
 * - Botón de alternancia para activar/desactivar el sistema
 * - LED de estado indicando el estado del sistema:
 *   * APAGADO: Sistema desactivado
 *   * Parpadeando: Sistema armado y monitoreando
 *   * Encendido fijo: Alarma activada
 * - Múltiples LEDs de advertencia que parpadean en secuencia cuando se activa la alarma
 * - Zumbador para alarma audible
 * - La alarma puede cancelarse presionando el botón de alternancia
 * - Protección contra rebotes para una operación confiable del botón
 * 
 * Operación:
 * 1. Presionar botón para activar/desactivar el sistema
 * 2. Cuando está armado, el LED de estado parpadea para indicar monitoreo activo
 * 3. Si se detecta movimiento, el sistema lo confirma con múltiples lecturas para evitar falsas alarmas
 * 4. Cuando se detecta un intruso, el sistema:
 *    - Activa el zumbador
 *    - Muestra mensaje de advertencia en LCD
 *    - Hace parpadear los LEDs de advertencia en secuencia
 *    - Mantiene la alarma activa por una duración establecida o hasta que se desactive manualmente
 */

// Sistema Anti-Robo para Motocicleta: Apantalla-Ratas™ 3077

#include <LiquidCrystal.h>

#include <DFRobotDFPlayerMini.h>
DFRobotDFPlayerMini myDFPlayer;

// LCD pin setup: RS, E, D4, D5, D6, D7
// LCD pin configuration: RS, E, D4, D5, D6, D7
// Configuración de pines para la pantalla LCD: RS, E, D4, D5, D6, D7
const int rs = 10, en = 11, d4 = 12, d5 = 13, d6 = 14, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pin assignments
// Pin assignments for system components
// Asignación de pines para los componentes del sistema
const int pirPin = 2;                   // PIR sensor OUT / Salida del sensor PIR
const int buzzerPin = 3;                // Buzzer control / Control del zumbador
const int alarmLeds[] = {4, 5, 8, 9};   // Dissuasive LEDs / LEDs disuasivos
const int buttonPin = 6;                // Push button toggle / Botón de alternancia
const int statusLedPin = 7;             // Armed/Disarmed status LED / LED de estado armado/desarmado

// System states
// System state variables
// Variables de estado del sistema
bool systemArmed = false;   // True if system is armed / Verdadero si el sistema está armado
bool alarmActive = false;   // True if alarm is active / Verdadero si la alarma está activa

// Button debounce
// Variables for button debounce logic
// Variables para la lógica de anti-rebote del botón
bool previousButtonState = LOW;           // Last button state / Último estado del botón
unsigned long lastDebounceTime = 0;      // Last debounce timestamp / Última marca de tiempo de anti-rebote
const int debounceDelay = 200;           // Debounce delay in ms / Retardo de anti-rebote en ms

// Alarm duration
// Duration for which the alarm stays active (ms)
// Duración durante la cual la alarma permanece activa (ms)
const int alarmDuration = 5000; // ms

void setup() {
  // Initialize serial communication for debugging
  // Inicializa la comunicación serial para depuración
  Serial.begin(9600);

  // Initialize LCD display
  // Inicializa la pantalla LCD
  lcd.begin(16, 2);
  // lcd.print("Hello world!");
  lcd.setCursor(0, 0);
  lcd.print("Apantalla Ratas");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000); // Wait for 2 seconds / Espera 2 segundos
  lcd.clear();

  // Set pin modes for all components
  // Configura los modos de los pines para todos los componentes
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(statusLedPin, OUTPUT);

  // Set pin modes for each alarm LED and turn them off
  // Configura los pines de los LEDs de alarma y los apaga
  for (int i = 0; i < sizeof(alarmLeds) / sizeof(alarmLeds[0]); i++) {
    pinMode(alarmLeds[i], OUTPUT);
    digitalWrite(alarmLeds[i], LOW);
  }

  // Ensure buzzer and status LED are off at start
  // Asegura que el zumbador y el LED de estado estén apagados al inicio
  digitalWrite(buzzerPin, LOW);
  digitalWrite(statusLedPin, LOW);
}

// Confirm movement (avoid false positives)
// Function to confirm movement by checking the PIR sensor multiple times
// Función para confirmar movimiento revisando el sensor PIR varias veces
bool isMovementConfirmed() {
  int count = 0; // Counter for positive PIR readings / Contador de lecturas positivas del PIR
  for (int i = 0; i < 5; i++) {
    // Read PIR sensor and increment count if motion detected
    // Lee el sensor PIR y aumenta el contador si detecta movimiento
    if (digitalRead(pirPin) == HIGH) {
      count++;
    }
    delay(50); // Wait 50 ms between readings / Espera 50 ms entre lecturas
  }
  // Return true if at least 3 out of 5 readings detected motion
  // Devuelve verdadero si al menos 3 de 5 lecturas detectaron movimiento
  return (count >= 3);
}

void loop() {
  // Handle button toggle with debounce
  // Maneja el cambio de estado del botón con anti-rebote
  bool buttonState = digitalRead(buttonPin); // Read current button state / Lee el estado actual del botón
  if (buttonState != previousButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis(); // Update debounce timer / Actualiza el temporizador de anti-rebote
    if (buttonState == HIGH) {
      // Toggle system armed state
      // Alterna el estado armado del sistema
      systemArmed = !systemArmed;
      Serial.print("system in now ");
      Serial.println(systemArmed ? "ARMED" : "DISARMED");

      // Update LCD with system status
      // Actualiza la pantalla LCD con el estado del sistema
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Status:");
      lcd.setCursor(0, 1);
      // lcd.print(millis() / 1000);
      lcd.print(systemArmed ? "ARMED" : "DISARMED");
    }
    previousButtonState = buttonState; // Save button state / Guarda el estado del botón
  }

  // Show armed status
  // Muestra el estado armado
  if (!systemArmed) {
    digitalWrite(statusLedPin, LOW); // off / apagado
  } else if (!alarmActive) {
    // Blinking when armed
    // Parpadea cuando está armado
    digitalWrite(statusLedPin, HIGH);
    delay(100);
    digitalWrite(statusLedPin, LOW);
    delay(900);
  }

  // Trigger alarm if movement detected
  // Activa la alarma si se detecta movimiento
  if (systemArmed && isMovementConfirmed()) {
    alarmActive = true; // Set alarm as active / Marca la alarma como activa
    Serial.println("Intruder detected - alarm triggered!");

    // Show alarm message on LCD
    // Muestra mensaje de alarma en la pantalla LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! INTRUDER !!!");
    lcd.setCursor(0, 1);
    lcd.print("ALARM TRIGGERED");

    unsigned long startTime = millis(); // Record alarm start time / Registra el tiempo de inicio de la alarma
    while (millis() - startTime < alarmDuration) {
      digitalWrite(buzzerPin, HIGH); // Activate buzzer / Activa el zumbador

      // Fancy LED flashing
      // Secuencia de parpadeo de LEDs
      for (int i = 0; i < sizeof(alarmLeds) / sizeof(alarmLeds[0]); i++) {
        digitalWrite(alarmLeds[i], HIGH); // Turn on LED / Enciende el LED
        delay(70);                        // Wait 70 ms / Espera 70 ms
        digitalWrite(alarmLeds[i], LOW);  // Turn off LED / Apaga el LED
      }

      digitalWrite(statusLedPin, HIGH); // Solid ON while alarm active / Encendido fijo mientras la alarma está activa

      // Allow alarm cancellation by button
      // Permite cancelar la alarma con el botón
      bool currentButtonState = digitalRead(buttonPin);
      if (currentButtonState != previousButtonState && (millis() - lastDebounceTime) > debounceDelay){
        lastDebounceTime = millis();
        if (currentButtonState == HIGH) {
          systemArmed = false; // Disarm system / Desarma el sistema
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("System DISARMED");
          lcd.setCursor(0, 1);
          lcd.print("Alarm aborted");
          break; // Exit alarm loop / Sale del ciclo de alarma
        }
        previousButtonState = currentButtonState;
      }
    }
  }
}