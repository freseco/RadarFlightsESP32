#include "api.h"

void fetchAirplanes() {
  Serial.println("Buscando aviones...");
  float distNM = (pref_rad + 5.0) * 0.539957; 
  String url = "https://api.airplanes.live/v2/point/" + String(pref_lat, 4) + "/" + String(pref_lon, 4) + "/" + String(distNM, 1);
  Serial.print("URL: "); Serial.println(url);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(10000); 
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
        JsonArray ac = doc["ac"];
        
        if (ac.isNull()) {
          Serial.println("El array 'ac' es null (no hay aviones en esta zona).");
        } else {
          int count = 0;
          for (JsonVariant v : ac) {
            JsonObject planeData = v.as<JsonObject>();
            Airplane p;
            p.altitude = 0; p.velocity = 0; p.callsign = ""; p.category = 0;
            bool on_ground = false;
            
            if (planeData["flight"]) {
              p.callsign = planeData["flight"].as<String>();
              p.callsign.trim();
            }
            if (planeData["lon"]) p.lon = planeData["lon"].as<float>();
            if (planeData["lat"]) p.lat = planeData["lat"].as<float>();
            if (planeData["track"]) p.heading = planeData["track"].as<float>();
            
            if (planeData["alt_baro"]) {
              if (planeData["alt_baro"].is<String>() && planeData["alt_baro"] == "ground") {
                 on_ground = true;
              } else {
                 p.altitude = planeData["alt_baro"].as<float>() * 0.3048; // Convertir de pies a metros
              }
            } else if (planeData["alt_geom"]) {
              p.altitude = planeData["alt_geom"].as<float>() * 0.3048; // Convertir de pies a metros (respaldo)
            }

            if (planeData["gs"]) p.velocity = planeData["gs"].as<float>() * 1.852; // Nudos a km/h
            
            if (planeData["category"]) {
               String cat = planeData["category"].as<String>();
               if (cat == "A1" || cat == "A2") p.category = 1; // Avioneta
               else if (cat == "A5") p.category = 5; // Heavy
               else if (cat == "A7") p.category = 8;
               else if (cat == "B1" || cat == "B4") p.category = 9;
               else if (cat == "B2") p.category = 10;
               else if (cat == "B5") p.category = 11;
               else if (cat.startsWith("C")) on_ground = true;
            }

            if (p.lat != 0 && p.lon != 0 && !on_ground) {
              calculatePolar(p);
              newPlanes.push_back(p);
              count++;
            }
          }
          Serial.printf("Se encontraron %d aviones válidos en vuelo.\n", count);
          
          if (dataMutex != NULL) {
            xSemaphoreTake(dataMutex, portMAX_DELAY);
            
            // Check for planes that landed or left (in planes but not in newPlanes)
            if (planes.size() > 0) { // Don't trigger on first boot or when empty
              for (auto& oldPlane : planes) {
                bool found = false;
                for (auto& newPlane : newPlanes) {
                  if (oldPlane.callsign == newPlane.callsign && oldPlane.callsign != "") {
                    found = true;
                    break;
                  }
                }
                if (!found) {
                  ledGreenUntil = millis() + 2000;
                }
              }
            }

            // Check for new planes (in newPlanes but not in planes)
            if (planes.size() > 0) { // Don't trigger on first boot
              for (auto& newPlane : newPlanes) {
                bool found = false;
                for (auto& oldPlane : planes) {
                  if (oldPlane.callsign == newPlane.callsign && newPlane.callsign != "") {
                    found = true;
                    break;
                  }
                }
                if (!found) {
                  ledRedUntil = millis() + 2000;
                }
              }
            }

            planes = newPlanes;
            xSemaphoreGive(dataMutex);
          }
        }
      } else {
        Serial.print("Error al parsear JSON: ");
        Serial.println(error.c_str());
        apiErrorMsg = "ERROR AL LEER JSON";
        addErrorLog("JSON Error: " + String(error.c_str()));
      }
    } else if (httpCode == 429) {
      apiErrorMsg = "LIMITE DIARIO EXCEDIDO (429)";
      addErrorLog("HTTP 429: Rate Limit Exceeded");
      Serial.println("Error 429: Rate Limit Exceeded");
    } else if (httpCode == 401 || httpCode == 403) {
      apiErrorMsg = "ACCESO DENEGADO API (" + String(httpCode) + ")";
      addErrorLog("HTTP " + String(httpCode) + ": Auth Error");
      Serial.println("Error Auth");
    } else {
      apiErrorMsg = "ERROR API: " + String(httpCode);
      addErrorLog("API Error HTTP: " + String(httpCode));
      Serial.println("Error de la API: " + String(httpCode));
    }
  } else {
    apiErrorMsg = "ERROR CONEXION API";
    addErrorLog("Conn Error: " + http.errorToString(httpCode));
    Serial.printf("Fallo de conexion: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void findClosestAemetStation() {
  if (pref_aemet_key == "") return;
  Serial.println("Buscando estacion AEMET mas cercana...");
  
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  String url = "https://opendata.aemet.es/opendata/api/observacion/convencional/todas";
  http.begin(client, url);
  http.addHeader("api_key", pref_aemet_key);
  
  int code = http.GET();
  if (code == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    String dataUrl = doc["datos"].as<String>();
    http.end();
    
    if (dataUrl != "") {
      http.begin(client, dataUrl);
      int code2 = http.GET();
      if (code2 == 200) {
        String payload = http.getString();
        
        float minDst = 999999.0;
        String closestId = "";
        
        int pos = 0;
        while(pos >= 0 && pos < payload.length()) {
          int idIdx = payload.indexOf("\"idema\"", pos);
          if (idIdx < 0) break;
          
          int idStart = payload.indexOf("\"", idIdx + 7) + 1;
          int idEnd = payload.indexOf("\"", idStart);
          String idema = payload.substring(idStart, idEnd);
          
          int latIdx = payload.indexOf("\"lat\"", idEnd);
          int latStart = payload.indexOf(":", latIdx) + 1;
          int latEnd = payload.indexOf(",", latStart);
          float lat = payload.substring(latStart, latEnd).toFloat();
          
          int lonIdx = payload.indexOf("\"lon\"", latEnd);
          int lonStart = payload.indexOf(":", lonIdx) + 1;
          int lonEnd = payload.indexOf(",", lonStart);
          float lon = payload.substring(lonStart, lonEnd).toFloat();
          
          float dLat = (lat - pref_lat) * M_PI / 180.0;
          float dLon = (lon - pref_lon) * M_PI / 180.0;
          float a = sin(dLat/2)*sin(dLat/2) + cos(pref_lat*M_PI/180.0)*cos(lat*M_PI/180.0)*sin(dLon/2)*sin(dLon/2);
          float dst = 6371.0 * 2 * atan2(sqrt(a), sqrt(1-a));
          
          if (dst < minDst) {
            minDst = dst;
            closestId = idema;
          }
          pos = payload.indexOf("}", lonEnd);
        }
        
        if (closestId != "") {
          pref_idema = closestId;
          preferences.putString("aemet_idema", pref_idema);
          Serial.println("Estacion mas cercana: " + pref_idema);
        }
      }
    }
  }
  http.end();
}

void fetchAemetWeather() {
  if (pref_aemet_key == "") return;
  if (pref_idema == "") {
    findClosestAemetStation();
  }
  if (pref_idema == "") return;
  
  Serial.println("Consultando AEMET estacion " + pref_idema);
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  String url = "https://opendata.aemet.es/opendata/api/observacion/convencional/datos/estacion/" + pref_idema;
  http.begin(client, url);
  http.addHeader("api_key", pref_aemet_key);
  
  int code = http.GET();
  if (code == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    String dataUrl = doc["datos"].as<String>();
    http.end();
    
    if (dataUrl != "") {
      http.begin(client, dataUrl);
      if (http.GET() == 200) {
        String payload = http.getString();
        JsonDocument wdoc;
        DeserializationError err = deserializeJson(wdoc, payload);
        if (!err) {
          JsonArray arr = wdoc.as<JsonArray>();
          if (arr.size() > 0) {
            JsonObject last = arr[arr.size() - 1]; // get the last element which is the most recent
            WeatherData newWeather;
            newWeather.ta = last["ta"].as<float>();
            newWeather.hr = last["hr"].as<float>();
            newWeather.prec = last["prec"].as<float>();
            newWeather.vv = last["vv"].as<float>();
            newWeather.dv = last["dv"].as<float>();
            newWeather.ubi = last["ubi"].as<String>();
            newWeather.valid = true;
            
            if (dataMutex != NULL) {
              xSemaphoreTake(dataMutex, portMAX_DELAY);
              currentWeather = newWeather;
              xSemaphoreGive(dataMutex);
            }
            
            Serial.println("AEMET OK: " + newWeather.ubi + " " + String(newWeather.ta) + "C");
          }
        }
      }
    }
  }
  http.end();
}

void fetchISSLocation() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  WiFiClient client;
  http.begin(client, "http://api.open-notify.org/iss-now.json");
  http.setTimeout(5000);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      if (doc["message"] == "success") {
        iss_lat = doc["iss_position"]["latitude"].as<float>();
        iss_lon = doc["iss_position"]["longitude"].as<float>();
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
        
        int y, M, d, h, m, s;
        if (sscanf(sr.c_str(), "%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
          h += (pref_offset / 3600);
          if (pref_dst) h += 1;
          if (h >= 24) h -= 24; else if (h < 0) h += 24;
          char buf[10];
          sprintf(buf, "%02d:%02d", h, m);
          sunriseTimeStr = String(buf);
          int sr_mins = h * 60 + m;
          
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
