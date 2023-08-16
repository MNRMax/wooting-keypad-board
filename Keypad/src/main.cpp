#include <Arduino.h>
#include <Keyboard.h>

#include "SoftPWM.h"

double r_led = 10;
double g_led = 9;
double b_led = 8;
double r_led2 = 3;
double g_led2 = 6;
double b_led2 = 5;

double calibrationCount = 50;

double key1Top = 0;
double key1Bot = 0;
double key2Top = 0;
double key2Bot = 0;
double key1Total = 1;
double key2Total = 1;

double activation = 10;
double reactivation = 5;
double deactivation = 5;

double normalActivation = 57.5;

double calibrated = false;

double key1LastActivation = 0;
double key2LastActivation = 0;
double key1LastDectivation = 0;
double key2LastDectivation = 0;

bool key1Active = false;
bool key2Active = false;

bool key1Released = true;
bool key2Released = true;

bool normalMode = false;

void setup() {
    // put your setup code here, to run once:
    SoftPWMBegin();
    SoftPWMSet(r_led, 0);
    SoftPWMSet(g_led, 0);
    SoftPWMSet(r_led2, 0);
    SoftPWMSet(g_led2, 0);
    SoftPWMSet(b_led, 0);
    SoftPWMSet(b_led2, 0);
    Keyboard.begin();
    Serial.begin(9600);
}

void loop() {
    double key1 = analogRead(A0);
    double key2 = analogRead(A1);
    double key1Percent = (key1Top - key1) / key1Total * 100;
    double key2Percent = (key2Top - key2) / key2Total * 100;
    if (key1Percent < 0) {
        key1Percent = 0;
    }
    if (key1Percent > 100) {
        key1Percent = 100;
    }
    if (key2Percent < 0) {
        key2Percent = 0;
    }
    if (key2Percent > 100) {
        key2Percent = 100;
    }
    Serial.print(String(key1Percent) + "," + String(key2Percent) + ";");

    if (calibrated) {
        if (normalMode) {  // Normal Mode
            // Key 1
            if (key1Percent > normalActivation) {
                Keyboard.press('w');
            } else {
                Keyboard.release('w');
            }
            // Key 2
            if (key2Percent > normalActivation) {
                Keyboard.press('q');
            } else {
                Keyboard.release('q');
            }
        } else {  // Wooting Mode
            // key1
            if (key1Percent < key1LastDectivation) {
                key1LastDectivation = key1Percent;
            }
            if (key1Active) {
                if (key1Percent > key1LastActivation) {
                    key1LastActivation = key1Percent;
                } else {
                    if (key1Percent < key1LastActivation - deactivation) {
                        key1LastDectivation = key1Percent;
                        key1Active = false;
                        Keyboard.release('w');
                    }
                }
            } else {
                if (key1Percent > key1LastDectivation + reactivation) {
                    key1Active = true;
                    key1Released = false;
                    key1LastActivation = key1Percent;
                    Keyboard.press('w');
                }
            }
            // key2
            if (key2Percent < key2LastDectivation) {
                key2LastDectivation = key2Percent;
            }
            if (key2Active) {
                if (key2Percent > key2LastActivation) {
                    key2LastActivation = key2Percent;
                } else {
                    if (key2Percent < key2LastActivation - deactivation) {
                        key2LastDectivation = key2Percent;
                        key2Active = false;
                        Keyboard.release('q');
                    }
                }
            } else {
                if (key2Percent > key2LastDectivation + reactivation) {
                    key2Active = true;
                    key2Released = false;
                    key2LastActivation = key2Percent;
                    Keyboard.press('q');
                }
            }
        }
    }

    if (Serial.available() > 0) {
        byte incomingByte = Serial.read();
        if (incomingByte == '#') {
            double values[3];
            for (int i = 0; i < 3; i++) {
                char char1 = Serial.read();
                char char2 = Serial.read();
                String hexString = String(char1) + String(char2);
                values[i] = strtol(hexString.c_str(), NULL, 16);
            }
            SoftPWMSet(r_led, values[0]);
            SoftPWMSet(g_led, values[1]);
            SoftPWMSet(b_led, values[2]);
            SoftPWMSet(r_led2, values[0]);
            SoftPWMSet(g_led2, values[1]);
            SoftPWMSet(b_led2, values[2]);
            Serial.flush();
        }
        if (incomingByte == 'o') {
            SoftPWMSet(r_led, 0);
            SoftPWMSet(g_led, 0);
            SoftPWMSet(b_led, 0);
            SoftPWMSet(r_led2, 0);
            SoftPWMSet(g_led2, 0);
            SoftPWMSet(b_led2, 0);
        }
        if (incomingByte == 'd') {
            calibrated = true;
        }
        if (incomingByte == 'c') {
            double key1Sum = 0;
            double key2Sum = 0;
            for (int i = 0; i < calibrationCount; i++) {
                key1Sum += analogRead(A0);
            }
            for (int i = 0; i < calibrationCount; i++) {
                key2Sum += analogRead(A1);
            }
            double key1Avg = round(key1Sum / calibrationCount);
            double key2Avg = round(key2Sum / calibrationCount);
            byte tb = Serial.read();
            if (tb == 't') {
                key1Top = key1Avg;
                key2Top = key2Avg;
            }
            if (tb == 'b') {
                key1Bot = key1Avg;
                key2Bot = key2Avg;
                Serial.println(key2Avg);
            }
            key1Total = key1Top - key1Bot;
            key2Total = key2Top - key2Bot;
        }
        if (incomingByte == 'n') {
            normalMode = true;
        }
        if (incomingByte == 'w') {
            normalMode = false;
        }
        if (incomingByte == 'a') {
            byte type = Serial.read();
            if (type == 'n') {
                String numberString;
                for (int i = 0; i < 4; i++) {
                    numberString += String((char) Serial.read());
                }
                normalActivation = numberString.toDouble();
                // Serial.println(normalActivation);
            }
            if (type == 'w') {
                String numberString;
                for (int i = 0; i < 4; i++) {
                    numberString += String((char) Serial.read());
                }
                activation = numberString.toDouble();
                // Serial.println(activation);
            }
        }
        if (incomingByte == 'r') {
            byte type = Serial.read();
            if (type == 'w') {
                String numberString;
                for (int i = 0; i < 4; i++) {
                    numberString += String((char) Serial.read());
                }
                reactivation = numberString.toDouble();
                deactivation = numberString.toDouble();
                // Serial.println(reactivation);
            }
        }
    }
}
