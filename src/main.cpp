#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

#ifndef RGB_BUILTIN
#define RGB_BUILTIN 48
#endif
#include <time.h>

#include "globals.h"
#include "web_server.h"
#include "display.h"
#include "api.h"
#include "math_utils.h"

TaskHandle_t networkTaskHandle;

void networkTask(void *pvParameters) {
  while (true) {
    if (isAPMode) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    
    server.handleClient();
    
    unsigned long now = millis();
    if (now - lastWeatherFetch > 1800000 || lastWeatherFetch == 0) {
      if (WiFi.status() == WL_CONNECTED) {
        fetchAemetWeather();
      }
      lastWeatherFetch = millis();
    }
    
    if (now - lastFetchTime > fetchInterval || lastFetchTime == 0) {
      if (WiFi.status() == WL_CONNECTED) {
        fetchAirplanes();
      }
      lastFetchTime = millis();
    }
    
    if (now - lastIssFetch > 60000 || lastIssFetch == 0) {
      if (WiFi.status() == WL_CONNECTED) {
        fetchISSLocation();
      }
      lastIssFetch = millis();
    }
    
    if (now - lastSunFetch > 3600000 || lastSunFetch == 0) { // Every 1 hour
      if (WiFi.status() == WL_CONNECTED) {
        fetchSunTimes();
      }
      lastSunFetch = millis();
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Yield para el Watchdog y otras tareas RTOS
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  
  dataMutex = xSemaphoreCreateMutex();

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // Leer opciones de NVS
  preferences.begin("radar", false);
  
  // Reseteo de fábrica con botón BOOT (pin 0 en ESP32-S3)
  pinMode(0, INPUT_PULLUP);
  if (digitalRead(0) == LOW) {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Borrando memoria...", centerX, centerY);
    preferences.clear();
    delay(2000);
    ESP.restart();
  }

  pref_ssid = preferences.getString("ssid", "");
  pref_pass = preferences.getString("pass", "");
  pref_lat = preferences.getFloat("lat", 40.4722);
  pref_lon = preferences.getFloat("lon", -3.5609);
  pref_rad = preferences.getFloat("rad", 20.0);
  pref_max_planes = preferences.getInt("maxp", 15);
  pref_color = preferences.getString("color", "orange");
  pref_geoip = preferences.getBool("geoip", false);
  pref_airport_id = preferences.getString("airport_id", "MAD");
  pref_country = preferences.getString("country", "Spain");
  pref_lang = preferences.getString("lang", "es");
  pref_units = preferences.getString("units", "m");
  pref_offset = preferences.getLong("offset", 3600); // Default to UTC+1
  pref_dst = preferences.getBool("dst", true); // Default to DST ON for summer
  pref_ghost_mins = preferences.getInt("ghost", 10);
  pref_ghost_speed = preferences.getInt("ghost_speed", 150);
  pref_ghost_trail = preferences.getInt("ghost_trail", 100);
  pref_aemet_key = preferences.getString("aemet_key", "");
  pref_idema = preferences.getString("aemet_idema", "");
  
  pref_show_radar = preferences.getBool("sh_radar", true);
  pref_show_time = preferences.getBool("sh_time", true);
  pref_show_weather = preferences.getBool("sh_wea", true);
  pref_show_moon = preferences.getBool("sh_moon", true);
  pref_show_horizon = preferences.getBool("sh_horiz", true);
  pref_show_iss = preferences.getBool("sh_iss", true);
  pref_show_sun = preferences.getBool("sh_sun", true);
  
  pref_screen_time_s = preferences.getInt("screen_time", 30);
  pref_radar_time_s = preferences.getInt("radar_time", 30);

  configTime(pref_offset + (pref_dst ? 3600 : 0), 0, "pool.ntp.org", "time.nist.gov");

  spr.createSprite(240, 240);
  
  if (pref_ssid == "") {
    isAPMode = true;
    drawSplashScreen("Sin config. Iniciando AP...", tft.color565(255, 150, 0));
    delay(1500);
  } else {
    drawSplashScreen("Conectando WiFi...", TFT_YELLOW);
    WiFi.begin(pref_ssid.c_str(), pref_pass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) { // 10 segundos max
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      isAPMode = true;
      drawSplashScreen("Fallo WiFi. Iniciando AP...", TFT_RED);
      delay(1500);
    } else {
      drawSplashScreen("WiFi OK", TFT_GREEN);
      delay(1000);
    }
  }

  if (isAPMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(1, 2, 3, 4), IPAddress(1, 2, 3, 4), IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP32-Radar");
    dnsServer.start(53, "*", WiFi.softAPIP()); // Captive portal DNS intercept
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(tft.color565(0, 200, 255), TFT_BLACK);
    tft.drawString("MODO CONFIGURACION", centerX, 50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Unete a la red WiFi:", centerX, 90);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("ESP32-Radar", centerX, 120);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Y abre en el navegador:", centerX, 150);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("http://1.2.3.4", centerX, 180);
  } else {
    Serial.println("\nWiFi conectado! IP: " + WiFi.localIP().toString());
    
    // Obtener coordenadas por IP si está habilitado
    if (pref_geoip) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Localizando por IP...", centerX, centerY);
      
      HTTPClient httpGeo;
      httpGeo.begin("http://ip-api.com/json/?fields=status,lat,lon,country,offset");
      int code = httpGeo.GET();
      if (code == HTTP_CODE_OK) {
        String payload = httpGeo.getString();
        JsonDocument geoDoc;
        if (!deserializeJson(geoDoc, payload)) {
          if (geoDoc["status"] == "success") {
            pref_lat = geoDoc["lat"];
            pref_lon = geoDoc["lon"];
            if (geoDoc["country"]) pref_country = geoDoc["country"].as<String>();
            if (geoDoc["offset"]) {
                long fetchedOffset = geoDoc["offset"].as<long>();
                pref_offset = fetchedOffset;
                pref_dst = false; 
            }
            preferences.putFloat("lat", pref_lat);
            preferences.putFloat("lon", pref_lon);
            preferences.putString("country", pref_country);
            preferences.putLong("offset", pref_offset);
            preferences.putBool("dst", pref_dst);
            
            configTime(pref_offset, 0, "pool.ntp.org", "time.nist.gov");
            
            Serial.printf("GeoIP: %.4f, %.4f, %s, Offset: %ld\n", pref_lat, pref_lon, pref_country.c_str(), pref_offset);
          }
        }
      }
      httpGeo.end();
    }
    
    drawRadarUI();
    spr.pushSprite(0, 0);
  }
  
  if (MDNS.begin("radar")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started. Accede en: http://radar.local");
  }
  setupWebServer();
  
  xTaskCreatePinnedToCore(
    networkTask,
    "NetworkTask",
    10000,
    NULL,
    1,
    &networkTaskHandle,
    0 // Core 0
  );
  
  if (!pref_show_radar) {
    currentState = (DisplayState)(STATE_MAX - 1);
    nextState();
  }
}

void nextState() {
  for (int i = 0; i < STATE_MAX; i++) {
    int next = (currentState + 1) % STATE_MAX;
    currentState = (DisplayState)next;
    
    bool enabled = false;
    if (currentState == STATE_RADAR && pref_show_radar) enabled = true;
    else if (currentState == STATE_TIME && pref_show_time) enabled = true;
    else if (currentState == STATE_WEATHER && pref_show_weather) enabled = true;
    else if (currentState == STATE_MOON && pref_show_moon) enabled = true;
    else if (currentState == STATE_HORIZON && pref_show_horizon) enabled = true;
    else if (currentState == STATE_ISS && pref_show_iss) enabled = true;
    else if (currentState == STATE_SUN && pref_show_sun) enabled = true;
    
    if (enabled) {
      if (currentState == STATE_TIME) {
        if (pref_clock_mode == 0) {
          timeDisplayMode = (timeDisplayMode + 1) % 3;
        } else {
          timeDisplayMode = pref_clock_mode - 1;
        }
      }
      return;
    }
  }
  currentState = STATE_RADAR; // Fallback
}

void loop() {
  if (isAPMode) {
    server.handleClient();
    dnsServer.processNextRequest();
    return; // No ejecutamos el radar si estamos configurando
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Sin Conexion WiFi", centerX, centerY - 20);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Iniciando AP...", centerX, centerY + 20);
    delay(2000);
    
    isAPMode = true;
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(1, 2, 3, 4), IPAddress(1, 2, 3, 4), IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP32-Radar");
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(tft.color565(0, 200, 255), TFT_BLACK);
    tft.drawString("MODO CONFIGURACION", centerX, 50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Unete a la red WiFi:", centerX, 90);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("ESP32-Radar", centerX, 120);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Y abre en el navegador:", centerX, 150);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("http://1.2.3.4", centerX, 180);
    return;
  }
  
  unsigned long now = millis();
  
  // Manejo del botón BOOT para cambiar de pantalla manualmente
  static unsigned long lastBtnPress = 0;
  if (digitalRead(0) == LOW) {
    if (now - lastBtnPress > 500) { // 500ms debounce
      lastBtnPress = now;
      
      nextState();
      
      stateStartTime = now;
      spr.fillSprite(TFT_BLACK);
      lastDrawTime = 0; // Forzar dibujado inmediato
    }
  }
  
  // State Machine Logic
  struct tm timeinfo = {0};
  bool timeValid = getLocalTime(&timeinfo, 10);
  
  if (currentState == STATE_RADAR) {
    if (now - stateStartTime >= (pref_radar_time_s * 1000UL)) {
      nextState();
      stateStartTime = now;
      spr.fillSprite(TFT_BLACK);
      lastDrawTime = 0;
    }
  } else {
    if (now - stateStartTime >= (pref_screen_time_s * 1000UL)) {
      nextState();
      stateStartTime = now;
      spr.fillSprite(TFT_BLACK);
      lastDrawTime = 0;
    }
  }


  if (currentState == STATE_TIME) {
    if (now - lastDrawTime > 1000) {
      if (timeDisplayMode == 0) drawTimeUI(&timeinfo);
      else if (timeDisplayMode == 1) drawAnalogTimeUI(&timeinfo);
      else drawAnalog24hTimeUI(&timeinfo);
      lastDrawTime = now;
    }
    return; // Skip radar logic
  } else if (currentState == STATE_WEATHER) {
    if (now - lastDrawTime > 50) { // Reducido a 50ms para permitir animaciones fluidas
      drawWeatherUI(&timeinfo);
      lastDrawTime = now;
    }
    return; // Skip radar logic
  } else if (currentState == STATE_MOON) {
    if (now - lastDrawTime > 1000) { 
      drawMoonUI(&timeinfo);
      lastDrawTime = now;
    }
    return; // Skip radar logic
  } else if (currentState == STATE_HORIZON) {
    if (now - lastDrawTime > 100) { 
      drawArtificialHorizon();
      lastDrawTime = now;
    }
    return;

  } else if (currentState == STATE_ISS) {
    if (now - lastDrawTime > 1000) { 
      drawISS();
      lastDrawTime = now;
    }
    return;
  } else if (currentState == STATE_SUN) {
    if (now - lastDrawTime > 1000) { 
      drawSunArc(&timeinfo);
      lastDrawTime = now;
    }
    return;
  }
  
  if (pref_airport_id != "" && !pref_geoip) {
    if (!airportShownInitially) {
      showingAirport = true;
      airportDisplayStartTime = now;
      lastAirportShowTime = now;
      airportShownInitially = true;
    }
    
    if (!showingAirport && (now - lastAirportShowTime >= 180000)) { // 3 minutos
      showingAirport = true;
      airportDisplayStartTime = now;
      lastAirportShowTime = now;
    }
    
    if (showingAirport && (now - airportDisplayStartTime >= 3000)) { // 3 segundos
      showingAirport = false;
    }
  } else {
    showingAirport = false;
  }

  if (zoomAnimState > 0) {
    animateZoom(zoomAnimState == 1);
    zoomAnimState = 0;
    lastDrawTime = now; // Prevent immediate redraw skipping the final frame
  }
  
  // Ghost Plane Logic Update
  if (currentState == STATE_RADAR) {
    if (pref_ghost_mins > 0 && now - lastGhostSpawnTime > (pref_ghost_mins * 60000UL) && !ghostActive) {
      lastGhostSpawnTime = now;
      ghostActive = true;
      ghostTrail.clear();
      ghostTrailEndTime = 0;
      
      int side = random(4);
      float speed = (float)pref_ghost_speed;
      float startX, startY, endX, endY;
      
      if (side == 0) { // top -> bottom
        startX = random(40, 200); startY = -20;
        endX = random(40, 200); endY = 260;
      } else if (side == 1) { // right -> left
        startX = 260; startY = random(40, 200);
        endX = -20; endY = random(40, 200);
      } else if (side == 2) { // bottom -> top
        startX = random(40, 200); startY = 260;
        endX = random(40, 200); endY = -20;
      } else { // left -> right
        startX = -20; startY = random(40, 200);
        endX = 260; endY = random(40, 200);
      }
      
      ghostX = startX;
      ghostY = startY;
      
      float dx = endX - startX;
      float dy = endY - startY;
      float mag = sqrt(dx * dx + dy * dy);
      ghostVx = (dx / mag) * speed;
      ghostVy = (dy / mag) * speed;
    }
  }

  unsigned long currentDrawInterval = (ghostActive || !ghostTrail.empty()) ? 25 : drawInterval;

  if (now - lastDrawTime > currentDrawInterval) {
    float dt = (now - lastDrawTime) / 1000.0; 
    
    if (ghostActive) {
      ghostX += ghostVx * dt;
      ghostY += ghostVy * dt;
      
      if (ghostTrail.empty() || hypot(ghostX - ghostTrail.back().x, ghostY - ghostTrail.back().y) > 4.0) {
        ghostTrail.push_back({ghostX, ghostY});
      }
      if (ghostTrail.size() > pref_ghost_trail) {
        int idx = random(ghostTrail.size() - 1);
        ghostTrail.erase(ghostTrail.begin() + idx);
      }
      
      if (ghostX < -50 || ghostX > 290 || ghostY < -50 || ghostY > 290) {
         ghostActive = false;
      }
    } else if (!ghostTrail.empty()) {
      if (random(100) < 50) { 
        int idx = random(ghostTrail.size());
        ghostTrail.erase(ghostTrail.begin() + idx);
      }
    }
    
    for (int i = 0; i < planes.size(); i++) {
       if (planes[i].velocity > 0) {
         float distMovedKm = (planes[i].velocity * dt) / 3600.0;
         float dy = distMovedKm * cos(planes[i].heading * M_PI / 180.0);
         float dx = distMovedKm * sin(planes[i].heading * M_PI / 180.0);
         
         planes[i].lat += dy / 111.32;
         planes[i].lon += dx / (111.32 * cos(planes[i].lat * M_PI / 180.0));
         
         calculatePolar(planes[i]);
       }
    }
    
    drawRadarUI(); 
    drawPlanes();
    
    if (showingAirport) {
      String name = getAirportName(pref_airport_id);
      spr.setTextSize(3);
      int tWidth = spr.textWidth(name);
      int bWidth = tWidth + 30; // padding
      if (bWidth > 240) bWidth = 240;
      spr.fillRoundRect(centerX - bWidth/2, centerY - 25, bWidth, 50, 10, spr.color565(20, 20, 20));
      spr.drawRoundRect(centerX - bWidth/2, centerY - 25, bWidth, 50, 10, spr.color565(100, 100, 100));
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(TFT_WHITE);
      spr.drawString(name, centerX, centerY);
      spr.setTextSize(1);
    }
    
    drawGhostPlane();

    spr.pushSprite(0, 0); 
    
    lastDrawTime = now;
  }

  // Control WS2812 RGB LED on GPIO 21 (ESP32-S3-Zero)
  if (ledGreenUntil > now) {
    // Aterrizaje: Verde
    neopixelWrite(21, 0, 255, 0); 
  } else if (ledRedUntil > now) {
    // Nuevo avión: Rojo
    neopixelWrite(21, 255, 0, 0); 
  } else {
    // Apagado
    neopixelWrite(21, 0, 0, 0); 
  }
}