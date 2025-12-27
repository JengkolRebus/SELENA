/*
   Title    SELENA (Sky Explorer Low-cost Easy Navigation Assistant)
   by       _jengkolrebus
   Version  0.1

   Date   Oct 26 2025

   Description:
      Based on a low-cost microcontroller, this system controls stepper motors using
      the LX200 command set. Compatible with Stellarium, it was originally developed
      for my 3D-printed telescope mount but works equally well with commercially available models.

   Help by:
      Gemini 2.5 Flash
      ChatGPT GPT-5
      ;)

   Github Page:
      www.mypage.com

*/

#include <Arduino.h>
#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include "Config.h"
#include "Time.h"
#include "Command.h"
#include "Astro.h"
#include "Move.h"

double lat = DEFAULT_LATITUDE;
double lon = DEFAULT_LONGITUDE;
bool isMoving = false;
char isTracking = 'N';

// Timing for manual buttons (debounce + step interval)
unsigned long lastButtonMove = 0;
const unsigned long buttonInterval = 150; // ms between manual steps

WiFiServer server(MOUNTING_PORT);

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_EN_PIN, OUTPUT);
  digitalWrite(MOTOR_EN_PIN, LOW);

  pinMode(BTN_HOME, INPUT);

  alt_stepper.setMaxSpeed(maxSpeedSteps);
  alt_stepper.setAcceleration(accelerationSteps);
  alt_stepper.setCurrentPosition(0);

  az_stepper.setMaxSpeed(maxSpeedSteps);
  az_stepper.setAcceleration(accelerationSteps);
  az_stepper.setCurrentPosition(0);

  //
  alt_stepper.setPinsInverted(false, true, false);
  az_stepper.setPinsInverted(false, true, false);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address Server: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(MOUNTING_PORT);

  server.begin();
  server.setNoDelay(true);

  timeClient.begin();
  syncNTP();
  useNTP = false; // pastikan tidak diupdate lagi setelah itu
  Serial.println("Menunggu koneksi ke Stellarium.");

  // Set posisi awal stepper berdasarkan nilai di Config.cpp
  alt_stepper.setCurrentPosition(currentALT * STEPS_PER_DEGREE);
  az_stepper.setCurrentPosition(currentAZ * STEPS_PER_DEGREE);

  getCurrentTime();
  ALTAZ_TO_RADEC(currentALT, currentAZ, lat, lon, currentEpoch, currentRA, currentDEC);
  Serial.print(currentALT);
  Serial.print("\t");
  Serial.println(currentAZ);
}

void loop() {
  // put your main code here, to run repeatedly:
  alt_stepper.run();
  az_stepper.run();
  getCurrentTime();
  if(isTracking == 'T'){
    RADEC_TO_ALTAZ(targetRA, targetDEC, lat, lon, currentEpoch, targetALT, targetAZ);
    SLEW_TO_TARGET(targetALT, targetAZ);
    isSlewing = false;
  }
  // accept client
  WiFiClient client = server.accept();

  if (client) {
    Serial.println("Client connected");
    String buf = "";
    unsigned long lastActivity = millis();
    while (client.connected() && (millis() - lastActivity) < 300000) {
      alt_stepper.run();
      az_stepper.run();

      getCurrentTime();
      GET_CURRENT_POS();
      ALTAZ_TO_RADEC(currentALT, currentAZ, lat, lon, currentEpoch, currentRA, currentDEC);
      if (millis() - lastButtonMove >= buttonInterval)
      {
        if (digitalRead(BTN_HOME) == LOW)
        {
          Serial.println("Homing.");
          HOME();
        }
        lastButtonMove = millis();
      }

      // --- Cetak waktu tiap 1 detik ---
      if (lastActivity - lastGet >= PRINT_INTERVAL) {
        getCurrentTime();
        if(isTracking == 'T'){
          RADEC_TO_ALTAZ(targetRA, targetDEC, lat, lon, currentEpoch, targetALT, targetAZ);
          SLEW_TO_TARGET(targetALT, targetAZ);
          isSlewing = false;
        }

        lastGet = lastActivity;

        Serial.printf("%ld %s %s RA: %s Dec: %s Alt: %s Az: %s Lat: %s Lon: %s\n",
                      (long)currentEpoch, currentDate.c_str(), currentTime.c_str(), String(currentRA).c_str(), String(currentDEC).c_str(),
                    String(currentALT).c_str(), String(currentAZ).c_str(), String(lat).c_str(), String(lon).c_str());
        Serial.printf("T_RA: %s T_Dec: %s T_Alt: %s T_Az: %s\n",
                      String(targetRA).c_str(), String(targetDEC).c_str(),
                    String(targetALT).c_str(), String(targetAZ).c_str());

      }
      if (client.available()) {
        char c = client.read();
        lastActivity = millis();

        if (c == 0x06) {
          client.print("A");
          client.flush();
          buf = "";
          continue;
        }
        if (c == '#') {
          buf.trim();
          if (buf.length() > 0) {
            handleCmd(buf, client);
          }
          buf = "";
        }
        else {
          buf += c;
        }
      }
      yield();
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}