#include <Bounce2.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

// -----------------------------
// Hardware pin assignments
// -----------------------------
const int button1Pin = 2;      // Button 1: Restart prompt/cancellation
const int button2Pin = A5;     // Button 2: Call for Assistance

// LCD (parallel) connections: RS -> 11, EN -> 12, D4 -> 13, D5 -> A0, D6 -> A1, D7 -> A2
LiquidCrystal lcd(11, 12, 13, A0, A1, A2);

// Setup a 4x3 keypad (ensure these pins do not conflict with others)
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo for gate control (simulate gate opening/closing)
Servo gateServo;

// -----------------------------
// Bounce objects for buttons
// -----------------------------
Bounce debouncer1 = Bounce(); // For Button 1
Bounce debouncer2 = Bounce(); // For Button 2

// -----------------------------
// User Database
// -----------------------------
struct User {
  String name;
  String rfid;
  String fingerprint;
};

User users[] = {
  {"Alice", "123456", "FP1"},
  {"Bob", "654321", "FP2"},
  {"Charlie", "987654", "FP3"}
};
const int totalUsers = sizeof(users) / sizeof(users[0]);

// -----------------------------
// Global Variables
// -----------------------------
bool otpRequired = false;              // True if fingerprint fails max attempts.
int fingerprintAttempts = 0;             // Count failed fingerprint attempts.
const int maxFingerprintAttempts = 5;    // Maximum allowed fingerprint failures

int rfidAttempts = 0;                    // Count RFID fallback attempts.
const int maxRfidAttempts = 3;           // Maximum RFID attempts allowed

bool restartPromptActive = false;        // True when the restart prompt is active.
unsigned long button1PressStart = 0;
const unsigned long longPressThreshold = 700; // Threshold for long press on Button 1 (700ms)

String correctOTP = "7890";              // OTP for fallback authentication

// Restart PIN attempt limits
int restartPinAttempts = 0;
const int maxRestartPinAttempts = 3;

// -----------------------------
// Delay values (scaled down by factor of 10)
// -----------------------------
// Assistance delay: 3000ms becomes 300ms.
// Error/prompt messages: 2000ms becomes 200ms.
// Gate open delay: 3000ms becomes 300ms.

// -----------------------------
// Helper function: Scroll text if message longer than 16 characters
// -----------------------------
void scrollText(String message, int row) {
  lcd.setCursor(0, row);
  if (message.length() <= 16) {
    lcd.print(message);
    return;
  }
  for (int pos = 0; pos <= message.length() - 16; pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + 16));
    delay(250); // adjust scroll speed as needed
  }
}

// -----------------------------
// Setup Function
// -----------------------------
void setup() {
  Serial.begin(9600);
  Serial.println("System started. Awaiting input...");
  Serial.println("Scan Fingerprint...");

  // Initialize LCD (16x2)
  lcd.begin(16, 2);
  scrollText("Scan Fingerprint", 0);
  lcd.autoscroll();
  delay(500);
  lcd.noAutoscroll();


  // Attach servo to pin 10 and set initial position (gate closed)
  gateServo.attach(10);
  gateServo.write(0);

  // Setup buttons with internal pull-ups
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  
  // Initialize Bounce objects for buttons
  debouncer1.attach(button1Pin);
  debouncer1.interval(25);
  
  debouncer2.attach(button2Pin);
  debouncer2.interval(25);
}

// -----------------------------
// Main Loop
// -----------------------------
void loop() {
  debouncer1.update();
  debouncer2.update();
  
  processButton1();
  processButton2();
  processSerialInput();
  
  if (!restartPromptActive && !otpRequired) {
    lcd.setCursor(0, 0);
    lcd.print("Scan Fingerprint  "); // Clear any old text.
  }
}

