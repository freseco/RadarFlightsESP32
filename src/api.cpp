#include "api.h"

void fetchAirplanes() {
  Serial.println("Buscando aviones (OpenSky)...");

  // Bounding box from center + radius
  float deg = pref_rad / 111.32f;  // km → degrees
  float latMin = pref_lat - deg;
  float latMax = pref_lat + deg;
  float lonDeg = pref_rad / (111.32f * cos(pref_lat * M_PI / 180.0f));
  float lonMin = pref_lon - lonDeg;
  float lonMax = pref_lon + lonDeg;

  String url = "https://opensky-network.org/api/states/all"
               "?lamin=" + String(latMin, 4) +
               "&lomin=" + String(lonMin, 4) +
               "&lamax=" + String(latMax, 4) +
               "&lomax=" + String(lonMax, 4);
  Serial.print("URL: "); Serial.println(url);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(10000);
  http.setUserAgent("RadarFlightsESP32/1.0");
  int httpCode = http.GET();

  Serial.printf("HTTP Code: %d\n", httpCode);

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      apiErrorMsg = "";
      String payload = http.getString();
      Serial.printf("Payload size: %d bytes\n", payload.length());

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        std::vector<Airplane> newPlanes;
        JsonArray states = doc["states"];

        if (states.isNull()) {
          Serial.println("No hay aviones en esta zona.");
        } else {
          int count = 0;
          for (JsonArray state : states) {
            // OpenSky state vector indices:
            // 0=icao24, 1=callsign, 5=lon, 6=lat, 7=baro_alt(m),
            // 8=on_ground, 9=velocity(m/s), 10=true_track(deg)
            if (state.size() < 11) continue;

            bool on_ground = state[8].as<bool>();
            if (on_ground) continue;

            float lat = state[6].isNull() ? 0 : state[6].as<float>();
            float lon = state[5].isNull() ? 0 : state[5].as<float>();
            if (lat == 0 && lon == 0) continue;

            Airplane p;
            p.callsign = state[1].isNull() ? "" : state[1].as<String>();
            p.callsign.trim();
            p.lat      = lat;
            p.lon      = lon;
            p.altitude = state[7].isNull() ? 0 : state[7].as<float>(); // already in metres
            p.velocity = state[9].isNull() ? 0 : state[9].as<float>() * 3.6f; // m/s → km/h
            p.heading  = state[10].isNull() ? 0 : state[10].as<float>();
            p.category = 0;

            calculatePolar(p);
            newPlanes.push_back(p);
            count++;
            if (count >= pref_max_planes) break;
          }
          Serial.printf("Aviones válidos: %d\n", count);

          if (dataMutex != NULL) {
            xSemaphoreTake(dataMutex, portMAX_DELAY);

            // Landed / left
            if (planes.size() > 0) {
              for (auto& oldPlane : planes) {
                bool found = false;
                for (auto& newPlane : newPlanes) {
                  if (oldPlane.callsign == newPlane.callsign && oldPlane.callsign != "") { found = true; break; }
                }
                if (!found) ledGreenUntil = millis() + 2000;
              }
            }
            // New arrivals
            if (planes.size() > 0) {
              for (auto& newPlane : newPlanes) {
                bool found = false;
                for (auto& oldPlane : planes) {
                  if (oldPlane.callsign == newPlane.callsign && newPlane.callsign != "") { found = true; break; }
                }
                if (!found) ledRedUntil = millis() + 2000;
              }
            }

            planes = newPlanes;
            xSemaphoreGive(dataMutex);
          }
        }
      } else {
        Serial.print("Error JSON: "); Serial.println(error.c_str());
        apiErrorMsg = "ERROR AL LEER JSON";
        addErrorLog("JSON Error: " + String(error.c_str()));
      }
    } else if (httpCode == 429) {
      apiErrorMsg = "LIMITE DIARIO EXCEDIDO (429)";
      addErrorLog("HTTP 429: Rate Limit");
    } else if (httpCode == 401 || httpCode == 403) {
      apiErrorMsg = "ACCESO DENEGADO API (" + String(httpCode) + ")";
      addErrorLog("HTTP " + String(httpCode) + ": Auth Error");
    } else {
      apiErrorMsg = "ERROR API: " + String(httpCode);
      addErrorLog("API Error HTTP: " + String(httpCode));
    }
  } else {
    apiErrorMsg = "ERROR CONEXION API";
    addErrorLog("Conn Error: " + http.errorToString(httpCode));
  }
  http.end();
}




