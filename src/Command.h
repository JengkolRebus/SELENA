#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "Config.h"
#include "Move.h"
#include "Astro.h"
#include "Time.h"
#include "Alignment.h"

void handleCmd(const String &cmd, WiFiClient &client);

#endif