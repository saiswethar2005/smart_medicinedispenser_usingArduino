#include <Servo.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#define DS3231_I2C_ADDRESS 0x68

// Servo and LCD setup
Servo servo;  // create servo object
const int servoPin = 6;
const int buttonPin = 9;
const int ledPin = 7;
const int buzzerPin = 8;
int angle = 0;
int angleIncrement = 45;  // default 45 degrees for 4 compartments
int newAngle;
int buttonState;
int movementDelay = 50;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Function prototypes
void setDS3231time(byte, byte, byte, byte, byte, byte, byte);
void readDS3231time(byte *, byte *, byte *, byte *, byte *, byte *, byte *);
void alertUser();
void updateLCD();
byte decToBcd(byte);
byte bcdToDec(byte);

// Time variables
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  // Set initial time here: DS3231 seconds, minutes, hours, day, date, month, year
  setDS3231time(0, 0, 12, 2, 1, 12, 23);  // Adjust this for your initial time

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  servo.attach(servoPin);
  servo.write(angle);  // Initialize servo position
  lcd.begin(16, 2);
}

void loop() {
  // Update time variables
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  updateLCD();

  // Debugging
  Serial.print("Current Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);

  // Rotate servo at specific times
  if (second == 0) {  // Trigger once per minute
    newAngle = angle + angleIncrement;
    if (newAngle <= 180) {
      while (angle < newAngle) {
        angle++;
        servo.write(angle);
        delay(movementDelay);
      }
      alertUser();  // Alert the user
    } else {
      while (angle > 0) {
        angle--;
        servo.write(angle);
        delay(movementDelay);
      }
    }
  }
}

// Utility to set DS3231 time
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);  // Point to timekeeping register
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

// Utility to read DS3231 time
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);  // Start at register 0
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);

  *second = bcdToDec(Wire.read() & 0x7F);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3F);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

// Alert the user and wait for button press
void alertUser() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Take medicine!");
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);

  // Wait for the button to be pressed
  while (digitalRead(buttonPin) == HIGH) {
    delay(10);  // Debounce
  }

  // Turn off LED and buzzer
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  delay(1000);  // Additional debounce delay
  lcd.clear();
}

// Update the LCD with the current time and date
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (hour < 10) lcd.print("0");
  lcd.print(hour);
  lcd.print(":");
  if (minute < 10) lcd.print("0");
  lcd.print(minute);
  lcd.print(":");
  if (second < 10) lcd.print("0");
  lcd.print(second);

  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  if (dayOfMonth < 10) lcd.print("0");
  lcd.print(dayOfMonth);
  lcd.print("/");
  if (month < 10) lcd.print("0");
  lcd.print(month);
  lcd.print("/20");
  lcd.print(year);
}

// Helper functions for BCD conversion
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}
