#include "Command.h"

void handleCmd(const String &cmd, WiFiClient &client) {
  if (cmd == "#") {
    client.print("1#");
    client.flush();
  }

  else if (cmd.startsWith(":U")) {
    // Serial.println("TX: SEND NOTHING | :U");
  }
  else if (cmd.startsWith(":GVP")) {
    client.print(String(FIRMWARE_NAME) + "#");
    client.flush();
  }
  else if (cmd.startsWith(":GVN")) {
    client.print(String(FIRMWARE_VERSION) + "#");
    client.flush();
  }
  else if (cmd.startsWith(":GVD")) {
    client.print(String(FIRMWARE_DATE) + "#");
    client.flush();
  }
  else if (cmd.startsWith(":GVT")) {
    client.print(String(FIRMWARE_TIME) + "#");
    client.flush();
  }
  // :Gg - Get Current Site Longitude sDDD.MM#
  else if (cmd.startsWith(":Gg")) {
    client.print(String(formatLongitude(lon)));
    client.flush();
  }
  // :Gt - Get Current Site Latitdue sDD*MM#
  else if (cmd.startsWith(":Gt")) {
    client.print(String(formatLatitude(lat)));
    client.flush();
  }

  // :GC - Get Current Date MM/DD/YY#
  else if (cmd.startsWith(":GC")) {
    client.print(currentDate);
    client.print('#');
    client.flush();
  }

  // :GL# Get Local Time in 24 hour format HH:MM:SS#
  else if (cmd.startsWith(":GL")) {
    client.print(currentTime);
    client.print('#');
    client.flush();
  }

  //:GG# Get UTC offset time sHH# or sHH.H#
  else if (cmd.startsWith(":GG")) {
    double offset = 0.0;
    char sign = '+';
    if (UTC_OFFSET > 0) {
      sign = '-';
    }
    else if (UTC_OFFSET < 0) {
      sign = '+';
    }
    offset = fabs(UTC_OFFSET);
    char buf[14];
    // jika offset-nya bulat (misal 7.0)
    if (fmod(offset, 1.0) == 0.0) {
      sprintf(buf, "%c%02d#", sign, (int)offset);
    } else { // jika ada pecahan jam, misal 5.5
      sprintf(buf, "%c%04.1f#", sign, offset);
    }
    client.print(buf);
    client.flush();
  }

  // :D# - Distnace Bars
  else if (cmd.startsWith(":D")) {
    if (isMoving == true) {
      client.print("|#");
    }
    else {
      client.print('#');
    }
    client.flush();
  }

  // :GW# - Unknown command
  else if (cmd.startsWith(":GW")) {
    client.print('A');
    client.print(isTracking);
    client.print(alignmentStatus);
    client.flush();
  }

  // :GR# Get Telescope RA  HH:MM.T# or HH:MM:SS#
  else if (cmd.startsWith(":GR")) {
    double ra_decimal = currentRA;
    int HH = (int)ra_decimal;
    double sisa_jam = ra_decimal - HH;
    double menit_desimal = sisa_jam * 60.0;
    int MM = (int)menit_desimal;
    double sisa_menit = menit_desimal - MM;
    int SS = (sisa_menit * 60.0) + 0.5;
    char buffer[15];
    sprintf(buffer, "%02d:%02d:%02d#", HH, MM, SS);
    client.print(buffer);
    client.flush();
  }

  // :GD# Get Telescope Declination sDD*MM# or sDD*MM’SS#
  else if (cmd.startsWith(":GD")) {
    double dec_decimal = currentDEC;
    char sign_char = '+';
    if (dec_decimal < 0.0) {
      sign_char = '-';
      dec_decimal = fabs(dec_decimal); // Ambil nilai absolut untuk perhitungan
    }

    int DD = (int)dec_decimal;
    double sisa_derajat = dec_decimal - DD;
    double menit_desimal = sisa_derajat * 60.0;
    int MM = (int)menit_desimal;
    double sisa_menit = menit_desimal - MM;
    int SS = (sisa_menit * 60.0) + 0.5;
    char buffer[15];
    sprintf(buffer, "%c%02d*%02d'%02d#", sign_char, DD, MM, SS);
    client.print(buffer);
    client.flush();
  }

  // :SG#
  else if (cmd.startsWith(":SG")) {
    String val = cmd.substring(3);
    val.trim();
    if (val.length() >= 2 && (val[0] == '+' || val[0] == '-')) {
      float offset = val.toFloat();

      if (offset >= -12.0 && offset <= 14.0) {
        UTC_OFFSET = -offset;
        client.print("1"); // balasan valid
        client.flush();
      }
      else {
        client.print("0");
        client.flush();
      }
    }
    else {
      client.print("0");
      client.flush();
    }
  }

  // :St#
  else if (cmd.startsWith(":St")) {
    // Ambil substring setelah ":St"
    String val = cmd.substring(3);
    val.trim(); // hapus spasi / newline

    // Hapus '#' di akhir jika ada
    if (val.endsWith("#")) {
      val.remove(val.length() - 1);
    }

    // Contoh val: "+07*30" atau "-06*45"
    if (val.length() >= 5 && (val[0] == '+' || val[0] == '-')) {
      char sign = val[0];
      int starIndex = val.indexOf('*');
      if (starIndex != -1) {
        String degStr = val.substring(1, starIndex);
        String minStr = val.substring(starIndex + 1);

        int deg = degStr.toInt();
        int min = minStr.toInt();

        // Validasi rentang nilai
        if (deg >= 0 && deg <= 90 && min >= 0 && min < 60) {
          double latitude = deg + (min / 60.0);
          if (sign == '-') latitude = -latitude;

          lat = latitude;

          client.print("1");
          client.flush();
        } else {
          client.print("0");
          client.flush();
        }
      } else {
        client.print("0");
        client.flush();
      }
    } else {
      client.print("0");
      client.flush();
    }
  }

  //:SgDDD*MM#
  else if (cmd.startsWith(":Sg")) {
    String data = cmd.substring(3); // ambil antara 'Sg' dan '#'
    data.trim();
    int separator = data.indexOf('*');
    if (separator == -1) {
      client.print("0"); // invalid format
      client.flush();
      return;
    }
    String degStr = data.substring(0, separator);
    String minStr = data.substring(separator + 1);

    int deg = degStr.toInt();
    int min = minStr.toInt();

    if (deg < 0 || deg > 360 || min < 0 || min >= 60) {
      client.print("0");
      client.flush();
      //      Serial.println("Nilai longitude invalid");
      return;
    }
    double newLon = deg + (min / 60.0);
    if (newLon > 180.0) newLon = -(360.0 - newLon);
    lon = -newLon;

    client.print("1"); // valid
    client.flush();
  }

  // :SLHH:MM:SS# — set time, pertahankan tanggal
  else if (cmd.startsWith(":SL")) {
    String val = cmd.substring(3);
    val.trim();
    if (val.endsWith("#")) val.remove(val.length() - 1);

    int HH, MM, SS;
    if (sscanf(val.c_str(), "%02d:%02d:%02d", &HH, &MM, &SS) == 3) {
      // pertahankan tanggal
      int D = day();
      int Mo = month();
      int Y = year();
      setTime(HH, MM, SS + 2, D, Mo, Y);
      client.print("1");
      client.flush();
    }
    else {
      client.print("0");
      client.flush();
    }
  }

  // Handler :SC - set date (MM/DD/YY) — hanya ubah tanggal, waktu (HH:MM:SS) dipertahankan
  else if (cmd.startsWith(":SC")) {
    String val = cmd.substring(3);
    val.trim();

    int MM_in, DD_in, YY_in;
    if (sscanf(val.c_str(), "%02d/%02d/%02d", &MM_in, &DD_in, &YY_in) == 3) {
      int tahun = 2000 + YY_in;
      // pertahankan jam saat ini
      int HH = hour();
      int Min = minute();
      int Sec = second();
      setTime(HH, Min, Sec, DD_in, MM_in, tahun);
      client.print("1");
      client.print("Updating Planetary Data##");
      client.flush();
    } else {
      client.print("0");
      client.flush();
    }
  }

  //:SrHH:MM:SS#
  //  Set target object RA to HH:MM.T or HH:MM:SS depending on the current precision setting.
  //Returns:
  //0 – Invalid
  //1 - Valid
  else if (cmd.startsWith(":Sr")) {
    targetTime = currentEpoch = now();
    String val = cmd.substring(3);
    val.trim();

    int HH, MM, SS;
    if (sscanf(val.c_str(), "%02d:%02d:%02d", &HH, &MM, &SS) == 3) {
      targetRA = HH + (MM / 60.0) + (SS / 3600.0);
      client.print("1");
      client.flush();
      Serial.printf("Target RA: %02d:%02d:%02d\n", HH, MM, SS);
    }
    else {
      client.print("0");
      client.flush();
      Serial.println("Format :Sr salah");
    }
  }

  //  :SdsDD*MM#
  //Set target object declination to sDD*MM or sDD*MM:SS depending on the current precision setting
  //Returns:
  //1 - Dec Accepted
  //0 – Dec invalid
  else if (cmd.startsWith(":Sd")) {
    String val = cmd.substring(3);
    val.trim();
    char s = '-';
    int DD, MM, SS;
    if (sscanf(val.c_str(), "%c%02d:%02d:%02d", &s, &DD, &MM, &SS) == 4) {
      targetDEC = DD + (MM / 60.0) + (SS / 3600.0);
      if (s == '-') {
        targetDEC = -targetDEC;
      }
      RADEC_TO_ALTAZ(targetRA, targetDEC, lat, lon, targetTime, targetALT, targetAZ);
      client.print("1");
      client.flush();
      Serial.printf("Target DEC: %c%02d:%02d:%02d\n", s, DD, MM, SS);
      Serial.printf("Target Time: %s\n", String(targetTime).c_str());
      Serial.printf("Target LAT: %.02f\n", lat);
      Serial.printf("Target LON: %.02f\n", lon);
      Serial.printf("Target ALT: %.02f\n", targetALT);
      Serial.printf("Target AZ: %0.2f\n", targetAZ);
    }
    else {
      client.print("0");
      client.flush();
      Serial.println("Format :Sd salah");
    }
  }

  //:MS# Slew to Target Object
  // Returns:
  //  0 Slew is Possible
  //  1<string># Object Below Horizon w/string message
  //  2<string># Object Below Higher w/string message
  else if (cmd.startsWith(":MS")) {
    client.print("0");
    client.flush();
    haveTarget = true;

    Serial.printf("Alt: %.2f Az: %.2f RA: %.2f DEC: %.2f T_Alt: %.2f T_Az: %.2f T_RA: %.2f T_DEC: %.2f\n",
                  currentALT, currentAZ, currentRA, currentDEC, targetALT, targetAZ, targetRA, targetDEC);
    Serial.println("Sleeeeeeeeeew");

    // set slew speed
    SET_SLEW_SPEED(4);

    SLEW_TO_TARGET(targetALT, targetAZ);
    while (alt_stepper.distanceToGo() != 0 || az_stepper.distanceToGo() != 0) {
      alt_stepper.run();
      az_stepper.run();
      // yield() diperlukan untuk menjaga koneksi WiFi/OS ESP8266 tetap hidup
      yield();
    }
    isSlewing = false;
    isTracking = 'T';
  }

  // set slewing speed:
  // :RG# Set Slew rate to Guiding Rate (slowest)
  // :RC# Set Slew rate to Centering rate (2nd slowest)
  // :RM# Set Slew rate to Find Rate (2nd Fastest)
  // :RS# Set Slew rate to max (fastest)
  else if (cmd.startsWith(":RG")) {
    SET_SLEW_SPEED(0);
    Serial.println("Slew Speed Change: 1");
  }
  else if (cmd.startsWith(":RC")) {
    SET_SLEW_SPEED(1);
    Serial.println("Slew Speed Change: 2");
  }
  else if (cmd.startsWith(":RM")) {
    SET_SLEW_SPEED(2);
    Serial.println("Slew Speed Change: 3");
  }
  else if (cmd.startsWith(":RS")) {
    SET_SLEW_SPEED(3);
    Serial.println("Slew Speed Change: 4");
  }

  // Move Axis
  //  :Me# Move Telescope East at current slew rate
  //  :Mn# Move Telescope North at current slew rate
  //  :Ms# Move Telescope South at current slew rate
  //  :Mw# Move Telescope West at current slew rate
  else if (cmd.startsWith(":Mw")) {
    isTracking ='N';
    az_stepper.moveTo(az_stepper.currentPosition() + maxSpeedSteps);
    Serial.println(+maxSpeedSteps);
  }
  else if (cmd.startsWith(":Me")) {
    isTracking ='N';
    az_stepper.moveTo(az_stepper.currentPosition() - maxSpeedSteps);
    Serial.println(-maxSpeedSteps);
  }
  else if (cmd.startsWith(":Mn")) {
    isTracking ='N';
    alt_stepper.moveTo(alt_stepper.currentPosition() + maxSpeedSteps);
    Serial.println(+maxSpeedSteps);
  }
  else if (cmd.startsWith(":Ms")) {
    isTracking ='N';
    alt_stepper.moveTo(alt_stepper.currentPosition() - maxSpeedSteps);
    Serial.println(-maxSpeedSteps);
  }

  // Halt Slewing
  //  :Q# Halt all current slewing
  //  :Qe# Halt eastward Slews
  //  :Qn# Halt northward Slews
  //  :Qs# Halt southward Slews
  //  :Qw# Halt westward Slews
  else if (cmd.startsWith(":Q")) {
    alt_stepper.stop();
    az_stepper.stop();
    GET_CURRENT_POS();
    getCurrentTime();
    ALTAZ_TO_RADEC(currentALT, currentAZ, lat, lon, currentEpoch, targetRA, targetDEC);
    if (haveTarget){
      isTracking = 'T';
    }
    Serial.println("ALT AZ Stop.");
  }
  else if (cmd.startsWith(":Qe")) {
    az_stepper.stop();
    Serial.println("AZ Stop.");
  }
  else if (cmd.startsWith(":Qw")) {
    az_stepper.stop();
    Serial.println("AZ Stop.");
  }
  else if (cmd.startsWith(":Qn")) {
    alt_stepper.stop();
    Serial.println("ALT Stop.");
  }
  else if (cmd.startsWith(":Qs")) {
    alt_stepper.stop();
    Serial.println("ALT Stop.");
  }

  else if (cmd.startsWith(":CM")){
    addDataPoint(targetAZ, targetALT, currentAZ, currentALT);
    APPLY_MODEL(model[0], model[1], model[2], model[3], targetALT, targetAZ); // mendapatkan nilai koreksi ALT dan AZ
    client.print('#');

  }







  else{
    //     unknown: respond generic
    client.print("1#");
    //    Serial.println("Unknown Respon");
  }
}