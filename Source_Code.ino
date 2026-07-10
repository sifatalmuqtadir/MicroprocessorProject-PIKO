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

void showOLED(String status, String line2, String line3) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("PIKO");
  display.drawLine(0, 17, 128, 17, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(status);
  display.setTextSize(1);
  display.setCursor(0, 42);
  display.println(line2);
  display.setCursor(0, 54);
  display.println(line3);
  display.display();
}

int ultrasonic() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(4);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  long t = pulseIn(Echo, HIGH, 30000);
  if (t == 0) return 999;
  return t / 29 / 2;
}

void forward() {
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void backward() {
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}

void right() {
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void left() {
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}

void Stop() {
  M1.run(RELEASE);
  M2.run(RELEASE);
  M3.run(RELEASE);
  M4.run(RELEASE);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  servo.attach(motor);
  servo.write(spoint);
  M1.setSpeed(Speed);
  M2.setSpeed(Speed);
  M3.setSpeed(Speed);
  M4.setSpeed(Speed);
  M1.run(RELEASE);
  M2.run(RELEASE);
  M3.run(RELEASE);
  M4.run(RELEASE);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18, 5);
  display.println("PIKO");
  display.setTextSize(1);
  display.setCursor(20, 52);
  display.println("Robot Car v1.0");
  display.display();
  delay(3000);
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  Obstacle();
  // Bluetoothcontrol();
  // voicecontrol();
}

void Obstacle() {
  distance = ultrasonic();
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance > 20 || distance == 999) {
    forward();
    if (distance == 999) {
      showOLED("FORWARD", "Dist: Clear", "Path is clear!");
    } else {
      showOLED("FORWARD", "Dist: " + String(distance) + " cm", "Path is clear!");
    }
    return;
  }

  Stop();
  showOLED("OBSTACLE!", "Dist: " + String(distance) + " cm", "Object detected!");
  digitalWrite(BUZZER, HIGH);
  delay(300);
  digitalWrite(BUZZER, LOW);
  delay(200);

  backward();
  showOLED("BACKWARD", "Dist: " + String(distance) + " cm", "Moving back...");
  delay(500);
  Stop();
  delay(300);

  showOLED("SCANNING", "Checking LEFT...", "Please wait...");
  servo.write(180);
  delay(1000);
  L = ultrasonic();
  delay(200);
  servo.write(spoint);
  delay(600);

  showOLED("SCANNING", "Checking RIGHT...", "Left: " + String(L) + " cm");
  servo.write(20);
  delay(1000);
  R = ultrasonic();
  delay(200);
  servo.write(spoint);
  delay(600);

  Serial.print("Left: "); Serial.println(L);
  Serial.print("Right: "); Serial.println(R);

  showOLED("RESULT", "L:" + String(L) + " R:" + String(R), "Deciding...");
  delay(500);

  if (L > R) {
    left();
    showOLED("TURN LEFT", "L:" + String(L) + " > R:" + String(R), "Going left!");
    delay(600);
    Stop();
    delay(300);
  } else if (R > L) {
    right();
    showOLED("TURN RIGHT", "R:" + String(R) + " > L:" + String(L), "Going right!");
    delay(600);
    Stop();
    delay(300);
  } else {
    right();
    showOLED("TURN RIGHT", "Both: " + String(L) + " cm", "Default right!");
    delay(800);
    Stop();
    delay(300);
  }
}

void Bluetoothcontrol() {
  if (Serial.available() > 0) {
    value = Serial.read();
    Serial.println(value);
  }
  if (value == 'F') { forward();  showOLED("FORWARD",  "Bluetooth", "CMD: F"); }
  else if (value == 'B') { backward(); showOLED("BACKWARD", "Bluetooth", "CMD: B"); }
  else if (value == 'L') { left();     showOLED("LEFT",     "Bluetooth", "CMD: L"); }
  else if (value == 'R') { right();    showOLED("RIGHT",    "Bluetooth", "CMD: R"); }
  else if (value == 'S') { Stop();     showOLED("STOP",     "Bluetooth", "CMD: S"); }
}

void voicecontrol() {
  if (Serial.available() > 0) {
    value = Serial.read();
    Serial.println(value);
    if (value == '^') {
      forward();
      showOLED("FORWARD", "Voice Control", "CMD: ^");
    } else if (value == '-') {
      backward();
      showOLED("BACKWARD", "Voice Control", "CMD: -");
    } else if (value == '<') {
      servo.write(180);
      delay(800);
      L = ultrasonic();
      servo.write(spoint);
      delay(500);
      if (L >= 10) {
        left();
        showOLED("LEFT", "Space: " + String(L) + " cm", "Moving left...");
        delay(500);
        Stop();
      } else {
        Stop();
        digitalWrite(BUZZER, HIGH);
        delay(150);
        digitalWrite(BUZZER, LOW);
        showOLED("BLOCKED!", "Left: " + String(L) + " cm", "Cannot go left");
      }
    } else if (value == '>') {
      servo.write(20);
      delay(800);
      R = ultrasonic();
      servo.write(spoint);
      delay(500);
      if (R >= 10) {
        right();
        showOLED("RIGHT", "Space: " + String(R) + " cm", "Moving right...");
        delay(500);
        Stop();
      } else {
        Stop();
        digitalWrite(BUZZER, HIGH);
        delay(150);
        digitalWrite(BUZZER, LOW);
        showOLED("BLOCKED!", "Right: " + String(R) + " cm", "Cannot go right");
      }
    } else if (value == '*') {
      Stop();
      showOLED("STOP", "Voice Control", "CMD: *");
    }
  }
}