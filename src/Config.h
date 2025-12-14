#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Configuration fo Selena

// Firmware Configuration
#define FIRMWARE_DATE "Oct 26 2025"
#define FIRMWARE_TIME "00:00:00"
#define FIRMWARE_VERSION "0.1"
#define FIRMWARE_NAME "SELENA"

// ===== USER CONFIGURATION =====
#define WIFI_SSID "GOGO PANGESTU 2"
#define WIFI_PASS "00000001"
const int MOUNTING_PORT = 10001; // Stellarium port connection

// ===== LOCATION CONFIGURATION =====
const double DEFAULT_LATITUDE = -3.45;
const double DEFAULT_LONGITUDE = 102.51;

// ===== BUTTON PIN =====
#define BTN_HOME D3

// Time Config
extern double UTC_OFFSET;
extern double utc_offsett_in_second;

// Global Var
extern double lat;
extern double lon;
extern time_t currentEpoch;
extern String currentDate;
extern String currentTime;
extern bool useNTP;
extern bool isMoving;
extern char isTracking;
extern char alignmentStatus;
extern double AZ_CORR;
extern double ALT_CORR;

// Mounts Variable
extern double currentRA;
extern double currentDEC;
extern double currentALT;
extern double currentAZ;
extern time_t targetTime;
extern double targetRA;
extern double targetDEC;
extern double targetALT;
extern double targetAZ;
extern double deltaALT;
extern double deltaAZ;
extern bool isSlewing;
extern bool haveTarget;

// Non Blocking Component
extern unsigned long lastGet;
const unsigned long PRINT_INTERVAL = 1000; // cetak tiap 1 detik

// ===== MOTOR CONFIGURATION =====
extern const double MOTOR_STEPS;
extern const double MICROSTEPS;
extern const double GEAR_RATIO;
extern const double STEPS_PER_DEGREE;
extern const double MAX_SPEED_DEGREE_PER_SEC;
extern const double ACCELERATION_DEGREE_PER_SEC2;
extern const double presetSpeed[];
extern const double presetAccel[];

// Konversi dari Derajat/Detik ke Langkah/Detik sebelum diterapkan
extern long maxSpeedSteps;
extern long accelerationSteps;

// ===== MOUNTING LIMITS =====
extern const double ALT_MIN_DEG;
extern const double ALT_MAX_DEG;
extern const double AZ_MIN_DEG;
extern const double AZ_MAX_DEG;


// ===== ALIGNMENT =====
const int MAX_POINTS = 15;
extern int pointCount;
extern double model[];


#endif