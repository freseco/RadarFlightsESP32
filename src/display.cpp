#include "display.h"

void animateZoom(bool zoomIn) {
  uint16_t radarColor = spr.color565(0, 180, 0);
  int frames = 25;
  int delayPerFrame = 15;
  
  for (int i = 0; i <= frames; i++) {
    float progress = (float)i / frames; // 0.0 to 1.0
    spr.fillSprite(TFT_BLACK);
    
    float r0, r1, r2, r3;
    if (zoomIn) {
      r0 = (radarRadius * 0.33) * progress; 
      r1 = radarRadius * 0.33 + (radarRadius * 0.33) * progress;
      r2 = radarRadius * 0.66 + (radarRadius * 0.34) * progress;
      r3 = radarRadius + (radarRadius * 0.33) * progress;
    } else {
      r0 = radarRadius * 0.33 * (1.0 - progress);
      r1 = radarRadius * 0.66 - (radarRadius * 0.33) * progress;
      r2 = radarRadius - (radarRadius * 0.34) * progress;
      r3 = (radarRadius * 1.33) - (radarRadius * 0.33) * progress;
    }
    
    if (r0 > 0 && r0 <= 120) spr.drawCircle(centerX, centerY, r0, radarColor);
    if (r1 > 0 && r1 <= 120) spr.drawCircle(centerX, centerY, r1, radarColor);
    if (r2 > 0 && r2 <= 120) spr.drawCircle(centerX, centerY, r2, radarColor);
    if (r3 > 0 && r3 <= 120) spr.drawCircle(centerX, centerY, r3, radarColor);
    
    spr.drawLine(centerX, centerY - radarRadius, centerX, centerY + radarRadius, radarColor);
    spr.drawLine(centerX - radarRadius, centerY, centerX + radarRadius, centerY, radarColor);
    
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.drawString("N", centerX, 8);
    spr.drawString("S", centerX, 240 - 8); 
    spr.drawString("E", 240 - 8, centerY);
    spr.drawString("O", 8, centerY);
    
    spr.pushSprite(0, 0);
    delay(delayPerFrame);
  }
}

void drawSplashScreen(String wifiStatus, uint16_t wifiColor) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(tft.color565(0, 220, 0), TFT_BLACK); 
  tft.drawString("RadarFlights", centerX, 40);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Por Freseco (2026)", centerX, 70);
  
  tft.setTextColor(tft.color565(180, 180, 180), TFT_BLACK);
  tft.drawString("freseco@gmail.com", centerX, 100);
  
  tft.setTextColor(tft.color565(100, 150, 255), TFT_BLACK);
  tft.drawString("Datos: Airplanes.live", centerX, 130);

  tft.setTextColor(tft.color565(200, 200, 200), TFT_BLACK);
  tft.drawString("v" + FIRMWARE_VERSION, centerX, 160);
  
  tft.setTextColor(wifiColor, TFT_BLACK);
  tft.drawString(wifiStatus, centerX, 190);
}

void drawRadarUI() {
  spr.fillSprite(TFT_BLACK);
  
  uint16_t radarColor = spr.color565(0, 180, 0);
  spr.drawCircle(centerX, centerY, radarRadius, radarColor);
  spr.drawCircle(centerX, centerY, radarRadius * 0.66, radarColor);
  spr.drawCircle(centerX, centerY, radarRadius * 0.33, radarColor);
  
  spr.drawLine(centerX, centerY - radarRadius, centerX, centerY + radarRadius, radarColor);
  spr.drawLine(centerX - radarRadius, centerY, centerX + radarRadius, centerY, radarColor);
  
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.drawString("N", centerX, 8);
  spr.drawString("S", centerX, 240 - 8); 
  spr.drawString("E", 240 - 8, centerY);
  spr.drawString("O", 8, centerY);
  
  if (pref_airport_id != "" && !pref_geoip) {
    spr.setTextColor(spr.color565(100, 100, 100), TFT_BLACK);
    spr.drawString(pref_airport_id, centerX, centerY + 12);
  }
  
  spr.setTextDatum(MR_DATUM);
  spr.drawString(String((int)pref_rad) + "km", centerX + radarRadius - 20, centerY + 12);
  
  spr.setTextDatum(BC_DATUM);
  
  // Contar cuántos vamos a dibujar para poner el texto exacto
  int count = 0;
  for (int i = 0; i < planes.size(); i++) {
    if (planes[i].distanceKm <= pref_rad) count++;
  }
  if (count > pref_max_planes) count = pref_max_planes;
  
  String planesText = "Aviones: " + String(count);
  spr.drawString(planesText, centerX, 240 - 20);
  
  // Mostrar solo el último octeto de la IP, asegurando que quede dentro de pantallas circulares
  IPAddress ip = WiFi.localIP();
  String ipEnd = "." + String(ip[3]);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(spr.color565(80, 80, 80), TFT_BLACK); // Un poco más visible
  spr.drawString(ipEnd, centerX + 4, 45);
  
  if (apiErrorMsg != "") {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(TFT_RED, TFT_BLACK);
    spr.drawString(apiErrorMsg, centerX, centerY - 20); // En medio de la pantalla
  }
}

