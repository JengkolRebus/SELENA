#include "Move.h"


AccelStepper alt_stepper(AccelStepper::DRIVER, MOTOR_ALT_STEP_PIN, MOTOR_ALT_DIR_PIN);
AccelStepper az_stepper(AccelStepper::DRIVER, MOTOR_AZ_STEP_PIN, MOTOR_AZ_DIR_PIN);

void SET_SLEW_SPEED(int index) {
  if (index < 0) {
    index = 0;
  }
  if (index > 3) {
    index = 3;
  }
  long double degSpeed = presetSpeed[index];
  long double degAccel = presetAccel[index];

  maxSpeedSteps = (long)(degSpeed * STEPS_PER_DEGREE);
  accelerationSteps = (long)(degAccel * STEPS_PER_DEGREE);

  alt_stepper.setMaxSpeed(maxSpeedSteps);
  alt_stepper.setAcceleration(accelerationSteps);

  az_stepper.setMaxSpeed(maxSpeedSteps);
  az_stepper.setAcceleration(accelerationSteps);
}

void SLEW_TO_TARGET(double TARGET_ALT, double TARGET_AZ) {
  APPLY_MODEL(model[0], model[1], model[2], model[3], TARGET_ALT, TARGET_AZ);
  double ALT = TARGET_ALT - ALT_CORR;
  double AZ = TARGET_AZ - AZ_CORR;
  ALT = constrain(ALT, ALT_MIN_DEG, ALT_MAX_DEG);
  AZ = constrain(AZ, AZ_MIN_DEG, AZ_MAX_DEG);

  long targetAltSteps = (long)(ALT * STEPS_PER_DEGREE);
  long targetAzSteps = CALCULATE_SHORTEST_PATH((az_stepper.currentPosition() / STEPS_PER_DEGREE), AZ);

  alt_stepper.moveTo(targetAltSteps);
  az_stepper.moveTo(targetAzSteps);

  Serial.print("ALT CORR: ");
  Serial.println(ALT_CORR);
  Serial.print("AZ CORR: ");
  Serial.println(AZ_CORR);

  if (alt_stepper.distanceToGo() != 0 || az_stepper.distanceToGo() != 0) {
    isSlewing = true;
  }
  else {
    isSlewing = false;
  }
}

long CALCULATE_SHORTEST_PATH(float currentAZ, float targetAZ) {
  float diff = fmod((targetAZ - currentAZ + 540.0), 360.0) - 180.0;
  float targetDeg = currentAZ + diff;
  return (long)(targetDeg * STEPS_PER_DEGREE);
}

void GET_CURRENT_POS(){
  currentALT = (alt_stepper.currentPosition() / STEPS_PER_DEGREE);
  currentAZ = (az_stepper.currentPosition() / STEPS_PER_DEGREE);
  if (currentAZ < 0) {
    currentAZ += 360.0;
  }
}

void HOME(){
  isTracking = 'N';
  haveTarget = false;
  
  SET_SLEW_SPEED(4);
  SLEW_TO_TARGET(0,180);
  while (alt_stepper.distanceToGo() != 0 || az_stepper.distanceToGo() != 0) {
    alt_stepper.run();
    az_stepper.run();
    yield();
  }
  getCurrentTime();
  GET_CURRENT_POS();
  ALTAZ_TO_RADEC(currentALT, currentAZ, lat, lon, currentEpoch, currentRA, currentDEC);
}