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