void drawPlanes() {
  uint16_t planeColor = TFT_RED;
  if (pref_color == "blue") planeColor = TFT_BLUE;
  else if (pref_color == "orange") planeColor = TFT_ORANGE;
  
  uint16_t textColor = spr.color565(200, 200, 0); 
  
  spr.setTextDatum(ML_DATUM);
  
  int drawn = 0;
  
  std::vector<Airplane> localPlanes;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    localPlanes = planes;
    xSemaphoreGive(dataMutex);
  } else {
    localPlanes = planes;
  }
  
  for (int i = 0; i < localPlanes.size(); i++) {
    if (drawn >= pref_max_planes) break; // Limitar aviones dibujados
    
    Airplane p = localPlanes[i];
    
    if (p.distanceKm > pref_rad + 5.0) continue;
    
    float rad = (p.bearingDeg - 90.0) * M_PI / 180.0;
    
    if (p.distanceKm > pref_rad) {
      int px = centerX + ((radarRadius + 4) * cos(rad));
      int py = centerY + ((radarRadius + 4) * sin(rad));
      spr.fillCircle(px, py, 2, TFT_RED);
      drawn++;
      continue;
    }
    
    float screenDist = (p.distanceKm / pref_rad) * radarRadius;
    
    int px = centerX + (screenDist * cos(rad));
    int py = centerY + (screenDist * sin(rad));
    
    float headRad = (p.heading - 90.0) * M_PI / 180.0;
    float c_rad = cos(headRad);
    float s_rad = sin(headRad);
    
    auto rotate = [&](float x, float y, int& outX, int& outY) {
      outX = px + (x * c_rad - y * s_rad);
      outY = py + (x * s_rad + y * c_rad);
    };

    if (p.category == 8) { // Helicóptero
      int tX, tY;
      rotate(-7, 0, tX, tY); 
      spr.fillCircle(px, py, 3, planeColor);
      spr.drawLine(px, py, tX, tY, planeColor);
      spr.drawPixel(tX, tY, TFT_WHITE);
    } else if (p.category == 9 || p.category == 12) { // Planeador o Ultraligero
      int wlX, wlY, wrX, wrY, bX, bY;
      rotate(2, -8, wlX, wlY);
      rotate(2, 8, wrX, wrY);
      rotate(-2, 0, bX, bY);
      spr.drawLine(px, py, wlX, wlY, planeColor);
      spr.drawLine(px, py, wrX, wrY, planeColor);
      spr.drawLine(px, py, bX, bY, planeColor);
    } else if (p.category == 11) { // Dron
      int p1X, p1Y, p2X, p2Y, p3X, p3Y, p4X, p4Y;
      rotate(3, 3, p1X, p1Y);
      rotate(3, -3, p2X, p2Y);
      rotate(-3, 3, p3X, p3Y);
      rotate(-3, -3, p4X, p4Y);
      spr.fillRect(p1X-1, p1Y-1, 2, 2, planeColor);
      spr.fillRect(p2X-1, p2Y-1, 2, 2, planeColor);
      spr.fillRect(p3X-1, p3Y-1, 2, 2, planeColor);
      spr.fillRect(p4X-1, p4Y-1, 2, 2, planeColor);
      spr.drawPixel(px, py, TFT_WHITE);
    } else if (p.category == 10) { // Globo
      spr.fillCircle(px, py, 4, planeColor);
      int sqX, sqY;
      rotate(-5, 0, sqX, sqY);
      spr.fillRect(sqX - 1, sqY - 1, 3, 3, TFT_WHITE);
    } else if (p.category == 16 || p.category == 17) { // Vehículos de superficie
      spr.fillRect(px - 2, py - 2, 5, 5, planeColor);
    } else if (p.category == 1) { // Avioneta / Avión Pequeño
      int nX, nY, wlX, wlY, wrX, wrY, tailX, tailY;
      rotate(4, 0, nX, nY);       
      rotate(0, -4, wlX, wlY);   
      rotate(0, 4, wrX, wrY);    
      rotate(-4, 0, tailX, tailY); 
      spr.drawLine(nX, nY, tailX, tailY, planeColor); 
      spr.drawLine(wlX, wlY, wrX, wrY, planeColor);   
    } else if (p.category == 5) { // Avión Pesado (Jumbo)
      int nX, nY, wlX, wlY, wrX, wrY, rootX, rootY;
      rotate(10, 0, nX, nY);       
      rotate(-7, -8, wlX, wlY);   
      rotate(-7, 8, wrX, wrY);    
      rotate(-4, 0, rootX, rootY); 
      spr.fillTriangle(nX, nY, wlX, wlY, rootX, rootY, planeColor);
      spr.fillTriangle(nX, nY, wrX, wrY, rootX, rootY, planeColor);
    } else { // Avión Comercial Estándar
      int nX, nY, wlX, wlY, wrX, wrY, rootX, rootY;
      rotate(7, 0, nX, nY);       
      rotate(-5, -5, wlX, wlY);   
      rotate(-5, 5, wrX, wrY);    
      rotate(-2, 0, rootX, rootY); 
      spr.fillTriangle(nX, nY, wlX, wlY, rootX, rootY, planeColor);
      spr.fillTriangle(nX, nY, wrX, wrY, rootX, rootY, planeColor);
    }
    
    String cs = p.callsign;
    cs.trim(); 
    
    spr.setTextDatum(ML_DATUM);
    int textX = px + 8;
    
    bool hasCs = (cs.length() > 0);
    bool hasAlt = (p.altitude > 0);
    
    if (hasCs && hasAlt) {
      spr.setTextColor(textColor);
      spr.drawString(cs, textX, py - 5);
      spr.setTextColor(TFT_WHITE);
      spr.drawString(String((int)p.altitude) + "m", textX, py + 5);
    } else if (hasCs) {
      spr.setTextColor(textColor);
      spr.drawString(cs, textX, py);
    } else if (hasAlt) {
      spr.setTextColor(TFT_WHITE);
      spr.drawString(String((int)p.altitude) + "m", textX, py);
    }
    
    drawn++;
  }
}

void drawTimeUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  char dateStr[15];
  sprintf(dateStr, "%02d/%02d/%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
  
  spr.setTextDatum(MC_DATUM);
  
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(255, 150, 150), TFT_BLACK);
  spr.drawString("CPU: " + String((int)temperatureRead()) + "C", centerX, centerY - 90);
  
  spr.setTextFont(4);
  spr.setTextSize(2); 
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(timeStr, centerX, centerY - 30);
  
  spr.setTextFont(2);
  spr.setTextSize(2);
  spr.setTextColor(spr.color565(200, 200, 200), TFT_BLACK);
  spr.drawString(dateStr, centerX, centerY + 15);
  
  spr.setTextColor(spr.color565(150, 200, 255), TFT_BLACK);
  spr.drawString(pref_country, centerX, centerY + 45);
  
  int dbm = WiFi.RSSI();
  int wifiQuality = (dbm <= -100) ? 0 : ((dbm >= -50) ? 100 : 2 * (dbm + 100));
  spr.setTextColor(spr.color565(100, 255, 100), TFT_BLACK);
  spr.drawString("WiFi: " + String(wifiQuality) + "%", centerX, centerY + 75);
  
  spr.setTextFont(1); // Restore default font
  spr.setTextSize(1); // Restore size
  spr.pushSprite(0, 0);
}

void drawAnalogTimeUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  // Dibujar esfera del reloj
  uint16_t faceColor = spr.color565(30, 30, 30);
  spr.fillCircle(centerX, centerY, 118, faceColor);
  spr.drawCircle(centerX, centerY, 118, spr.color565(100, 100, 100));
  spr.drawCircle(centerX, centerY, 119, spr.color565(100, 100, 100));
  
  // Dibujar marcas de las horas y números principales
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, faceColor);
  
  for (int i = 0; i < 12; i++) {
    float angle = i * 30.0 * M_PI / 180.0;
    int x1 = centerX + 108 * sin(angle);
    int y1 = centerY - 108 * cos(angle);
    int x2 = centerX + 118 * sin(angle);
    int y2 = centerY - 118 * cos(angle);
    
    if (i == 0) {
      spr.drawString("12", centerX, centerY - 98);
    } else if (i == 3) {
      spr.drawString("3", centerX + 98, centerY);
    } else if (i == 6) {
      spr.drawString("6", centerX, centerY + 98);
    } else if (i == 9) {
      spr.drawString("9", centerX - 98, centerY);
    } else {
      spr.drawLine(centerX + 110 * sin(angle), centerY - 110 * cos(angle), x2, y2, spr.color565(180, 180, 180));
    }
  }
  
  // Mostrar AM o PM
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(200, 200, 200), faceColor);
  String ampm = (timeinfo->tm_hour < 12) ? "AM" : "PM";
  spr.drawString(ampm, centerX, centerY - 35);

  // Pequeña fase lunar
  int phase = getMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  int mX = centerX;
  int mY = centerY - 60;
  int mR = 12;
  uint16_t moonColor = spr.color565(240, 240, 200); 
  
  spr.fillCircle(mX, mY, mR, moonColor);
  
  switch(phase) {
    case 0: spr.fillCircle(mX, mY, mR, faceColor); break;
    case 1: spr.fillCircle(mX - 5, mY, mR, faceColor); break;
    case 2: spr.fillRect(mX - mR, mY - mR, mR, mR * 2, faceColor); break;
    case 3: spr.fillCircle(mX - 10, mY, mR, faceColor); break;
    case 4: break; // Llena
    case 5: spr.fillCircle(mX + 10, mY, mR, faceColor); break;
    case 6: spr.fillRect(mX, mY - mR, mR, mR * 2, faceColor); break;
    case 7: spr.fillCircle(mX + 5, mY, mR, faceColor); break;
  }
  
  // Dibujar el contorno sutil de la lunita para que destaque sobre el fondo
  spr.drawCircle(mX, mY, mR, spr.color565(100, 100, 100));
  
  // Calcular ángulos de las agujas
  float secAngle = timeinfo->tm_sec * 6.0 * M_PI / 180.0;
  float minAngle = (timeinfo->tm_min + timeinfo->tm_sec / 60.0) * 6.0 * M_PI / 180.0;
  float hrAngle = (timeinfo->tm_hour % 12 + timeinfo->tm_min / 60.0) * 30.0 * M_PI / 180.0;
  
  // Aguja de las horas (más gruesa, dibujando triángulos o varias líneas)
  int hx = centerX + 65 * sin(hrAngle);
  int hy = centerY - 65 * cos(hrAngle);
  spr.drawLine(centerX, centerY, hx, hy, TFT_WHITE);
  spr.drawLine(centerX + 1, centerY, hx + 1, hy, TFT_WHITE);
  spr.drawLine(centerX, centerY + 1, hx, hy + 1, TFT_WHITE);
  
  // Aguja de los minutos
  int mx = centerX + 100 * sin(minAngle);
  int my = centerY - 100 * cos(minAngle);
  spr.drawLine(centerX, centerY, mx, my, spr.color565(200, 200, 200));
  spr.drawLine(centerX + 1, centerY + 1, mx, my, spr.color565(200, 200, 200));
  
  // Aguja de los segundos (roja y delgada)
  spr.drawLine(centerX, centerY, centerX + 115 * sin(secAngle), centerY - 115 * cos(secAngle), TFT_RED);
  
  // Centro del reloj
  spr.fillCircle(centerX, centerY, 4, TFT_RED);
  
  // Mostrar la fecha abajo del reloj en digital
  char dateStr[15];
  sprintf(dateStr, "%02d/%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1);
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(150, 200, 255), faceColor);
  spr.drawString(dateStr, centerX, centerY + 70);

  int dbm = WiFi.RSSI();
  int wifiQuality = (dbm <= -100) ? 0 : ((dbm >= -50) ? 100 : 2 * (dbm + 100));
  spr.setTextColor(spr.color565(100, 255, 100), faceColor);
  spr.drawString("WiFi", centerX + 55, centerY - 8);
  spr.drawString(String(wifiQuality) + "%", centerX + 55, centerY + 8);

  // CPU temperature on the left side of the clock face
  spr.setTextColor(spr.color565(255, 150, 150), faceColor);
  spr.drawString("CPU", centerX - 55, centerY - 8);
  spr.drawString(String((int)temperatureRead()) + "\xF7", centerX - 55, centerY + 8);

  spr.pushSprite(0, 0);
}

