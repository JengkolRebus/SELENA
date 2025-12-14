#ifndef MOVE_H
#define MOVE_H

#include <Arduino.h>
#include <AccelStepper.h>
#include "Config.h"
#include "Alignment.h"
#include "Time.h"
#include "Astro.h"

#define MOTOR_ALT_STEP_PIN D1    // Pin STEP (Altitude)
#define MOTOR_ALT_DIR_PIN D2     // Pin DIR (Altitude)
#define MOTOR_AZ_STEP_PIN D5    // Pin STEP (Azimuth)
#define MOTOR_AZ_DIR_PIN D6     // Pin DIR (Azimuth)
#define MOTOR_EN_PIN D7 

extern AccelStepper alt_stepper;
extern AccelStepper az_stepper;

void SET_SLEW_SPEED(int index);
void SLEW_TO_TARGET(double Alt, double Az);
long CALCULATE_SHORTEST_PATH(float currentAZ, float targetAZ);
void GET_CURRENT_POS();
void HOME();

#endif