void fetchOpenMeteoWeather() {
  Serial.println("Consultando Open-Meteo para lat " + String(pref_lat, 4) + " lon " + String(pref_lon, 4));
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(pref_lat, 4) + "&longitude=" + String(pref_lon, 4) + "&current=temperature_2m,relative_humidity_2m,precipitation,weather_code,wind_speed_10m,wind_direction_10m,is_day";
  http.begin(client, url);
  
  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    JsonDocument wdoc;
    DeserializationError err = deserializeJson(wdoc, payload);
    if (!err) {
      if (wdoc.containsKey("current")) {
        JsonObject current = wdoc["current"];
        WeatherData newWeather;
        newWeather.ta = current["temperature_2m"].as<float>();
        newWeather.hr = current["relative_humidity_2m"].as<float>();
        newWeather.prec = current["precipitation"].as<float>();
        newWeather.vv = current["wind_speed_10m"].as<float>();
        newWeather.dv = current["wind_direction_10m"].as<float>();
        newWeather.weather_code = current["weather_code"].as<int>();
        newWeather.is_day = current["is_day"].as<int>();
        newWeather.tamax = newWeather.ta; // Open-Meteo current API doesn't provide max/min easily without daily, so we just use current.
        newWeather.tamin = newWeather.ta;
        newWeather.ubi = "Open-Meteo";
        newWeather.valid = true;
        
        if (dataMutex != NULL) {
          xSemaphoreTake(dataMutex, portMAX_DELAY);
          currentWeather = newWeather;
          xSemaphoreGive(dataMutex);
        }
        
        Serial.println("Open-Meteo OK: " + String(newWeather.ta) + "C, code: " + String(newWeather.weather_code));
      }
    }
  } else {
    Serial.println("Open-Meteo error HTTP: " + String(code));
  }
  http.end();
}

void fetchISSLocation() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); // wheretheiss.at uses HTTPS
  http.begin(client, "https://api.wheretheiss.at/v1/satellites/25544");
  http.setUserAgent("RadarFlightsESP32/1.0");
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(5000);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      if (doc.containsKey("latitude") && doc.containsKey("longitude")) {
        if (iss_lat != 0.0) {
          iss_last_lat = iss_lat;
          iss_last_lon = iss_lon;
        }
        iss_lat = doc["latitude"].as<float>();
        iss_lon = doc["longitude"].as<float>();
      }
    }
  }
  http.end();
}

void fetchISSPass() {
  if (pref_n2yo_key == "") {
    iss_next_pass_time     = 0;
    iss_next_pass_max_el   = 0;
    iss_next_pass_duration = 0;
    return;
  }
  if (WiFi.status() != WL_CONNECTED) return;

  // n2yo.com: próximos pasos visibles de la ISS (NORAD 25544) con elevación ≥10°
  // Endpoint: /rest/v1/satellite/visualpasses/25544/{lat}/{lon}/{alt}/{days}/{minElevation}&apiKey={key}
  String url = "https://api.n2yo.com/rest/v1/satellite/visualpasses/25544/"
               + String(pref_lat, 4) + "/"
               + String(pref_lon, 4) + "/0/2/10&apiKey=" + pref_n2yo_key;

  Serial.println("Consultando pases ISS n2yo...");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(10000);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      JsonArray passes = doc["passes"];
      if (!passes.isNull() && passes.size() > 0) {
        // Obtener tiempo actual UTC en Unix
        struct tm now_tm;
        time_t now_unix = 0;
        if (getLocalTime(&now_tm, 10)) {
          // Convertir a UTC restando el offset
          struct tm utc_tm = now_tm;
          time_t local_unix = mktime(&utc_tm);
          now_unix = local_unix - pref_offset;
          if (pref_dst) now_unix -= 3600;
        }

        // Buscar el primer paso que aún no haya empezado
        for (JsonVariant p : passes) {
          long startUTC = p["startUTC"].as<long>();
          int maxEl     = p["maxEl"].as<int>();
          int duration  = p["duration"].as<int>();
          if (startUTC > now_unix) {
            iss_next_pass_time     = startUTC;
            iss_next_pass_max_el   = maxEl;
            iss_next_pass_duration = duration;
            Serial.printf("Proximo paso ISS: UTC %ld, elev max %d°, %ds\n",
                          startUTC, maxEl, duration);
            break;
          }
        }
      } else {
        // Sin pasos visibles en los próximos 2 días
        iss_next_pass_time   = -1;
        iss_next_pass_max_el = 0;
        Serial.println("No hay pasos ISS visibles en 2 dias");
      }
    }
  } else {
    Serial.printf("Error n2yo: %d\n", code);
    addErrorLog("n2yo ISS: HTTP " + String(code));
  }
  http.end();
}