void drawAnalog24hTimeUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  uint16_t faceColor = spr.color565(30, 30, 30);
  spr.fillCircle(centerX, centerY, 90, faceColor);
  
  // Dibujar el borde, pintándolo amarillo en las horas de luz de sol (ej. 7 a 20)
  spr.drawCircle(centerX, centerY, 90, spr.color565(100, 100, 100));
  spr.drawCircle(centerX, centerY, 91, spr.color565(100, 100, 100));
  
  for (int i = 0; i < 360; i++) {
    float h = i / 15.0;
    if (h >= 7 && h <= 20) {
      float angle = i * M_PI / 180.0;
      int x = centerX + 90 * sin(angle);
      int y = centerY - 90 * cos(angle);
      int x2 = centerX + 91 * sin(angle);
      int y2 = centerY - 91 * cos(angle);
      spr.drawPixel(x, y, TFT_YELLOW);
      spr.drawPixel(x2, y2, TFT_YELLOW);
    }
  }

  // Dibujar marcas de las horas y números principales (cada 3 horas)
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, faceColor);
  
  for (int i = 0; i < 24; i++) {
    float angle = i * 15.0 * M_PI / 180.0;
    int x1 = centerX + 80 * sin(angle);
    int y1 = centerY - 80 * cos(angle);
    int x2 = centerX + 90 * sin(angle);
    int y2 = centerY - 90 * cos(angle);
    
    if (i % 3 == 0) {
      int tx = centerX + 75 * sin(angle);
      int ty = centerY - 75 * cos(angle);
      spr.drawString(i == 0 ? "24h" : String(i), tx, ty);
    } else {
      spr.drawLine(centerX + 85 * sin(angle), centerY - 85 * cos(angle), x2, y2, spr.color565(180, 180, 180));
    }
  }

  // Pequeña fase lunar
  int phase = getMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  int mX = centerX;
  int mY = centerY - 45;
  int mR = 12;
  uint16_t moonColor = spr.color565(240, 240, 200); 
  
  spr.fillCircle(mX, mY, mR, moonColor);
  
  switch(phase) {
    case 0: spr.fillCircle(mX, mY, mR, faceColor); break;
    case 1: spr.fillCircle(mX - 5, mY, mR, faceColor); break;
    case 2: spr.fillRect(mX - mR, mY - mR, mR, mR * 2, faceColor); break;
    case 3: spr.fillCircle(mX - 10, mY, mR, faceColor); break;
    case 4: break;
    case 5: spr.fillCircle(mX + 10, mY, mR, faceColor); break;
    case 6: spr.fillRect(mX, mY - mR, mR, mR * 2, faceColor); break;
    case 7: spr.fillCircle(mX + 5, mY, mR, faceColor); break;
  }
  
  spr.drawCircle(mX, mY, mR, spr.color565(100, 100, 100));

  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(200, 200, 200), faceColor);
  
  char dateStr[10];
  sprintf(dateStr, "%02d/%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1);
  spr.drawString(dateStr, centerX, centerY + 55); 

  String ampm = (timeinfo->tm_hour < 12) ? "AM" : "PM";
  spr.drawString(ampm, centerX, centerY + 35);

  int dbm = WiFi.RSSI();
  int wifiQuality = (dbm <= -100) ? 0 : ((dbm >= -50) ? 100 : 2 * (dbm + 100));
  spr.setTextColor(spr.color565(100, 255, 100), faceColor);
  spr.drawString("WiFi", centerX + 45, centerY - 8);
  spr.drawString(String(wifiQuality) + "%", centerX + 45, centerY + 8);

  // Icono del Sol indicando el centro de las horas de luz (aprox 13:30)
  float sunAngle = 13.5 * 15.0 * M_PI / 180.0;
  int sunX = centerX + 105 * sin(sunAngle);
  int sunY = centerY - 105 * cos(sunAngle);
  spr.fillCircle(sunX, sunY, 8, TFT_YELLOW);
  for(int i=0; i<8; i++) {
     float a = i * 45 * M_PI / 180.0;
     spr.drawLine(sunX + cos(a)*10, sunY + sin(a)*10, sunX + cos(a)*14, sunY + sin(a)*14, TFT_YELLOW);
  }

  // Icono de la Luna indicando el centro de las horas de oscuridad (aprox 01:30)
  float moonAngle = 1.5 * 15.0 * M_PI / 180.0;
  int moonX = centerX + 105 * sin(moonAngle);
  int moonY = centerY - 105 * cos(moonAngle);
  spr.fillCircle(moonX, moonY, 8, spr.color565(220, 220, 220));
  spr.fillCircle(moonX + 3, moonY - 2, 7, TFT_BLACK); // Sombrear con el color del fondo


  // Calcular ángulos de las agujas
  float secAngle = timeinfo->tm_sec * 6.0 * M_PI / 180.0;
  float minAngle = (timeinfo->tm_min + timeinfo->tm_sec / 60.0) * 6.0 * M_PI / 180.0;
  float hrAngle = (timeinfo->tm_hour + timeinfo->tm_min / 60.0) * 15.0 * M_PI / 180.0;
  
  // Aguja de las horas (24h)
  int hx = centerX + 50 * sin(hrAngle);
  int hy = centerY - 50 * cos(hrAngle);
  spr.drawLine(centerX, centerY, hx, hy, TFT_WHITE);
  spr.drawLine(centerX + 1, centerY, hx + 1, hy, TFT_WHITE);
  spr.drawLine(centerX, centerY + 1, hx, hy + 1, TFT_WHITE);
  
  // Aguja de los minutos
  int mx = centerX + 75 * sin(minAngle);
  int my = centerY - 75 * cos(minAngle);
  spr.drawLine(centerX, centerY, mx, my, spr.color565(200, 200, 200));
  spr.drawLine(centerX + 1, centerY + 1, mx, my, spr.color565(200, 200, 200));
  
  // Aguja de los segundos
  spr.drawLine(centerX, centerY, centerX + 85 * sin(secAngle), centerY - 85 * cos(secAngle), TFT_RED);
  
  // Centro del reloj
  spr.fillCircle(centerX, centerY, 4, TFT_RED);
  


  // CPU temperature on the left side of the clock face
  spr.setTextColor(spr.color565(255, 150, 150), faceColor);
  spr.drawString("CPU", centerX - 55, centerY - 8);
  spr.drawString(String((int)temperatureRead()) + "\xF7", centerX - 55, centerY + 8);

  spr.pushSprite(0, 0);
}

void drawMoonUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  // Dibujar estrellas animadas
  static const int stars[][2] = {
    {30, 40}, {70, 25}, {200, 30}, {180, 80}, {40, 150},
    {210, 160}, {60, 200}, {190, 210}, {20, 100}, {220, 110},
    {110, 20}, {140, 220}, {230, 50}, {10, 180}, {90, 215}
  };
  
  for (int i = 0; i < 15; i++) {
    int sx = stars[i][0];
    int sy = stars[i][1];
    float t = millis() / (400.0 + i * 70.0) + i; 
    uint8_t b = 50 + 205 * (sin(t) + 1.0) / 2.0; 
    
    spr.drawPixel(sx, sy, spr.color565(b, b, b));
    if (i % 3 == 0) { // Algunas estrellas con un pequeño halo
      uint8_t halfB = b / 2;
      uint16_t haloColor = spr.color565(halfB, halfB, halfB);
      spr.drawPixel(sx+1, sy, haloColor);
      spr.drawPixel(sx-1, sy, haloColor);
      spr.drawPixel(sx, sy+1, haloColor);
      spr.drawPixel(sx, sy-1, haloColor);
    }
  }

  int phase = getMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  
  String phaseName = "";
  int mX = centerX;
  int mY = centerY - 15;
  int mR = 60;
  uint16_t moonColor = spr.color565(240, 240, 200); 
  uint16_t shadowColor = TFT_BLACK;
  
  spr.fillCircle(mX, mY, mR, moonColor);
  
  switch(phase) {
    case 0: 
      phaseName = "Luna Nueva";
      spr.fillCircle(mX, mY, mR, shadowColor);
      break;
    case 1: 
      phaseName = "Creciente Concava";
      spr.fillCircle(mX - 25, mY, mR, shadowColor);
      break;
    case 2: 
      phaseName = "Cuarto Creciente";
      spr.fillRect(mX - mR, mY - mR, mR, mR * 2, shadowColor);
      break;
    case 3: 
      phaseName = "Creciente Convexa";
      spr.fillCircle(mX - 50, mY, mR, shadowColor); 
      break;
    case 4: 
      phaseName = "Luna Llena";
      break;
    case 5: 
      phaseName = "Menguante Convexa";
      spr.fillCircle(mX + 50, mY, mR, shadowColor);
      break;
    case 6: 
      phaseName = "Cuarto Menguante";
      spr.fillRect(mX, mY - mR, mR, mR * 2, shadowColor);
      break;
    case 7: 
      phaseName = "Menguante Concava";
      spr.fillCircle(mX + 25, mY, mR, shadowColor);
      break;
  }

  // Dibujar un borde para que se vea la luna nueva
  spr.drawCircle(mX, mY, mR, spr.color565(100, 100, 100));

  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(phaseName, centerX, centerY + 65);
  
  spr.setTextFont(1);
  spr.setTextSize(1);
  spr.pushSprite(0, 0);
}

void drawWeatherIcon(int x, int y, int type) {
  // type: 0 = sun, 1 = moon, 2 = cloud, 3 = rain
  if (type == 0) { // Sun
    spr.fillCircle(x, y, 15, TFT_YELLOW);
    float angleOffset = (millis() % 36000) / 50.0; // Rotación: 1 grado cada 50ms
    float extraR = 3.0 * sin((millis() % 2000) / 2000.0 * 2.0 * M_PI); // Latido cíclico de 2 segundos
    for(int i=0; i<8; i++) {
       float a = (i * 45 + angleOffset) * M_PI / 180.0;
       spr.drawLine(x + cos(a)*18, y + sin(a)*18, x + cos(a)*(24 + extraR), y + sin(a)*(24 + extraR), TFT_YELLOW);
    }
  } else if (type == 1) { // Moon
    spr.fillCircle(x, y, 14, spr.color565(220, 220, 220));
    spr.fillCircle(x + 6, y - 4, 11, TFT_BLACK);
  } else if (type == 2) { // Cloud
    int cx = x + 8 * sin((millis() % 4000) / 4000.0 * 2.0 * M_PI); // Desplazamiento lateral, ciclo de 4s
    uint16_t cColor = spr.color565(180, 180, 180);
    spr.fillCircle(cx, y-2, 12, cColor);
    spr.fillCircle(cx - 12, y + 4, 10, cColor);
    spr.fillCircle(cx + 12, y + 4, 10, cColor);
    spr.fillRect(cx - 12, y + 4, 24, 10, cColor);
  } else if (type == 3) { // Rain
    int cx = x + 8 * sin((millis() % 4000) / 4000.0 * 2.0 * M_PI); // Desplazamiento lateral, ciclo de 4s
    uint16_t cColor = spr.color565(100, 100, 100);
    spr.fillCircle(cx, y-5, 12, cColor);
    spr.fillCircle(cx - 12, y, 10, cColor);
    spr.fillCircle(cx + 12, y, 10, cColor);
    spr.fillRect(cx - 12, y, 24, 10, cColor);
    
    int rainOffset = (millis() % 1000) / 100; // Lluvia cayendo, 0 a 9 cada 100ms
    uint16_t rColor = spr.color565(0, 150, 255);
    spr.drawLine(cx - 10, y + 10 + rainOffset, cx - 12, y + 14 + rainOffset, rColor);
    spr.drawLine(cx, y + 10 + rainOffset, cx - 2, y + 14 + rainOffset, rColor);
    spr.drawLine(cx + 10, y + 10 + rainOffset, cx + 8, y + 14 + rainOffset, rColor);
  }
}

void drawWeatherUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  WeatherData cw;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    cw = currentWeather;
    xSemaphoreGive(dataMutex);
  } else {
    cw = currentWeather;
  }
  
  if (!cw.valid) {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(TFT_RED, TFT_BLACK);
    spr.setTextFont(2);
    spr.setTextSize(1);
    spr.drawString("Sin datos del tiempo", centerX, centerY);
    if (pref_aemet_key == "") {
      spr.setTextColor(TFT_YELLOW, TFT_BLACK);
      spr.drawString("Falta API Key", centerX, centerY + 20);
    }
    spr.pushSprite(0, 0);
    return;
  }
  
  spr.setTextDatum(MC_DATUM);
  
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(150, 200, 255), TFT_BLACK);
  spr.drawString(cw.ubi, centerX, 25);
  
  // Icon logic
  int iconType = 0;
  bool isDay = (timeinfo->tm_hour >= 7 && timeinfo->tm_hour < 21);
  if (cw.prec > 0.0) {
    iconType = 3; // Rain
  } else if (cw.hr > 85.0) {
    iconType = 2; // Cloud
  } else {
    iconType = isDay ? 0 : 1; // Sun or Moon
  }
  
  drawWeatherIcon(centerX, centerY - 25, iconType);
  
  uint16_t tempColor;
  if (cw.ta <= 5.0) tempColor = TFT_WHITE;
  else if (cw.ta <= 15.0) tempColor = spr.color565(100, 200, 255); // Azul claro
  else if (cw.ta <= 25.0) tempColor = TFT_YELLOW;
  else if (cw.ta <= 35.0) tempColor = TFT_ORANGE;
  else tempColor = TFT_RED;

  spr.setTextFont(4); // 26px font
  spr.setTextSize(2);
  spr.setTextColor(tempColor, TFT_BLACK);
  spr.drawString(String(cw.ta, 1) + " C", centerX, centerY + 25);
  
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  String extra = "HR: " + String(cw.hr, 0) + "%";
  spr.drawString(extra, centerX, centerY + 60);
  
  int wY = centerY + 80;
  String windStr = String(cw.vv * 3.6, 1) + " km/h";
  spr.drawString(windStr, centerX + 20, wY);

  float windRad = (cw.dv + 90.0) * M_PI / 180.0;
  int aX = centerX - 45; 
  int aY = wY;
  int aR = 8; 
  int tipX = aX + aR * cos(windRad);
  int tipY = aY + aR * sin(windRad);
  int backX = aX - aR * cos(windRad);
  int backY = aY - aR * sin(windRad);
  
  spr.drawLine(backX, backY, tipX, tipY, spr.color565(150, 200, 255));
  float headRad1 = windRad + M_PI * 0.75;
  float headRad2 = windRad - M_PI * 0.75;
  spr.drawLine(tipX, tipY, tipX + 4 * cos(headRad1), tipY + 4 * sin(headRad1), spr.color565(150, 200, 255));
  spr.drawLine(tipX, tipY, tipX + 4 * cos(headRad2), tipY + 4 * sin(headRad2), spr.color565(150, 200, 255));
  
  // Dibujar puntos cardinales
  spr.setTextFont(1);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  spr.drawString("N", aX, aY - 14);
  spr.drawString("S", aX, aY + 14);
  spr.drawString("E", aX + 14, aY);
  spr.drawString("O", aX - 14, aY);

  if (cw.prec > 0) {
    spr.setTextColor(spr.color565(0, 200, 255), TFT_BLACK);
    spr.drawString("Prec: " + String(cw.prec, 1) + "mm", centerX, centerY + 100);
  }
  
  spr.setTextFont(1);
  spr.setTextSize(1);
  spr.pushSprite(0, 0);
}

