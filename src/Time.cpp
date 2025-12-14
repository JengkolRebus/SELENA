// src/Time.cpp

#include "Config.h" // Perlu akses ke currentEpoch, currentDate, currentTime, UTC_OFFSET
#include "Time.h"
#include <Arduino.h>

double utc_offsett_in_second = UTC_OFFSET *3600;

// Definisi Objek Library
WiFiUDP ntpUDP;
// Inisialisasi timeClient di sini, menggunakan konstanta dari Config.h
NTPClient timeClient(ntpUDP, "pool.ntp.org", utc_offsett_in_second);

void syncNTP() {
  if (timeClient.update()) {
    setTime(timeClient.getEpochTime());
    Serial.println("Waktu disinkronkan dari NTP.");
  } else {
    Serial.println("Gagal update NTP (akan coba lagi nanti).");
  }
}

void getCurrentTime() {
  // ambil waktu aktif dari TimeLib
  currentEpoch = now();
  
  int tahun = year(currentEpoch);
  int bulan = month(currentEpoch);
  int tanggal = day(currentEpoch);
  int jam = hour(currentEpoch);
  int menit = minute(currentEpoch);
  int detik = second(currentEpoch);

  char buf[25];
  sprintf(buf, "%02d/%02d/%02d", bulan, tanggal, tahun%100);
  currentDate = buf;
  sprintf(buf, "%02d:%02d:%02d", jam, menit, detik);
  currentTime = buf;
}