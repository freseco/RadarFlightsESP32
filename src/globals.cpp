#include "globals.h"

SemaphoreHandle_t dataMutex = NULL;
const String FIRMWARE_VERSION = "26081201";

String pref_ssid = "";
String pref_pass = "";
float pref_lat = 40.4722;
float pref_lon = -3.5609;
float pref_rad = 20.0;
int pref_max_planes = 15;
String pref_color = "orange"; 
bool pref_geoip = false;
String pref_airport_id = "MAD";
long pref_offset = 7200; // default 2h (CEST)
bool pref_dst = false;
int pref_ghost_mins = 10;
int pref_ghost_speed = 150;
int pref_ghost_trail = 100;
int pref_clock_mode = 0; // 0=Cycle, 1=Digital, 2=Analog12h, 3=Analog24h


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); 

const int centerX = 120;
const int centerY = 120;
const int radarRadius = 110;

unsigned long lastFetchTime = 0;
unsigned long lastDrawTime = 0;
unsigned long lastWeatherFetch = 0;
String apiErrorMsg = "";
std::deque<String> errorLog;
void addErrorLog(String msg) {
  struct tm timeinfo;
  char timeStr[20] = "";
  if (getLocalTime(&timeinfo, 10)) {
    sprintf(timeStr, "[%02d:%02d:%02d] ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  }
  errorLog.push_front(String(timeStr) + msg);
  if (errorLog.size() > 5) {
    errorLog.pop_back();
  }
}
int zoomAnimState = 0;
const unsigned long fetchInterval = 10000;
const unsigned long drawInterval = 1000;   

unsigned long airportDisplayStartTime = 0;
bool showingAirport = false;
unsigned long lastAirportShowTime = 0;
bool airportShownInitially = false;

unsigned long ledGreenUntil = 0;
unsigned long ledRedUntil = 0;

DisplayState currentState = STATE_RADAR;
String pref_country = "Spain";
unsigned long stateStartTime = 0;
int timeDisplayMode = 0;

String pref_aemet_key = "";
String pref_idema = "";

WeatherData currentWeather;

unsigned long lastGhostSpawnTime = 0;
bool ghostActive = false;
float ghostX = 0;
float ghostY = 0;
float ghostVx = 0;
float ghostVy = 0;
std::vector<Point2D> ghostTrail;
unsigned long ghostTrailEndTime = 0;

std::vector<Airplane> planes;

Preferences preferences;
WebServer server(80);
DNSServer dnsServer;
bool isAPMode = false;