// -----------------------------
// Process Button 1 (Restart Prompt/Cancellation)
// -----------------------------
void processButton1() {
  bool btnState = debouncer1.read();
  
  if (debouncer1.fell()) { 
    button1PressStart = millis();
    Serial.println("Button 1 pressed");
  }
  
  if (debouncer1.rose()) {
    unsigned long duration = millis() - button1PressStart;
    Serial.print("Button 1 released, duration: ");
    Serial.println(duration);
    
    if (restartPromptActive) {
      if (duration >= longPressThreshold) {
        handleButton1LongPress();
      } else {
        restartPromptActive = false;
      }
    } else {
      if (duration >= longPressThreshold) {
        handleButton1LongPress();
      } else {
        handleButton1ShortPress();
      }
    }
  }
  
  if (restartPromptActive && debouncer1.read() == LOW) {
    unsigned long currentDuration = millis() - button1PressStart;
    if (currentDuration >= longPressThreshold) {
      handleButton1LongPress();
      while (debouncer1.read() == LOW) {
        debouncer1.update();
      }
    }
  }
}

// -----------------------------
// Process Button 2 (Assistance)
// -----------------------------
void processButton2() {
  if (debouncer2.fell()) {
    Serial.println("Button 2 pressed: Calling Assistance");
    lcd.clear();
    lcd.print("Calling Assistance........");
  for (int positionCounter = 0; positionCounter < 20; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(150);
  }
   for (int positionCounter = 0; positionCounter < 29; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    delay(100);
  }
    delay(1500); // 300 ms delay (scaled down)
    lcd.clear();
    lcd.print("Scan Fingerprint");
  }
}

// -----------------------------
// Process Serial Input for Authentication
// -----------------------------
void processSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.startsWith("FP:")) {
      String fpInput = input.substring(3);
      Serial.print("Received fingerprint: ");
      Serial.println(fpInput);
      
      bool matchFound = false;
      for (int i = 0; i < totalUsers; i++) {
        if (fpInput == users[i].fingerprint) {
          Serial.print("Fingerprint match: ");
          Serial.println(users[i].name);
          lcd.clear();
          scrollText("Welcome " + users[i].name, 0);
          openGate();
          fingerprintAttempts = 0;
          rfidAttempts = 0;
          matchFound = true;
          break;
        }
      }
      if (!matchFound) {
        fingerprintAttempts++;
        Serial.print("Fingerprint attempt ");
        Serial.print(fingerprintAttempts);
        Serial.println(" failed.");
        lcd.clear();
        lcd.print("FP Failed");
        delay(200); // 200 ms delay (scaled down)
        if (fingerprintAttempts >= maxFingerprintAttempts) {
          otpRequired = true;
          rfidAttempts = 0;
          Serial.println("Max FP attempts reached. Use RFID+OTP.");
          lcd.clear();
          scrollText("Use RFID+OTP", 0);
        } else {
          lcd.clear();
          lcd.print("Scan Fingerprint");
        }
      }
    }
    else if (input.startsWith("RFID:") && otpRequired) {
      String rfidInput = input.substring(5);
      Serial.print("Received RFID: ");
      Serial.println(rfidInput);
      
      bool userFound = false;
      for (int i = 0; i < totalUsers; i++) {
        if (rfidInput == users[i].rfid) {
          Serial.print("RFID match: ");
          Serial.println(users[i].name);
          lcd.clear();
          lcd.print("RFID OK");
          String otp = readOTP();
          Serial.print("OTP entered: ");
          Serial.println(otp);
          if (otp == correctOTP) {
            lcd.clear();
            lcd.print("OTP Verified");
            openGate();
            userFound = true;
          } else {
            lcd.clear();
            lcd.print("Access Denied");
            Serial.println("Incorrect OTP.");
          }
          break;
        }
      }
      if (!userFound) {
        rfidAttempts++;
        Serial.print("RFID attempt ");
        Serial.print(rfidAttempts);
        Serial.println(" failed.");
        lcd.clear();
        lcd.print("Invalid RFID");
        if (rfidAttempts >= maxRfidAttempts) {
          Serial.println("Max RFID attempts reached. Exiting fallback mode.");
          otpRequired = false;
          rfidAttempts = 0;
        }
        delay(200); // 200 ms delay
        lcd.clear();
        lcd.print("Scan Fingerprint");
      }
    }
  }
}

