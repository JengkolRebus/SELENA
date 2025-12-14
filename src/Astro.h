#ifndef ASTRO_H
#define ASTRO_H

#include <Arduino.h>
#include <math.h>
#include <TimeLib.h>
#include "Config.h"


double JulianDay(time_t epoch);
double gmst(double JD);
double lst(double JD, double lonDeg);
void ALTAZ_TO_RADEC(double ALTITUDE, double AZIMUTH, double LATITUDE, double LONGITUDE, time_t EPOCH, double &RA, double &DECLINATION);
void RADEC_TO_ALTAZ(double RA, double DECLINATION, double LATITUDE, double LONGITUDE, time_t EPOCH, double &ALT, double &AZ);
String formatLongitude(double lon);
String formatLatitude(double lat);


#endif