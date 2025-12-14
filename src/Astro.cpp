#include "Astro.h"

//Julian Day
double JulianDay(time_t epoch) {
  bool greg = false;
  int C = 0;

  int tahun = year(epoch);
  int bulan = month(epoch);
  int tanggal = day(epoch);
  double jam = hour(epoch);
  double menit = minute(epoch);
  double detik = second(epoch);
  if (bulan < 3) {
    tahun = tahun - 1;
    bulan = bulan + 12;
  }
  int A = tahun / 100;
  if (tahun == 1582 && bulan == 10) {
    if (tanggal >= 14) {
      greg = true;
    }
    else if (tanggal <= 4) {
      greg = false;
    }
  }
  else if (tahun > 1582) {
    greg = true;
  }
  else if (tahun < 1582) {
    greg = false;
  }

  if (greg) {
    C = int(2 - A + int(A / 4));
  }
  else {
    C = 0;
  }
  int E = int(365.25 * (tahun + 4716));
  int F = int(30.6001 * (bulan + 1));
  double JD = C + tanggal + E + F - 1524.5 + ((jam + (menit / 60) + (detik / 3600)) / 24);
  return JD;
}

double gmst(double JD) {
  double T = (JD - 2451545.0) / 36525.0;
  double GMST = 280.46061837 +
                360.98564736629 * (JD - 2451545.0) +
                0.000387933 * T * T -
                (T * T * T) / 38710000.0;

  GMST = fmod(GMST, 360.0);
  if (GMST < 0) GMST += 360.0;

  return GMST;   // degrees
}

double lst(double JD, double lonDeg) {
  double LST = gmst(JD) + lonDeg;
  LST = fmod(LST, 360.0);
  if (LST < 0) LST += 360.0;
  return LST; // degrees
}

// altaz to radec V3
void ALTAZ_TO_RADEC(double ALTITUDE, double AZIMUTH, double LATITUDE, double LONGITUDE, time_t EPOCH, double &RA, double &DECLINATION) {
  double JD = JulianDay(EPOCH - (7 * 3600));
  double LST_deg = lst(JD, LONGITUDE);    // degrees
  double LST_rad = LST_deg * DEG_TO_RAD;

  double altRad = (ALTITUDE + ALT_CORR) * DEG_TO_RAD;
  double azRad  = (AZIMUTH + AZ_CORR)  * DEG_TO_RAD;
  double latRad = LATITUDE * DEG_TO_RAD;

  // DEC
  double sinDec =
    sin(altRad) * sin(latRad) +
    cos(altRad) * cos(latRad) * cos(azRad);

  double decRad = asin(sinDec);

  // HA
  double sinHA = -sin(azRad) * cos(altRad) / cos(decRad);
  double cosHA = (sin(altRad) - sin(latRad) * sinDec) /
                 (cos(latRad) * cos(decRad));

  double HA = atan2(sinHA, cosHA);  // radians

  // RA
  double raRad = LST_rad - HA;
  raRad = fmod(raRad, 2 * M_PI);
  if (raRad < 0) raRad += 2 * M_PI;

  RA  = (raRad * RAD_TO_DEG) / 15.0;   // hours
  DECLINATION = decRad * RAD_TO_DEG;           // degrees
}

// radec to altaz V2
void RADEC_TO_ALTAZ(double RA, double DECLINATION, double LATITUDE, double LONGITUDE, time_t EPOCH, double &ALT, double &AZ) {
  double JD = JulianDay(EPOCH - (7 * 3600));
  double LST_deg = lst(JD, LONGITUDE);

  double raDeg = RA * 15.0;
  double HA = LST_deg - raDeg;
  if (HA < 0) HA += 360.0;

  double HA_rad  = HA * DEG_TO_RAD;
  double dec_rad = DECLINATION * DEG_TO_RAD;
  double lat_rad = LATITUDE * DEG_TO_RAD;

  double sinAlt =
    sin(dec_rad) * sin(lat_rad) +
    cos(dec_rad) * cos(lat_rad) * cos(HA_rad);

  ALT = asin(sinAlt) * RAD_TO_DEG;

  double sinAz =
    -cos(dec_rad) * sin(HA_rad) / cos(asin(sinAlt));

  double cosAz =
    (sin(dec_rad) - sinAlt * sin(lat_rad)) /
    (cos(lat_rad) * cos(asin(sinAlt)));

  AZ = atan2(sinAz, cosAz) * RAD_TO_DEG;
  if (AZ < 0) AZ += 360.0;
}

// Format Longitude
String formatLongitude(double lon) {
  char sign = '+'; // default: West positif
  if (lon > 0) { // East = negatif (sesuai LX200)
    sign = '-';
  }
  lon = fabs(lon); // ubah ke nilai absolut
  int degrees = (int)lon;
  int minutes = (int)((lon - degrees) * 60.0);
  char buffer[16];
  sprintf(buffer, "%c%03d*%02d#", sign, degrees, minutes);
  return String(buffer);
}

// Format Latitude
String formatLatitude(double lat) {
  char sign = '+'; // default: West positif
  if (lat < 0) { // East = negatif (sesuai LX200)
    sign = '-';
  }
  lat = fabs(lat); // ubah ke nilai absolut
  int degrees = (int)lat;
  int minutes = (int)((lat - degrees) * 60.0 + 0.5);
  char buffer[16];
  sprintf(buffer, "%c%02d*%02d#", sign, degrees, minutes);
  return String(buffer);
}