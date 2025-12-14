#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#include <Arduino.h>
#include <math.h>
#include "Config.h"


void addDataPoint(double TARGET_AZ, double TARGET_ALT, double MOTOR_AZ, double MOTOR_ALT);
void fitting();
void APPLY_MODEL(double P1, double P7, double P5, double P6, double TARGET_ALT, double TARGET_AZ);
void printMatrix(String name, double *arr, int rows, int cols);


#endif