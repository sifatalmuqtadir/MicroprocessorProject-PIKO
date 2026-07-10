
#include <Servo.h>
#include <AFMotor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define Echo A0
#define Trig A1
#define motor 10
#define Speed 200
#define spoint 90
#define BUZZER 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define OBSTACLE_THRESHOLD_CM 20
#define NO_ECHO_VALUE 999
#define ULTRASONIC_TIMEOUT_US 30000
#define SERVO_LEFT_ANGLE 180
#define SERVO_RIGHT_ANGLE 20
#define SERVO_SETTLE_DELAY 600
#define SERVO_SCAN_DELAY 1000
#define SERVO_READ_SETTLE_DELAY 200
#define BACKUP_MOVE_DELAY 500
#define TURN_MOVE_DELAY 600
#define DEFAULT_TURN_DELAY 800
#define POST_MOVE_STOP_DELAY 300
#define RESULT_DISPLAY_DELAY 500
#define BUZZER_BEEP_ON_DELAY 300
#define BUZZER_BEEP_OFF_DELAY 200
#define STARTUP_LOGO_DELAY 3000
#define STARTUP_BEEP_ON_DELAY 100
#define STARTUP_BEEP_OFF_DELAY 100
#define VOICE_SCAN_DELAY 800
#define VOICE_SETTLE_DELAY 500
#define VOICE_BLOCK_BEEP_DELAY 150
#define VOICE_MIN_CLEARANCE_CM 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char value;
int distance;
int L = 0;
int R = 0;
int Left;
int Right;

Servo servo;
AF_DCMotor M1(1);
AF_DCMotor M2(2);
AF_DCMotor M3(3);
AF_DCMotor M4(4);

void drawOLEDHeader() 
{
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  display.println("PIKO");
  display.drawLine(0, 17, 128, 17, SSD1306_WHITE);
}

void drawOLEDStatusLine(String status) 
{
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(status);
}

void drawOLEDDetailLines(String line2, String line3) 
{
  display.setTextSize(1);
  display.setCursor(0, 42);
  display.println(line2);

  display.setCursor(0, 54);
  display.println(line3);
}

void showOLED(String status, String line2, String line3) 
{
  display.clearDisplay();
  drawOLEDHeader();
  drawOLEDStatusLine(status);

  drawOLEDDetailLines(line2, line3);
  display.display();
}

void triggerUltrasonicPulse() 
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(4);
  digitalWrite(Trig, HIGH);

  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
}

long readUltrasonicPulseDuration() 
{
  return pulseIn(Echo, HIGH, ULTRASONIC_TIMEOUT_US);
}

int convertPulseDurationToDistanceCM(long pulseDuration) 
{
  if (pulseDuration == 0) 
  {
    return NO_ECHO_VALUE;
  }
  return pulseDuration / 29 / 2;
}

int ultrasonic() 
{
  triggerUltrasonicPulse();
  long t = readUltrasonicPulseDuration();
  return convertPulseDurationToDistanceCM(t);
}

void setAllMotorsSpeed(int speedValue) 
{
  M1.setSpeed(speedValue);
  M2.setSpeed(speedValue);
  M3.setSpeed(speedValue);
  M4.setSpeed(speedValue);
}

void releaseAllMotors() 
{
  M1.run(RELEASE);
  M2.run(RELEASE);
  M3.run(RELEASE);
  M4.run(RELEASE);
}

