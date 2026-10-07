#pragma once
#include "globals.h"
#include "math_utils.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetchAirplanes();
void fetchISSLocation();
void fetchISSPass();
void fetchSunTimes();
void fetchOpenMeteoWeather();
void fetchElectricityData();
void fetchAirQuality();
void fetchCryptoData();