// -----------------------------
// Read a 4-digit OTP from Keypad with Cancellation Support
// -----------------------------
String readOTPWithCancel() {
  String otp = "";
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN:    ");
  
  unsigned long cancelTimer = 0;
  bool cancelActive = false;
  
  while (otp.length() < 4) {
    char key = keypad.getKey();
    if (key) {
      otp += key;
      lcd.setCursor(otp.length() - 1, 1);
      lcd.print("*");
    }
    if (digitalRead(button1Pin) == LOW) {
      if (!cancelActive) {
        cancelTimer = millis();
        cancelActive = true;
      }
      if (millis() - cancelTimer >= longPressThreshold) {
        Serial.println("PIN entry cancelled via long press.");
        lcd.print("PIN Entry Cancelled");
        return ""; // Indicate cancellation.
      }
    } else {
      cancelActive = false;
    }
  }
  return otp;
}
String readOTP() {
  String otp = "";
  lcd.setCursor(0, 1);
  lcd.print("Enter OTP PIN:    ");
  
  unsigned long cancelTimer = 0;
  bool cancelActive = false;
  
  while (otp.length() < 4) {
    char key = keypad.getKey();
    if (key) {
      otp += key;
      lcd.setCursor(otp.length() - 1, 1);
      lcd.print("*");
    }
    if (digitalRead(button1Pin) == LOW) {
      if (!cancelActive) {
        cancelTimer = millis();
        cancelActive = true;
      }
      if (millis() - cancelTimer >= longPressThreshold) {
        Serial.println("OTP entry cancelled via long press.");
        return ""; // Indicate cancellation.
      }
    } else {
      cancelActive = false;
    }
  }
  return otp;
}

// -----------------------------
// Handle Button 1 Short Press: Initiate Restart Prompt (requires PIN)
// -----------------------------
void handleButton1ShortPress() {
  Serial.println("Button 1 Short Press: Initiating restart prompt.");
  lcd.clear();
  lcd.print("Enter Restart PIN");
  restartPromptActive = true;
  for (int positionCounter = 0; positionCounter < 17; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(100);
  }
   for (int positionCounter = 0; positionCounter < 17; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    delay(100);
  }
  
  
  String pin = readOTPWithCancel();
  if (pin == "") {
    Serial.println("Restart prompt cancelled.");
    lcd.clear();
    lcd.print("Restart Cancelled");
    delay(200); // 200 ms delay
    lcd.clear();
    lcd.print("Scan Fingerprint");
    restartPromptActive = false;
    return;
  }
  Serial.print("Restart PIN entered: ");
  Serial.println(pin);
  
  if (pin == "1234") {
    lcd.clear();
    lcd.print("Restarting...");
    Serial.println("Restart PIN correct. Restarting system...");
    restartPromptActive = false;
    restartPinAttempts = 0; // Reset attempts on success.
    delay(200); // 200 ms delay
    setup();  // Reinitialize system (simulation)
  } else {
    restartPinAttempts++;
    Serial.print("Restart PIN incorrect. Attempt ");
    Serial.print(restartPinAttempts);
    Serial.println(" of 3.");
    if (restartPinAttempts >= maxRestartPinAttempts) {
      lcd.clear();
      lcd.print("Max Attempts");
      Serial.println("Max restart PIN attempts reached.");
      delay(200); // 200 ms delay
      lcd.clear();
      lcd.print("Scan Fingerprint");
      restartPromptActive = false;
      restartPinAttempts = 0; // Reset counter
    } else {
      lcd.clear();
      lcd.print("Wrong PIN");
      Serial.println("Incorrect Restart PIN.");
      delay(200); // 200 ms delay
      lcd.clear();
      lcd.print("Scan Fingerprint");
      restartPromptActive = false;
    }
  }
}

// -----------------------------
// Handle Button 1 Long Press: Cancel Restart Prompt
// -----------------------------
void handleButton1LongPress() {
  Serial.println("Button 1 Long Press: Cancelling restart prompt.");
  lcd.clear();
  lcd.print("Restart Cancelled");
  delay(200); // 200 ms delay
  lcd.clear();
  lcd.print("Scan Fingerprint");
  restartPromptActive = false;
}

// -----------------------------
// Simulate Gate Operation using Servo
// -----------------------------
void openGate() {
  Serial.println("Gate opening...");
  gateServo.write(90);  // Open gate position
  delay(3000);           // 300 ms delay
  Serial.println("Gate closing...");
  lcd.clear();
  lcd.println("Gate Closing!!!!!!...");
  delay(800);           // 100 ms delay
  gateServo.write(0);   // Close gate
  lcd.clear();
  lcd.print("Scan Fingerprint");
}