void fetchSunTimes() {
  if (WiFi.status() != WL_CONNECTED) return;
  String url = "https://api.sunrise-sunset.org/json?lat=" + String(pref_lat, 4) + "&lng=" + String(pref_lon, 4) + "&formatted=0";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(8000);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      if (doc["status"] == "OK") {
        String sr = doc["results"]["sunrise"].as<String>();
        String ss = doc["results"]["sunset"].as<String>();
        String sn = doc["results"]["solar_noon"].as<String>();
        
        int y, M, d, h, m, s;
        if (sscanf(sr.c_str(), "%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
          h += (pref_offset / 3600);
          if (pref_dst) h += 1;
          if (h >= 24) h -= 24; else if (h < 0) h += 24;
          char buf[10];
          sprintf(buf, "%02d:%02d", h, m);
          sunriseTimeStr = String(buf);
          int sr_mins = h * 60 + m;
          
          if (sscanf(sn.c_str(), "%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
            h += (pref_offset / 3600);
            if (pref_dst) h += 1;
            if (h >= 24) h -= 24; else if (h < 0) h += 24;
            char buf[10];
            sprintf(buf, "%02d:%02d", h, m);
            solarNoonTimeStr = String(buf);
          }

          if (sscanf(ss.c_str(), "%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
            h += (pref_offset / 3600);
            if (pref_dst) h += 1;
            if (h >= 24) h -= 24; else if (h < 0) h += 24;
            sprintf(buf, "%02d:%02d", h, m);
            sunsetTimeStr = String(buf);
            int ss_mins = h * 60 + m;
            
            struct tm now_tm;
            if (getLocalTime(&now_tm)) {
               int now_mins = now_tm.tm_hour * 60 + now_tm.tm_min;
               if (now_mins >= sr_mins && now_mins <= ss_mins) {
                 // Day time (0.0 to 0.5)
                 sun_progress = ((float)(now_mins - sr_mins) / (float)(ss_mins - sr_mins)) * 0.5;
               } else {
                 // Night time (0.5 to 1.0)
                 int night_elapsed = (now_mins > ss_mins) ? (now_mins - ss_mins) : ((1440 - ss_mins) + now_mins);
                 int night_total = (1440 - ss_mins) + sr_mins;
                 sun_progress = 0.5 + ((float)night_elapsed / (float)night_total) * 0.5;
               }
            }
          }
        }
      }
    }
  }
  http.end();
}
void fetchElectricityData() {
  if (WiFi.status() != WL_CONNECTED) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) return;

  char url[150];
  sprintf(url, "https://api.esios.ree.es/archives/70/download?date=%04d-%02d-%02d&format=json", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(8000);
  http.addHeader("User-Agent", "Mozilla/5.0");
  http.addHeader("Accept", "application/json");
  
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonArray pvpc = doc["PVPC"];
        if (!pvpc.isNull() && pvpc.size() >= 24) {
          for (int h = 0; h < 24; h++) {
            String pcb_str = pvpc[h]["PCB"].as<String>();
            pcb_str.replace(",", "."); // Convert 123,45 to 123.45
            electricity_prices[h] = pcb_str.toFloat() / 1000.0f;
          }
          lastElectricityFetch = millis();
        } else {
          addErrorLog("Electricity JSON array missing");
        }
      } else {
        addErrorLog("Electricity JSON err");
      }
    } else {
      addErrorLog("Electricity HTTP " + String(httpCode));
    }
  } else {
    addErrorLog("Electricity API failed");
  }
  http.end();
}
void fetchAirQuality() {
  if (WiFi.status() != WL_CONNECTED) return;

  // Nearest station by geo coordinates
  String url = "https://api.waqi.info/feed/geo:" + String(pref_lat, 4) + ";" + String(pref_lon, 4) + "/?token=" + pref_aqicn_token;
  Serial.println("Consultando calidad del aire: " + url);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(8000);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err && doc["status"] == "ok") {
      AirQualityData newAQI;
      newAQI.aqi = doc["data"]["aqi"].as<int>();

      // Sub-pollutants (may not always be present)
      JsonObject iaqi = doc["data"]["iaqi"];
      newAQI.pm25 = iaqi["pm25"]["v"] | -1.0f;
      newAQI.pm10 = iaqi["pm10"]["v"] | -1.0f;
      newAQI.o3   = iaqi["o3"]["v"]   | -1.0f;
      newAQI.no2  = iaqi["no2"]["v"]  | -1.0f;

      // Station name
      newAQI.stationName = doc["data"]["city"]["name"].as<String>();
      // Keep only last part after last comma for brevity
      int commaIdx = newAQI.stationName.lastIndexOf(',');
      if (commaIdx > 0) newAQI.stationName = newAQI.stationName.substring(0, commaIdx);
      if (newAQI.stationName.length() > 22) newAQI.stationName = newAQI.stationName.substring(0, 22);

      newAQI.valid = true;

      if (dataMutex != NULL) {
        xSemaphoreTake(dataMutex, portMAX_DELAY);
        currentAQI = newAQI;
        xSemaphoreGive(dataMutex);
      }
      Serial.printf("AQI: %d, PM2.5: %.1f, Estacion: %s\n", newAQI.aqi, newAQI.pm25, newAQI.stationName.c_str());
    } else {
      addErrorLog("AQI JSON err: " + String(doc["status"].as<String>()));
      Serial.println("AQI error: " + String(doc["status"].as<String>()));
    }
  } else {
    addErrorLog("AQI HTTP: " + String(code));
    Serial.println("AQI HTTP error: " + String(code));
  }
  http.end();
}
