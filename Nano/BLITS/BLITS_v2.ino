// Libraries
#include <Adafruit_TiCoServo.h>
#include <avr/power.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_MAX31855.h"
#include "HX711.h"

// Pin definitions
#define TC_DO_PIN   3
#define TC_CS_PIN   4
#define TC_CLK_PIN  5

#define LC_DAT_PIN  6
#define LC_CLK_PIN  7

#define PS1_PIN  A7
#define PS2_PIN  A6
#define PS3_PIN  A5

#define HURTS_PIN A3

#define DATA_SERIAL Serial
#define DATA_BAUDRATE 115200

#define CONTROL_SERIAL Serial1
#define CONTROL_BAUDRATE 9600

#define DIFFERENT_SERIALS //Use this define if the two above serials are different

const char* safe_message = "system state: SAFE, enter \"start\" to marm system";
const char* marm_message = "system state: MARM, enter \"yes\" to prime system";
const char* prime_message = "system state: PRIME, enter \"fire\" to fire";
const char* fire_message = "reading test data";

// KEEP IN ACENDING ORDER OF DANGER
enum class STATE {
   SAFE,
   MARM,
   PRIME,
   FIRE
};

// Object declarations
SoftwareSerial Serial1(10,11);
Adafruit_MAX31855 ThermoCouple(TC_CLK_PIN, TC_CS_PIN, TC_DO_PIN);
HX711 LoadCell;

STATE state = STATE::SAFE;
String command;
bool armState = false;

const unsigned long fireTime = 15000;
const unsigned long dataOffset = 5000;
const unsigned long hurtsTime = 2000;
unsigned long fireStart;
unsigned long relativeTime;

bool serial_setup() {
  #ifdef DIFFERENT_SERIALS
  CONTROL_SERIAL.begin(CONTROL_BAUDRATE);
  #endif
  DATA_SERIAL.begin(DATA_BAUDRATE);
}

void proccess_current_state() {
    switch (state) 
    {
    case STATE::SAFE:
        CONTROL_SERIAL.println(safe_message);
        armState = false;
        break;
    case STATE::MARM:
        CONTROL_SERIAL.println(marm_message);
        armState = false;
        break;
    case STATE::PRIME:
        CONTROL_SERIAL.println(prime_message);
        armState = true;
        break;
    case STATE::FIRE:
        CONTROL_SERIAL.prinln(fire_message);
        fireStart = millis();
        break;
    }
}

void sensor_read() {
    data = "";
    Serial.print(millis());
    Serial.print(",");

    double c = ThermoCouple.readCelsius();
    Serial.print(c);
    Serial.print(",");

    float m = LoadCell.get_units(0);
    Serial.print(m);
    Serial.print(",");

    float p = analogRead(PS1_PIN);
    Serial.print(p);
    Serial.print(",");

    p = analogRead(PS2_PIN);
    Serial.print(p);
    Serial.print(",");

    p = analogRead(PS3_PIN);
    Serial.print(p);
    Serial.print(",");

    //Serial.print(pos);
    //Servo.write(pos);
    Serial.println();
}

void safe_to_marm() {
    if (state == STATE::SAFE) {
        state = STATE::MARM;
    }
    proccess_current_state();
}

void marm_to_prime() {
    if (state == STATE::MARM) {
        state = STATE::PRIME
    }
    proccess_current_state();
}

void prime_to_fire() {
    if (state == STATE::PRIME) {
        state = STATE::FIRE;
    }
    proccess_current_state();
}

void setup() {
  serial_setup();
  pinMode(SOL_PIN, OUTPUT);
  pinMode(HURTS_PIN, OUTPUT);
  LoadCell.begin(LC_DAT_PIN, LC_CLK_PIN);
  LoadCell.set_scale(4883);              // found with HX_set_persistent example code
  LoadCell.tare();
  #if (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1);
  #endif
  Servo.attach(SRVO_PIN);
}

void loop() {
    if (CONTROL_SERIAL.available() > 0) {
        message = CONTROL_SERIAL.readString();
        if (command == "read data") {
            test_data_reading();
        } else if (command == "info") {
            print_system_info();
        } else if (command == "safe") {
            state = STATE::SAFE;
            proccess_current_state();
        } else if (command == "start") {
            safe_to_marm();
        } else if (command == "yes") {
            marm_to_prime();
        } else if (command == "fire") {
            prime_to_fire();
        }
    }
    
    switch (state)
    {
    case STATE::SAFE:
        break;
    case STATE::MARM:
        break;
    case STATE::PRIME:
        break;
    case STATE::FIRE:
        relativeTime = millis() - fireStart;
        if (armState && relativeTime > dataOffset) {
            digitalWrite(HURTS_PIN, HIGH);
            if (relativeTime - dataOffset > hurtsTime) {
                armState = false;
                digitalWrite(HURTS_PIN, LOW);
            }g today at 4 in Edu50 as usual. Wel
        }
        sensor_read();
        break;
    }

}