void forward() 
{
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void backward() 
{
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}

void right() 
{
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void left() 
{
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}

void Stop() 
{
  releaseAllMotors();
}

void initializeSerialCommunication() 
{
  Serial.begin(9600);
}

void initializeI2CBus() 
{
  Wire.begin();
}

void configureUltrasonicPins() 
{
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
}

void configureBuzzerPin() 
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

void initializeSteeringServo() 
{
  servo.attach(motor);
  servo.write(spoint);
}

void initializeDriveMotors() 
{
  setAllMotorsSpeed(Speed);
  releaseAllMotors();
}

void initializeOLEDDisplay() 
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void showStartupSplashScreen() 
{
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18, 5);

  display.println("PIKO");
  display.setTextSize(1);
  display.setCursor(20, 52);

  display.println("Robot Car v1.0");
  display.display();
  delay(STARTUP_LOGO_DELAY);
}

void playStartupBeepSequence() 
{
  digitalWrite(BUZZER, HIGH);
  delay(STARTUP_BEEP_ON_DELAY);
  digitalWrite(BUZZER, LOW);
  delay(STARTUP_BEEP_OFF_DELAY);
  digitalWrite(BUZZER, HIGH);
  delay(STARTUP_BEEP_ON_DELAY);
  digitalWrite(BUZZER, LOW);
}

void setup() 
{
  initializeSerialCommunication();
  initializeI2CBus();

  configureUltrasonicPins();
  configureBuzzerPin();

  initializeSteeringServo();
  initializeDriveMotors();

  initializeOLEDDisplay();
  showStartupSplashScreen();

  playStartupBeepSequence();
}

void loop() 
{
  Obstacle();
  // Bluetoothcontrol();
  // voicecontrol();
}

void logDistanceToSerial(int measuredDistance) 
{
  Serial.print("Distance: ");
  Serial.println(measuredDistance);
}

bool isPathClear(int measuredDistance) 
{
  return (measuredDistance > OBSTACLE_THRESHOLD_CM || measuredDistance == NO_ECHO_VALUE);
}

void handleClearPath(int measuredDistance) 
{
  forward();
  if (measuredDistance == NO_ECHO_VALUE) 
  {
    showOLED("FORWARD", "Dist: Clear", "Path is clear!");
  } else {
    showOLED("FORWARD", "Dist: " + String(measuredDistance) + " cm", "Path is clear!");
  }
}

void announceObstacleOnDisplay(int measuredDistance) 
{
  showOLED("OBSTACLE!", "Dist: " + String(measuredDistance) + " cm", "Object detected!");
}

void soundObstacleAlertBuzzer() 
{
  digitalWrite(BUZZER, HIGH);
  delay(BUZZER_BEEP_ON_DELAY);
  digitalWrite(BUZZER, LOW);
  delay(BUZZER_BEEP_OFF_DELAY);
}

void performBackwardManeuver(int measuredDistance) 
{
  backward();
  showOLED("BACKWARD", "Dist: " + String(measuredDistance) + " cm", "Moving back...");
  delay(BACKUP_MOVE_DELAY);
  Stop();
  delay(POST_MOVE_STOP_DELAY);
}

int scanLeftSideDistance() 
{
  showOLED("SCANNING", "Checking LEFT...", "Please wait...");
  servo.write(SERVO_LEFT_ANGLE);
  delay(SERVO_SCAN_DELAY);
  int measuredLeft = ultrasonic();
  delay(SERVO_READ_SETTLE_DELAY);
  servo.write(spoint);
  delay(SERVO_SETTLE_DELAY);
  return measuredLeft;
}

int scanRightSideDistance(int currentLeftValue) 
{
  showOLED("SCANNING", "Checking RIGHT...", "Left: " + String(currentLeftValue) + " cm");
  servo.write(SERVO_RIGHT_ANGLE);
  delay(SERVO_SCAN_DELAY);
  int measuredRight = ultrasonic();
  delay(SERVO_READ_SETTLE_DELAY);
  servo.write(spoint);
  delay(SERVO_SETTLE_DELAY);
  return measuredRight;
}

void logScanResultsToSerial(int leftValue, int rightValue) 
{
  Serial.print("Left: "); Serial.println(leftValue);
  Serial.print("Right: "); Serial.println(rightValue);
}

void displayScanResultSummary(int leftValue, int rightValue) 
{
  showOLED("RESULT", "L:" + String(leftValue) + " R:" + String(rightValue), "Deciding...");
  delay(RESULT_DISPLAY_DELAY);
}

void executeLeftTurnDecision(int leftValue, int rightValue) 
{
  left();
  showOLED("TURN LEFT", "L:" + String(leftValue) + " > R:" + String(rightValue), "Going left!");
  delay(TURN_MOVE_DELAY);

  Stop();
  delay(POST_MOVE_STOP_DELAY);
}

void executeRightTurnDecision(int leftValue, int rightValue) 
{
  right();
  showOLED("TURN RIGHT", "R:" + String(rightValue) + " > L:" + String(leftValue), "Going right!");
  delay(TURN_MOVE_DELAY);

  Stop();
  delay(POST_MOVE_STOP_DELAY);
}

void executeDefaultTurnDecision(int leftValue) 
{
  right();
  showOLED("TURN RIGHT", "Both: " + String(leftValue) + " cm", "Default right!");
  delay(DEFAULT_TURN_DELAY);

  Stop();
  delay(POST_MOVE_STOP_DELAY);
}

void decideTurnDirection(int leftValue, int rightValue) 
{
  if (leftValue > rightValue) 
  {
    executeLeftTurnDecision(leftValue, rightValue);
  } 
  else if (rightValue > leftValue) 
  {
    executeRightTurnDecision(leftValue, rightValue);
  } 
  else 
  {
    executeDefaultTurnDecision(leftValue);
  }
}

void performObstacleAvoidanceRoutine(int measuredDistance) 
{
  Stop();
  announceObstacleOnDisplay(measuredDistance);
  soundObstacleAlertBuzzer();
  performBackwardManeuver(measuredDistance);

  L = scanLeftSideDistance();
  R = scanRightSideDistance(L);

  logScanResultsToSerial(L, R);
  displayScanResultSummary(L, R);
  decideTurnDirection(L, R);
}

void Obstacle() 
{
  distance = ultrasonic();
  logDistanceToSerial(distance);

  if (isPathClear(distance)) 
  {
    handleClearPath(distance);
    return;
  }

  performObstacleAvoidanceRoutine(distance);
}

char readIncomingSerialValue() 
{
  char incoming = Serial.read();
  Serial.println(incoming);
  return incoming;
}

void executeBluetoothForward() 
{
  forward();
  showOLED("FORWARD", "Bluetooth", "CMD: F");
}

void executeBluetoothBackward() 
{
  backward();
  showOLED("BACKWARD", "Bluetooth", "CMD: B");
}

void executeBluetoothLeft() 
{
  left();
  showOLED("LEFT", "Bluetooth", "CMD: L");
}

void executeBluetoothRight() 
{
  right();
  showOLED("RIGHT", "Bluetooth", "CMD: R");
}

void executeBluetoothStop() 
{
  Stop();
  showOLED("STOP", "Bluetooth", "CMD: S");
}

void dispatchBluetoothCommand(char command) 
{
  if (command == 'F') 
  { 
    executeBluetoothForward(); 
  }
  else if (command == 'B') 
  { 
    executeBluetoothBackward(); 
  }
  else if (command == 'L') 
  { 
    executeBluetoothLeft(); 
  }
  else if (command == 'R') 
  { 
    executeBluetoothRight(); 
  }
  else if (command == 'S') 
  { 
    executeBluetoothStop(); 
  }
}

void Bluetoothcontrol() 
{
  if (Serial.available() > 0) 
  {
    value = readIncomingSerialValue();
  }
  dispatchBluetoothCommand(value);
}

void executeVoiceForward() 
{
  forward();
  showOLED("FORWARD", "Voice Control", "CMD: ^");
}

void executeVoiceBackward() 
{
  backward();
  showOLED("BACKWARD", "Voice Control", "CMD: -");
}

void handleVoiceLeftClearance(int measuredLeft) 
{
  left();
  showOLED("LEFT", "Space: " + String(measuredLeft) + " cm", "Moving left...");
  delay(VOICE_SETTLE_DELAY);
  Stop();
}

void handleVoiceLeftBlocked(int measuredLeft) 
{
  Stop();
  digitalWrite(BUZZER, HIGH);
  delay(VOICE_BLOCK_BEEP_DELAY);
  digitalWrite(BUZZER, LOW);
  showOLED("BLOCKED!", "Left: " + String(measuredLeft) + " cm", "Cannot go left");
}

void executeVoiceLeftCheck() 
{
  servo.write(SERVO_LEFT_ANGLE);
  delay(VOICE_SCAN_DELAY);
  L = ultrasonic();
  servo.write(spoint);
  delay(VOICE_SETTLE_DELAY);
  if (L >= VOICE_MIN_CLEARANCE_CM) 
  {
    handleVoiceLeftClearance(L);
  } 
  else 
  {
    handleVoiceLeftBlocked(L);
  }
}

void handleVoiceRightClearance(int measuredRight) 
{
  right();
  showOLED("RIGHT", "Space: " + String(measuredRight) + " cm", "Moving right...");
  delay(VOICE_SETTLE_DELAY);
  Stop();
}

void handleVoiceRightBlocked(int measuredRight) 
{
  Stop();
  digitalWrite(BUZZER, HIGH);
  delay(VOICE_BLOCK_BEEP_DELAY);
  digitalWrite(BUZZER, LOW);
  showOLED("BLOCKED!", "Right: " + String(measuredRight) + " cm", "Cannot go right");
}

void executeVoiceRightCheck() 
{
  servo.write(SERVO_RIGHT_ANGLE);
  delay(VOICE_SCAN_DELAY);
  R = ultrasonic();
  servo.write(spoint);
  delay(VOICE_SETTLE_DELAY);
  if (R >= VOICE_MIN_CLEARANCE_CM) 
  {
    handleVoiceRightClearance(R);
  } 
  else 
  {
    handleVoiceRightBlocked(R);
  }
}

void executeVoiceStop() 
{
  Stop();
  showOLED("STOP", "Voice Control", "CMD: *");
}

void dispatchVoiceCommand(char command) 
{
  if (command == '^') 
  {
    executeVoiceForward();
  } 
  else if (command == '-') 
  {
    executeVoiceBackward();
  } 
  else if (command == '<') 
  {
    executeVoiceLeftCheck();
  } 
  else if (command == '>') 
  {
    executeVoiceRightCheck();
  } 
  else if (command == '*') 
  {
    executeVoiceStop();
  }
}

void voicecontrol() 
{
  if (Serial.available() > 0) 
  {
    value = readIncomingSerialValue();
    dispatchVoiceCommand(value);
  }
}