void drawGhostPlane() {
  static int ghostDesign = 0;
  static bool designSelected = false;
  
  if (!ghostActive) {
    designSelected = false;
  } else if (ghostActive && !designSelected) {
    ghostDesign = esp_random() % 2;
    designSelected = true;
  }

  // Draw trail
  for (int i = 0; i < ghostTrail.size(); i++) {
    float angle = 0;
    if (i + 1 < ghostTrail.size()) {
      angle = atan2(ghostTrail[i+1].y - ghostTrail[i].y, ghostTrail[i+1].x - ghostTrail[i].x);
    } else if (i > 0) {
      angle = atan2(ghostTrail[i].y - ghostTrail[i-1].y, ghostTrail[i].x - ghostTrail[i-1].x);
    } else {
      angle = atan2(ghostVy, ghostVx);
    }
    
    float pAngle = angle + M_PI / 2.0;
    
    if (ghostDesign == 0) {
      int offsetX = 3.5 * cos(pAngle);
      int offsetY = 3.5 * sin(pAngle);
      spr.fillCircle((int)ghostTrail[i].x + offsetX, (int)ghostTrail[i].y + offsetY, 1, TFT_WHITE);
      spr.fillCircle((int)ghostTrail[i].x - offsetX, (int)ghostTrail[i].y - offsetY, 1, TFT_WHITE);
    } else {
      int off1X = 5.0 * cos(pAngle);
      int off1Y = 5.0 * sin(pAngle);
      int off2X = 10.0 * cos(pAngle);
      int off2Y = 10.0 * sin(pAngle);
      spr.fillCircle((int)ghostTrail[i].x + off1X, (int)ghostTrail[i].y + off1Y, 1, TFT_WHITE);
      spr.fillCircle((int)ghostTrail[i].x - off1X, (int)ghostTrail[i].y - off1Y, 1, TFT_WHITE);
      spr.fillCircle((int)ghostTrail[i].x + off2X, (int)ghostTrail[i].y + off2Y, 1, TFT_WHITE);
      spr.fillCircle((int)ghostTrail[i].x - off2X, (int)ghostTrail[i].y - off2Y, 1, TFT_WHITE);
    }
  }
  
  if (ghostActive) {
    // Draw the plane
    float angle = atan2(ghostVy, ghostVx);
    int px = (int)ghostX;
    int py = (int)ghostY;
    
    float c_rad = cos(angle);
    float s_rad = sin(angle);
    
    auto rotate = [&](float x, float y, int& outX, int& outY) {
      outX = px + (x * c_rad - y * s_rad);
      outY = py + (x * s_rad + y * c_rad);
    };

    if (ghostDesign == 0) {
      int noseX, noseY, backRX, backRY, backLX, backLY;
      rotate(12, 0, noseX, noseY);
      rotate(-14, -2, backRX, backRY);
      rotate(-14, 2, backLX, backLY);

      int wingRootX, wingRootY, wingLX, wingLY, wingRX, wingRY;
      rotate(2, 0, wingRootX, wingRootY);
      rotate(-4, 12, wingLX, wingLY);
      rotate(-4, -12, wingRX, wingRY);

      int tailRootX, tailRootY, tailLX, tailLY, tailRX, tailRY;
      rotate(-8, 0, tailRootX, tailRootY);
      rotate(-13, 5, tailLX, tailLY);
      rotate(-13, -5, tailRX, tailRY);

      spr.fillTriangle(noseX, noseY, backRX, backRY, backLX, backLY, TFT_WHITE);
      spr.fillTriangle(wingRootX, wingRootY, wingLX, wingLY, wingRX, wingRY, TFT_WHITE);
      spr.fillTriangle(tailRootX, tailRootY, tailLX, tailLY, tailRX, tailRY, TFT_WHITE);
    } else {
      int noseX, noseY, backRX, backRY, backLX, backLY;
      rotate(15, 0, noseX, noseY);
      rotate(-16, -3, backRX, backRY);
      rotate(-16, 3, backLX, backLY);

      int wingRootX, wingRootY, wingLX, wingLY, wingRX, wingRY;
      rotate(4, 0, wingRootX, wingRootY);
      rotate(-6, 16, wingLX, wingLY);
      rotate(-6, -16, wingRX, wingRY);
      
      int tailRootX, tailRootY, tailLX, tailLY, tailRX, tailRY;
      rotate(-10, 0, tailRootX, tailRootY);
      rotate(-15, 6, tailLX, tailLY);
      rotate(-15, -6, tailRX, tailRY);

      spr.fillTriangle(noseX, noseY, backRX, backRY, backLX, backLY, TFT_WHITE);
      spr.fillTriangle(wingRootX, wingRootY, wingLX, wingLY, wingRX, wingRY, TFT_WHITE);
      spr.fillTriangle(tailRootX, tailRootY, tailLX, tailLY, tailRX, tailRY, TFT_WHITE);
      
      int eInL_X, eInL_Y, eInR_X, eInR_Y;
      rotate(-2, 5, eInL_X, eInL_Y);
      rotate(-2, -5, eInR_X, eInR_Y);
      spr.fillCircle(eInL_X, eInL_Y, 2, TFT_LIGHTGREY);
      spr.fillCircle(eInR_X, eInR_Y, 2, TFT_LIGHTGREY);
      
      int eOutL_X, eOutL_Y, eOutR_X, eOutR_Y;
      rotate(-4, 10, eOutL_X, eOutL_Y);
      rotate(-4, -10, eOutR_X, eOutR_Y);
      spr.fillCircle(eOutL_X, eOutL_Y, 1, TFT_LIGHTGREY);
      spr.fillCircle(eOutR_X, eOutR_Y, 1, TFT_LIGHTGREY);
    }
  }
}
