#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <vector>
#include <deque>

// --- VARIABLES GLOBALES Y DE CONFIGURACIÓN ---
extern const String FIRMWARE_VERSION;
extern SemaphoreHandle_t dataMutex;

extern String pref_ssid;
extern String pref_pass;
extern float pref_lat;
extern float pref_lon;
extern float pref_rad;
extern int pref_max_planes;
extern String pref_color;
extern bool pref_geoip;
extern String pref_airport_id;
extern long pref_offset;
extern bool pref_dst;
extern int pref_ghost_mins;
extern int pref_ghost_speed;
extern int pref_ghost_trail;
extern int pref_clock_mode;


extern TFT_eSPI tft;
extern TFT_eSprite spr;

extern const int centerX;
extern const int centerY;
extern const int radarRadius;

extern unsigned long lastFetchTime;
extern unsigned long lastDrawTime;
extern unsigned long lastWeatherFetch;
extern String apiErrorMsg;
extern std::deque<String> errorLog;
void addErrorLog(String msg);
extern int zoomAnimState;
extern const unsigned long fetchInterval;
extern const unsigned long drawInterval;

extern unsigned long airportDisplayStartTime;
extern bool showingAirport;
extern unsigned long lastAirportShowTime;
extern bool airportShownInitially;

extern unsigned long ledGreenUntil;
extern unsigned long ledRedUntil;

enum DisplayState {
  STATE_RADAR,
  STATE_TIME,
  STATE_WEATHER,
  STATE_MOON
};
extern DisplayState currentState;
extern String pref_country;
extern unsigned long stateStartTime;
extern int timeDisplayMode;

extern String pref_aemet_key;
extern String pref_idema;

struct WeatherData {
  float ta;
  float hr;
  float prec;
  float vv;
  float dv;
  String ubi;
  bool valid;
};
extern WeatherData currentWeather;

struct Point2D {
  float x;
  float y;
};
extern unsigned long lastGhostSpawnTime;
extern bool ghostActive;
extern float ghostX;
extern float ghostY;
extern float ghostVx;
extern float ghostVy;
extern std::vector<Point2D> ghostTrail;
extern unsigned long ghostTrailEndTime;

struct Airplane {
  String callsign;
  float lat;
  float lon;
  float altitude;
  float heading;
  float velocity;
  float distanceKm;
  float bearingDeg;
  int category;
};

extern std::vector<Airplane> planes;

extern Preferences preferences;
extern WebServer server;
extern DNSServer dnsServer;
extern bool isAPMode;
