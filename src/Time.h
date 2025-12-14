// src/Time.h

#ifndef TIME_H
#define TIME_H

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Deklarasi Objek Library (Memori dialokasikan di Time.cpp)
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// Prototipe Fungsi Waktu
void syncNTP();
void getCurrentTime();

#endif