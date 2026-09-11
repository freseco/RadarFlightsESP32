#include "display.h"
#include "world_map.h"

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
    spr.drawString(tr("N", "N"), centerX, 8);
    spr.drawString(tr("S", "S"), centerX, 240 - 8); 
    spr.drawString(tr("E", "E"), 240 - 8, centerY);
    spr.drawString(tr("O", "W"), 8, centerY);
    
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
  tft.drawString(tr("Por Freseco (2026)", "By Freseco (2026)"), centerX, 70);
  
  tft.setTextColor(tft.color565(180, 180, 180), TFT_BLACK);
  tft.drawString("freseco@gmail.com", centerX, 100);
  
  tft.setTextColor(tft.color565(100, 150, 255), TFT_BLACK);
  tft.drawString(tr("Datos: OpenSky Network", "Data: OpenSky Network"), centerX, 130);

  tft.setTextColor(tft.color565(200, 200, 200), TFT_BLACK);
  tft.drawString("v" + FIRMWARE_VERSION, centerX, 160);
  
  tft.setTextColor(wifiColor, TFT_BLACK);
  tft.drawString(wifiStatus, centerX, 190);
}

void drawRadarUI() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextSize(1);
  spr.setTextFont(1);
  
  uint16_t radarColor = spr.color565(0, 180, 0);
  spr.drawCircle(centerX, centerY, radarRadius, radarColor);
  spr.drawCircle(centerX, centerY, radarRadius * 0.66, radarColor);
  spr.drawCircle(centerX, centerY, radarRadius * 0.33, radarColor);
  
  spr.drawLine(centerX, centerY - radarRadius, centerX, centerY + radarRadius, radarColor);
  spr.drawLine(centerX - radarRadius, centerY, centerX + radarRadius, centerY, radarColor);
  
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(tr("N", "N"), centerX, 8);
  spr.drawString(tr("S", "S"), centerX, 240 - 8); 
  spr.drawString(tr("E", "E"), 240 - 8, centerY);
  spr.drawString(tr("O", "W"), 8, centerY);
  
  if (pref_airport_id != "" && !pref_geoip) {
    spr.setTextColor(spr.color565(100, 100, 100), TFT_BLACK);
    spr.drawString(pref_airport_id, centerX, centerY + 12);
  }
  
  spr.setTextDatum(MR_DATUM);
  spr.drawString(String((int)pref_rad) + "km", centerX + radarRadius - 20, centerY + 12);
  
  spr.setTextDatum(BC_DATUM);
  
  std::vector<Airplane> localPlanes;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    localPlanes = planes;
    xSemaphoreGive(dataMutex);
  } else {
    localPlanes = planes;
  }

  //    <a href="https://opensky-network.org/aircraft-profile" target="_blank" style="color: #4CAF50; text-decoration: none; font-size: 16px;">🌍 Ver en OpenSky Network</a>
  int count = 0;
  for (int i = 0; i < localPlanes.size(); i++) {
    if (localPlanes[i].distanceKm <= pref_rad) count++;
  }
  if (count > pref_max_planes) count = pref_max_planes;
  
  String planesText = tr("Aviones: ", "Planes: ") + String(count);
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
    
    String altStr = "";
    if (pref_units == "ft") {
      altStr = String((int)(p.altitude * 3.28084)) + "ft";
    } else {
      altStr = String((int)p.altitude) + "m";
    }

    if (hasCs && hasAlt) {
      spr.setTextColor(textColor);
      spr.drawString(cs, textX, py - 5);
      spr.setTextColor(TFT_WHITE);
      spr.drawString(altStr, textX, py + 5);
    } else if (hasCs) {
      spr.setTextColor(textColor);
      spr.drawString(cs, textX, py);
    } else if (hasAlt) {
      spr.setTextColor(TFT_WHITE);
      spr.drawString(altStr, textX, py);
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
  String cpuStr = "CPU: " + String((int)temperatureRead());
  spr.drawString(cpuStr, centerX, centerY - 90);
  int cpuW = spr.textWidth(cpuStr);
  spr.drawCircle(centerX + cpuW / 2 + 5, centerY - 90 - 6, 2, spr.color565(255, 150, 150));
  
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
  spr.setTextFont(4);
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
  spr.drawString(ampm, centerX + 22, centerY - 96);

  // Apertura clásica de fase lunar (estilo reloj mecánico)
  float moon_fraction = getMoonPhaseFraction(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  int mX = centerX;
  int mY = centerY - 50;
  int R = 24; // Radio de la apertura principal

  // 1. Fondo azul noche (bóveda celeste)
  uint16_t skyColor = spr.color565(10, 20, 60);
  spr.fillCircle(mX, mY, R, skyColor);
  
  // 2. Estrellas estáticas en el fondo
  spr.drawPixel(mX - 10, mY - 15, TFT_WHITE);
  spr.drawPixel(mX + 12, mY - 10, TFT_WHITE);
  spr.drawPixel(mX + 5,  mY - 20, TFT_WHITE);
  spr.drawPixel(mX - 18, mY - 5,  TFT_WHITE);

  // 3. Dibujar la luna (se mueve en arco según la fracción)
  // moon_fraction va de 0.0 (nueva) a 0.5 (llena) a 1.0 (nueva)
  float moonAngle = (-180.0 + moon_fraction * 180.0) * M_PI / 180.0;
  int mR = 8; // Radio de la luna
  int orbitR = R - mR - 2; 
  int moonX = mX + orbitR * cos(moonAngle);
  int moonY = mY + orbitR * sin(moonAngle);
  uint16_t moonColor = spr.color565(255, 240, 150);
  spr.fillCircle(moonX, moonY, mR, moonColor);

  // 4. Máscara inferior para crear la apertura con forma de "pecho"
  spr.fillRect(mX - R, mY, R * 2 + 1, R + 1, faceColor);
  
  // Dos círculos que suben para hacer la forma característica
  int humpR = 14; 
  spr.fillCircle(mX - 12, mY, humpR, faceColor);
  spr.fillCircle(mX + 12, mY, humpR, faceColor);
  
  // Contorno sutil del arco superior
  spr.drawCircle(mX, mY, R, spr.color565(80, 80, 80));
  
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
  spr.drawString("CPU", centerX - 45, centerY - 8);
  String cpuStrA = String((int)temperatureRead());
  spr.drawString(cpuStrA, centerX - 45, centerY + 8);
  int cpuWA = spr.textWidth(cpuStrA);
  spr.drawCircle(centerX - 45 + cpuWA / 2 + 5, centerY + 8 - 6, 2, spr.color565(255, 150, 150));

  spr.pushSprite(0, 0);
}

struct BlinkStar24h {
  int x, y;
  uint8_t maxBrightness;
  float phase;
  float speed;
};

struct ShootingStar24h {
  float x, y;
  float vx, vy;
  int len;
  bool active;
  unsigned long startMs;
  unsigned long durationMs;
};

struct Bird24h {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long startMs;
  unsigned long durationMs;
  float flapSpeed;
};

static BlinkStar24h mStars24[40];
static ShootingStar24h mShoot24[2];
static Bird24h mBirds[4];
static bool anim24hInit = false;

void drawAnalog24hTimeUI(struct tm* timeinfo) {
  if (!anim24hInit) {
    for (int i = 0; i < 40; i++) {
      mStars24[i].x = random(0, 240);
      mStars24[i].y = random(0, 240);
      mStars24[i].maxBrightness = random(100, 255);
      mStars24[i].phase = random(0, 314) / 100.0;
      mStars24[i].speed = random(2, 10) / 100.0;
    }
    for (int i = 0; i < 2; i++) mShoot24[i].active = false;
    for (int i = 0; i < 4; i++) mBirds[i].active = false;
    anim24hInit = true;
  }
  
  spr.fillSprite(TFT_BLACK);
  unsigned long now = millis();
  
  // Dibujar estrellas parpadeantes (noche)
  for (int i = 0; i < 40; i++) {
    int dx = mStars24[i].x - centerX;
    int dy = mStars24[i].y - centerY;
    float r = sqrt(dx*dx + dy*dy);
    if (r > 91) {
      float angle = atan2(dx, -dy) * 180.0 / M_PI;
      if (angle < 0) angle += 360;
      float h_star = angle / 15.0;
      if (h_star < 7 || h_star > 20) {
        mStars24[i].phase += mStars24[i].speed;
        int b = mStars24[i].maxBrightness * abs(sin(mStars24[i].phase));
        spr.drawPixel(mStars24[i].x, mStars24[i].y, spr.color565(b, b, b));
      }
    }
  }

  // Estrellas fugaces (noche)
  for (int i = 0; i < 2; i++) {
    if (!mShoot24[i].active) {
      if (random(1000) < 5) {
        mShoot24[i].active = true;
        mShoot24[i].startMs = now;
        mShoot24[i].durationMs = random(400, 1000);
        mShoot24[i].len = random(5, 15);
        if (random(2) == 0) {
          mShoot24[i].x = random(0, 100);
          mShoot24[i].y = random(0, 60);
          mShoot24[i].vx = random(30, 80) / 10.0;
          mShoot24[i].vy = random(10, 40) / 10.0;
        } else {
          mShoot24[i].x = random(140, 240);
          mShoot24[i].y = random(0, 60);
          mShoot24[i].vx = -random(30, 80) / 10.0;
          mShoot24[i].vy = random(10, 40) / 10.0;
        }
      }
    } else {
      float t = (float)(now - mShoot24[i].startMs) / mShoot24[i].durationMs;
      if (t > 1.0) {
        mShoot24[i].active = false;
      } else {
        mShoot24[i].x += mShoot24[i].vx;
        mShoot24[i].y += mShoot24[i].vy;
        int dx = (int)mShoot24[i].x - centerX;
        int dy = (int)mShoot24[i].y - centerY;
        float r = sqrt(dx*dx + dy*dy);
        if (r > 91) {
          float angle = atan2(dx, -dy) * 180.0 / M_PI;
          if (angle < 0) angle += 360;
          float h_star = angle / 15.0;
          if (h_star < 7 || h_star > 20) {
            float tailX = mShoot24[i].x - (mShoot24[i].vx * mShoot24[i].len * 0.1);
            float tailY = mShoot24[i].y - (mShoot24[i].vy * mShoot24[i].len * 0.1);
            spr.drawLine((int)mShoot24[i].x, (int)mShoot24[i].y, (int)tailX, (int)tailY, TFT_WHITE);
          }
        }
      }
    }
  }

  // Pájaros (día)
  for (int i = 0; i < 4; i++) {
    if (!mBirds[i].active) {
      if (random(1000) < 10) {
        mBirds[i].active = true;
        mBirds[i].startMs = now;
        mBirds[i].durationMs = random(4000, 8000);
        mBirds[i].flapSpeed = random(20, 50) / 100.0;
        if (random(2) == 0) {
          mBirds[i].x = 0;
          mBirds[i].y = random(150, 240);
          mBirds[i].vx = random(10, 30) / 10.0;
          mBirds[i].vy = random(-10, 10) / 10.0;
        } else {
          mBirds[i].x = 240;
          mBirds[i].y = random(150, 240);
          mBirds[i].vx = -random(10, 30) / 10.0;
          mBirds[i].vy = random(-10, 10) / 10.0;
        }
      }
    } else {
      float t = (float)(now - mBirds[i].startMs) / mBirds[i].durationMs;
      if (t > 1.0 || mBirds[i].x < -10 || mBirds[i].x > 250 || mBirds[i].y < -10 || mBirds[i].y > 250) {
        mBirds[i].active = false;
      } else {
        mBirds[i].x += mBirds[i].vx;
        mBirds[i].y += mBirds[i].vy;
        int dx = (int)mBirds[i].x - centerX;
        int dy = (int)mBirds[i].y - centerY;
        float r = sqrt(dx*dx + dy*dy);
        if (r > 91) {
          float angle = atan2(dx, -dy) * 180.0 / M_PI;
          if (angle < 0) angle += 360;
          float h_bird = angle / 15.0;
          if (h_bird >= 7 && h_bird <= 20) {
            float flapOffset = sin((now - mBirds[i].startMs) * mBirds[i].flapSpeed) * 3.0;
            spr.drawLine((int)mBirds[i].x, (int)mBirds[i].y, (int)mBirds[i].x - 4, (int)mBirds[i].y - 2 + (int)flapOffset, spr.color565(150, 150, 150));
            spr.drawLine((int)mBirds[i].x, (int)mBirds[i].y, (int)mBirds[i].x + 4, (int)mBirds[i].y - 2 + (int)flapOffset, spr.color565(150, 150, 150));
          }
        }
      }
    }
  }

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

  // Icono del clima en el centro de las horas de luz (aprox 13:30)
  float sunAngle = 13.5 * 15.0 * M_PI / 180.0;
  int sunX = centerX + 105 * sin(sunAngle);
  int sunY = centerY - 105 * cos(sunAngle);
  
  WeatherData cw24;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    cw24 = currentWeather;
    xSemaphoreGive(dataMutex);
  } else {
    cw24 = currentWeather;
  }
  
  if (!cw24.valid || (cw24.prec == 0.0 && cw24.hr <= 85.0)) {
    spr.fillCircle(sunX, sunY, 8, TFT_YELLOW);
    for(int i=0; i<8; i++) {
       float a = i * 45 * M_PI / 180.0;
       spr.drawLine(sunX + cos(a)*10, sunY + sin(a)*10, sunX + cos(a)*14, sunY + sin(a)*14, TFT_YELLOW);
    }
  } else if (cw24.prec > 0.0) {
    uint16_t cColor = spr.color565(100, 100, 100);
    spr.fillCircle(sunX, sunY-3, 6, cColor);
    spr.fillCircle(sunX - 6, sunY, 5, cColor);
    spr.fillCircle(sunX + 6, sunY, 5, cColor);
    spr.fillRect(sunX - 6, sunY, 12, 5, cColor);
    int rainOffset = (millis() % 1000) / 200;
    uint16_t rColor = spr.color565(0, 150, 255);
    spr.drawLine(sunX - 4, sunY + 6 + rainOffset, sunX - 5, sunY + 8 + rainOffset, rColor);
    spr.drawLine(sunX, sunY + 8 + rainOffset, sunX - 1, sunY + 10 + rainOffset, rColor);
    spr.drawLine(sunX + 4, sunY + 6 + rainOffset, sunX + 3, sunY + 8 + rainOffset, rColor);
  } else {
    uint16_t cColor = spr.color565(180, 180, 180);
    spr.fillCircle(sunX, sunY-3, 6, cColor);
    spr.fillCircle(sunX - 6, sunY, 5, cColor);
    spr.fillCircle(sunX + 6, sunY, 5, cColor);
    spr.fillRect(sunX - 6, sunY, 12, 5, cColor);
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
  spr.drawString("CPU", centerX - 45, centerY - 8);
  String cpuStr24 = String((int)temperatureRead());
  spr.drawString(cpuStr24, centerX - 45, centerY + 8);
  int cpuW24 = spr.textWidth(cpuStr24);
  spr.drawCircle(centerX - 45 + cpuW24 / 2 + 5, centerY + 8 - 6, 2, spr.color565(255, 150, 150));

  spr.pushSprite(0, 0);
}

// ─── Animaciones pantalla lunar ──────────────────────────────────────────────

// Estructura para una estrella fugaz
struct ShootingStar {
  float x, y;       // posición cabeza
  float vx, vy;     // velocidad
  int   len;        // longitud estela (px)
  bool  active;
  unsigned long startMs;
  unsigned long durationMs;
};

// Cohete/transbordador cruzando la pantalla
struct Rocket {
  float x, y;
  float vx, vy;
  bool  active;
  unsigned long startMs;
  unsigned long durationMs;
};

// Satélite orbitando (pequeño punto)
struct Satellite {
  float x, y;
  float vx, vy;
  bool  active;
  unsigned long startMs;
  unsigned long durationMs;
};

static ShootingStar mShoot[3];
static Rocket       mRocket;
static Satellite    mSatellite;
static bool         moonAnimInit = false;

static void initMoonAnimations() {
  for (int i = 0; i < 3; i++) {
    mShoot[i].active = false;
  }
  mRocket.active = false;
  mSatellite.active = false;
  moonAnimInit = true;
}

// Pseudo-random basado en millis() + semilla variable
static uint32_t moonRandSeed = 42;
static uint32_t moonRand() {
  moonRandSeed ^= moonRandSeed << 13;
  moonRandSeed ^= moonRandSeed >> 17;
  moonRandSeed ^= moonRandSeed << 5;
  return moonRandSeed;
}

static void updateMoonAnimations(unsigned long now) {
  // Reactivar estrellas fugaces
  for (int i = 0; i < 3; i++) {
    if (!mShoot[i].active) {
      // Esperar un intervalo aleatorio (3-8 s) antes de lanzar otra
      unsigned long wait = 3000 + (moonRand() % 5000);
      if (now - mShoot[i].startMs > wait) {
        // Ángulo: de izquierda-arriba hacia derecha-abajo, ±30°
        // Siempre en la zona de cielo (evitar luna)
        mShoot[i].x  = (float)(moonRand() % 200) + 10.0f;   // 10..210
        mShoot[i].y  = (float)(moonRand() % 60)  + 5.0f;    // 5..65
        float speed  = 3.0f + (moonRand() % 40) * 0.1f;     // 3..7 px/frame
        mShoot[i].vx = speed;
        mShoot[i].vy = speed * (0.3f + (moonRand() % 7) * 0.1f); // pendiente suave
        mShoot[i].len = 12 + (moonRand() % 16);
        mShoot[i].active    = true;
        mShoot[i].startMs   = now;
        mShoot[i].durationMs = 800 + (moonRand() % 600);
      }
    } else {
      // Comprobar si ha terminado (salió de pantalla o timeout)
      if (mShoot[i].x > 240 || mShoot[i].y > 240 ||
          now - mShoot[i].startMs > mShoot[i].durationMs) {
        mShoot[i].active  = false;
        mShoot[i].startMs = now; // reset timer de espera
      }
    }
  }

  // Cohete/transbordador
  if (!mRocket.active) {
    unsigned long wait = 6000 + (moonRand() % 8000); // 6-14 s entre cohetes
    if (now - mRocket.startMs > wait) {
      // Sentido aleatorio: izq->der o der->izq, siempre por zona de cielo
      bool ltr = (moonRand() % 2) == 0;
      mRocket.y  = 10.0f + (moonRand() % 50);         // fila 10..60
      mRocket.x  = ltr ? -20.0f : 260.0f;
      mRocket.vx = ltr ?  2.8f  : -2.8f;
      mRocket.vy = ltr ? 0.4f : -0.4f;
      mRocket.active    = true;
      mRocket.startMs   = now;
      mRocket.durationMs = 25000;
    }
  } else {
    // Avanzar posición
    mRocket.x += mRocket.vx;
    mRocket.y += mRocket.vy;
    if (mRocket.x > 260 || mRocket.x < -20 ||
        now - mRocket.startMs > mRocket.durationMs) {
      mRocket.active  = false;
      mRocket.startMs = now;
    }
  }

  // Satélite
  if (!mSatellite.active) {
    unsigned long wait = 4000 + (moonRand() % 6000); // 4-10 s entre satélites
    if (now - mSatellite.startMs > wait) {
      bool ltr = (moonRand() % 2) == 0;
      mSatellite.y  = 5.0f + (moonRand() % 70);         // fila 5..75
      mSatellite.x  = ltr ? -10.0f : 250.0f;
      mSatellite.vx = ltr ?  1.0f  : -1.0f;
      mSatellite.vy = (moonRand() % 100) / 100.0f - 0.5f; // Ligera inclinación
      mSatellite.active    = true;
      mSatellite.startMs   = now;
      mSatellite.durationMs = 25000;
    }
  } else {
    // Avanzar posición
    mSatellite.x += mSatellite.vx;
    mSatellite.y += mSatellite.vy;
    if (mSatellite.x > 260 || mSatellite.x < -20 ||
        now - mSatellite.startMs > mSatellite.durationMs) {
      mSatellite.active  = false;
      mSatellite.startMs = now;
    }
  }

  // Avanzar posición de estrellas fugaces activas
  for (int i = 0; i < 3; i++) {
    if (mShoot[i].active) {
      mShoot[i].x += mShoot[i].vx;
      mShoot[i].y += mShoot[i].vy;
    }
  }
}

static void drawShootingStars() {
  for (int i = 0; i < 3; i++) {
    if (!mShoot[i].active) continue;
    float hx = mShoot[i].x;
    float hy = mShoot[i].y;
    float dx = -mShoot[i].vx;
    float dy = -mShoot[i].vy;
    float mag = sqrtf(dx * dx + dy * dy);
    if (mag == 0) continue;
    dx /= mag; dy /= mag;

    int len = mShoot[i].len;
    for (int s = 0; s < len; s++) {
      uint8_t brightness = (uint8_t)(255 * (1.0f - (float)s / len));
      uint16_t col = spr.color565(brightness, brightness, brightness);
      int px = (int)(hx + dx * s);
      int py = (int)(hy + dy * s);
      if (px >= 0 && px < 240 && py >= 0 && py < 240)
        spr.drawPixel(px, py, col);
    }
  }
}

// Dibuja un cohete/transbordador grande (píxeles de 2×2, ~28×12 px total)
static void drawRocket(float x, float y, bool leftToRight) {
  int ix = (int)x;
  int iy = (int)y;
  uint16_t white   = spr.color565(255, 255, 255);
  uint16_t ltgray  = spr.color565(210, 210, 210);
  uint16_t gray    = spr.color565(150, 150, 150);
  uint16_t dkgray  = spr.color565(90,  90,  90);
  uint16_t orange  = spr.color565(255, 150,  0);
  uint16_t yellow  = spr.color565(255, 240,  0);
  uint16_t red     = spr.color565(220,  40, 40);
  uint16_t blue    = spr.color565(80,  170, 255);
  uint16_t lblue   = spr.color565(160, 210, 255);

  // Cada "celda lógica" = bloque de 2x2 px
  auto cell = [&](int cx, int cy, uint16_t c) {
    int rx = leftToRight ? ix - cx * 2 : ix + cx * 2;
    int ry = iy + cy * 2;
    for (int dy = 0; dy < 2; dy++)
      for (int ddx = 0; ddx < 2; ddx++) {
        int fx = leftToRight ? rx - ddx : rx + ddx;
        int fy = ry + dy;
        if (fx >= 0 && fx < 240 && fy >= 0 && fy < 240)
          spr.drawPixel(fx, fy, c);
      }
  };

  // Mapa del cohete (columna × fila, 0-indexed), fila 0 = arriba
  // 14 cols × 6 filas lógicas → 28×12 px reales
  //
  //  Fila 0:          _ _ W W _ _ _ A A _ _ _ _ _
  //  Fila 1:        _ W W W W W W W A A W W _ _ _
  //  Fila 2:      W W W W B B W W W W W W G G F F
  //  Fila 3:      _ W W W W W W W A A W W _ _ _ _
  //  Fila 4:          _ _ W W _ _ _ A A _ _ _ _ _
  //
  // W=white, G=gray, A=ltgray, B=blue, F=flame

  // Nariz (punta delantera)
  cell(0, 2, white);
  cell(1, 1, white); cell(1, 2, white); cell(1, 3, white);
  cell(2, 1, ltgray); cell(2, 2, white); cell(2, 3, ltgray);

  // Cuerpo principal
  for (int c = 3; c <= 9; c++) {
    cell(c, 1, ltgray);
    cell(c, 2, white);
    cell(c, 3, ltgray);
  }

  // Ventanilla (cockpit)
  cell(4, 2, lblue);
  cell(5, 2, blue);
  cell(6, 2, lblue);

  // Aleta superior
  cell(7, 0, gray);
  cell(8, 0, gray);
  cell(9, 0, dkgray);

  // Aleta inferior
  cell(7, 4, gray);
  cell(8, 4, gray);
  cell(9, 4, dkgray);

  // Sección motor / cola
  cell(10, 1, gray); cell(10, 2, gray); cell(10, 3, gray);
  cell(11, 1, dkgray); cell(11, 2, dkgray); cell(11, 3, dkgray);

  // Llama animada con parpadeo
  unsigned long t = millis();
  int phase = (t / 70) % 4;
  switch (phase) {
    case 0:
      cell(12, 2, orange);
      cell(13, 2, yellow);
      cell(14, 2, orange);
      break;
    case 1:
      cell(12, 1, orange); cell(12, 2, red);    cell(12, 3, orange);
      cell(13, 2, orange);
      cell(14, 2, yellow);
      break;
    case 2:
      cell(12, 2, yellow);
      cell(13, 1, orange); cell(13, 2, orange); cell(13, 3, orange);
      cell(14, 2, red);
      break;
    case 3:
      cell(12, 1, orange); cell(12, 2, orange); cell(12, 3, orange);
      cell(13, 2, red);
      cell(14, 2, orange);
      break;
  }
}

// ─────────────────────────────────────────────────────────────────────────────

void drawMoonUI(struct tm* timeinfo) {
  if (!moonAnimInit) initMoonAnimations();

  unsigned long now = millis();
  updateMoonAnimations(now);

  spr.fillSprite(TFT_BLACK);
  
  // Dibujar estrellas de fondo animadas (parpadeo)
  static const int stars[][2] = {
    {30, 40}, {70, 25}, {200, 30}, {180, 80}, {40, 150},
    {210, 160}, {60, 200}, {190, 210}, {20, 100}, {220, 110},
    {110, 20}, {140, 220}, {230, 50}, {10, 180}, {90, 215}
  };
  
  for (int i = 0; i < 15; i++) {
    int sx = stars[i][0];
    int sy = stars[i][1];
    float t = now / (400.0 + i * 70.0) + i; 
    uint8_t b = 50 + 205 * (sin(t) + 1.0) / 2.0; 
    
    spr.drawPixel(sx, sy, spr.color565(b, b, b));
    if (i % 3 == 0) {
      uint8_t halfB = b / 2;
      uint16_t haloColor = spr.color565(halfB, halfB, halfB);
      spr.drawPixel(sx+1, sy, haloColor);
      spr.drawPixel(sx-1, sy, haloColor);
      spr.drawPixel(sx, sy+1, haloColor);
      spr.drawPixel(sx, sy-1, haloColor);
    }
  }

  // Estrellas fugaces
  drawShootingStars();

  // Cohete/transbordador
  if (mRocket.active) {
    drawRocket(mRocket.x, mRocket.y, mRocket.vx > 0);
  }

  // Satélite
  if (mSatellite.active) {
    uint8_t pulse = 150 + 105 * (sin(now / 200.0) + 1.0) / 2.0;
    uint16_t satCol = spr.color565(pulse, pulse, 255);
    spr.drawPixel((int)mSatellite.x, (int)mSatellite.y, satCol);
    spr.drawPixel((int)mSatellite.x + 1, (int)mSatellite.y, satCol);
    spr.drawPixel((int)mSatellite.x, (int)mSatellite.y + 1, satCol);
    spr.drawPixel((int)mSatellite.x + 1, (int)mSatellite.y + 1, satCol);
  }

  // ── Luna ──────────────────────────────────────────────────────────────────
  int phase = getMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  
  String phaseName = "";
  int mX = centerX;
  int mY = centerY - 15;
  int mR = 60;
  uint16_t moonColor = spr.color565(240, 240, 200); 
  uint16_t shadowColor = TFT_BLACK;
  
  spr.fillCircle(mX, mY, mR, moonColor);
  
  auto blurShadowCircle = [&](int cx, int cy, int r, int dir) {
    for (int i = 8; i >= 0; i--) {
      spr.fillCircle(cx + i * dir, cy, r, spr.color565(240 * i / 9, 240 * i / 9, 200 * i / 9));
    }
  };
  auto blurShadowRect = [&](int rx, int ry, int rw, int rh, int dir) {
    for (int i = 8; i >= 0; i--) {
      spr.fillRect(rx + i * dir, ry, rw, rh, spr.color565(240 * i / 9, 240 * i / 9, 200 * i / 9));
    }
  };

  switch(phase) {
    case 0: 
      phaseName = tr("Luna Nueva", "New Moon");
      spr.fillCircle(mX, mY, mR, shadowColor);
      break;
    case 1: 
      phaseName = tr("Creciente Concava", "Waxing Crescent");
      blurShadowCircle(mX - 25, mY, mR, 1);
      break;
    case 2: 
      phaseName = tr("Cuarto Creciente", "First Quarter");
      blurShadowRect(mX - mR, mY - mR, mR, mR * 2, 1);
      break;
    case 3: 
      phaseName = tr("Creciente Convexa", "Waxing Gibbous");
      blurShadowCircle(mX - 50, mY, mR, 1); 
      break;
    case 4: 
      phaseName = tr("Luna Llena", "Full Moon");
      break;
    case 5: 
      phaseName = tr("Menguante Convexa", "Waning Gibbous");
      blurShadowCircle(mX + 50, mY, mR, -1);
      break;
    case 6: 
      phaseName = tr("Cuarto Menguante", "Last Quarter");
      blurShadowRect(mX, mY - mR, mR, mR * 2, -1);
      break;
    case 7: 
      phaseName = tr("Menguante Concava", "Waning Crescent");
      blurShadowCircle(mX + 25, mY, mR, -1);
      break;
  }

  // Borde para que se vea la luna nueva
  spr.drawCircle(mX, mY, mR, spr.color565(100, 100, 100));

  float fraction = getMoonPhaseFraction(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  int percentage = (int)round((1.0 - cos(fraction * 2.0 * M_PI)) / 2.0 * 100.0);

  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(4); // Fuente un poco mas grande
  spr.setTextSize(1);
  spr.setTextColor(TFT_CYAN); // Sin color de fondo para que sea transparente
  spr.drawString(String(percentage) + "%", mX, mY);

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
  // 4 = storm day no rain, 5 = storm night no rain
  // 6 = storm day rain, 7 = storm night rain
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
  } else if (type >= 2) {
    int cx = x + 8 * sin((millis() % 4000) / 4000.0 * 2.0 * M_PI); // Desplazamiento lateral, ciclo de 4s
    uint16_t cColor = (type == 2) ? spr.color565(180, 180, 180) : spr.color565(100, 100, 100);
    int yOffset = (type == 2) ? 4 : 0;

    // Draw background (sun or moon) for storm
    if (type == 4 || type == 6) {
      spr.fillCircle(cx + 12, y - 10 + yOffset, 8, TFT_YELLOW);
    } else if (type == 5 || type == 7) {
      spr.fillCircle(cx + 12, y - 10 + yOffset, 8, spr.color565(220, 220, 220));
      spr.fillCircle(cx + 16, y - 12 + yOffset, 6, TFT_BLACK);
    }

    auto drawSingleCloud = [&](float cloudX, float cloudY, uint16_t color, float scale) {
      if (scale <= 0.05f) return;
      int r1 = (int)(12.0f * scale);
      int r2 = (int)(10.0f * scale);
      int w = (int)(24.0f * scale);
      int h = (int)(10.0f * scale);
      int xOff = (int)(12.0f * scale);
      int yOff = (int)(5.0f * scale);
      spr.fillCircle((int)cloudX, (int)cloudY - yOff, r1, color);
      spr.fillCircle((int)cloudX - xOff, (int)cloudY, r2, color);
      spr.fillCircle((int)cloudX + xOff, (int)cloudY, r2, color);
      spr.fillRect((int)cloudX - xOff, (int)cloudY, w + 1, h + 1, color);
    };

    uint16_t extraCol1 = (type == 2) ? spr.color565(140, 140, 140) : spr.color565(70, 70, 70);
    uint16_t extraCol2 = (type == 2) ? spr.color565(200, 200, 200) : spr.color565(130, 130, 130);

    // Nube extra trasera (aparece y desaparece)
    float phase1 = (millis() % 3500) / 3500.0f;
    float scale1 = sin(phase1 * M_PI) * 0.6f;
    drawSingleCloud(x - 20 + (phase1 * 40.0f), y + yOffset - 6, extraCol1, scale1);

    // Nube principal
    drawSingleCloud(cx, y + yOffset, cColor, 1.0f);

    // Nube extra delantera (aparece y desaparece)
    float phase2 = (millis() % 4500) / 4500.0f;
    float scale2 = sin(phase2 * M_PI) * 0.5f;
    drawSingleCloud(x + 20 - (phase2 * 40.0f), y + yOffset + 4, extraCol2, scale2);

    // Draw rain
    if (type == 3 || type == 6 || type == 7) {
      int rainOffset = (millis() % 1000) / 100;
      uint16_t rColor = spr.color565(0, 150, 255);
      spr.drawLine(cx - 10, y + 10 + rainOffset, cx - 12, y + 14 + rainOffset, rColor);
      spr.drawLine(cx, y + 10 + rainOffset, cx - 2, y + 14 + rainOffset, rColor);
      spr.drawLine(cx + 10, y + 10 + rainOffset, cx + 8, y + 14 + rainOffset, rColor);
    }

    // Draw lightning
    if (type >= 4 && type <= 7) {
      if ((millis() % 1500) < 300) {
        uint16_t bColor = TFT_YELLOW;
        spr.fillTriangle(cx - 2, y + 8, cx + 4, y + 8, cx, y + 16, bColor);
        spr.fillTriangle(cx, y + 14, cx + 6, y + 14, cx - 4, y + 24, bColor);
      }
    }
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
    spr.drawString(tr("Sin datos del tiempo", "No weather data"), centerX, centerY);

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
  if (cw.weather_code == 95 || cw.weather_code == 96 || cw.weather_code == 99) {
    bool hasRain = (cw.prec > 0.0 || cw.weather_code == 96 || cw.weather_code == 99);
    if (cw.is_day) {
      iconType = hasRain ? 6 : 4; // Storm day (rain/no-rain)
    } else {
      iconType = hasRain ? 7 : 5; // Storm night (rain/no-rain)
    }
  } else if (cw.weather_code >= 51 && cw.weather_code <= 86) {
    iconType = 3; // Rain / Snow
  } else if (cw.weather_code >= 1 && cw.weather_code <= 48) {
    iconType = 2; // Cloud / Fog
  } else {
    iconType = cw.is_day ? 0 : 1; // Sun or Moon
  }
  
  drawWeatherIcon(centerX, centerY - 40, iconType);
  
  uint16_t tempColor;
  if (cw.ta <= 5.0) tempColor = TFT_WHITE;
  else if (cw.ta <= 15.0) tempColor = spr.color565(100, 200, 255); // Azul claro
  else if (cw.ta <= 25.0) tempColor = TFT_YELLOW;
  else if (cw.ta <= 35.0) tempColor = TFT_ORANGE;
  else tempColor = TFT_RED;

  spr.setTextFont(4); // 26px font
  spr.setTextSize(2);
  spr.setTextColor(tempColor, TFT_BLACK);
  String taStr = String(cw.ta, 1);
  spr.drawString(taStr, centerX, centerY + 25);
  int taWidth = spr.textWidth(taStr);
  spr.drawCircle(centerX + taWidth / 2 + 8, centerY + 25 - 18, 4, tempColor);
  spr.drawCircle(centerX + taWidth / 2 + 8, centerY + 25 - 18, 3, tempColor);
  
  // Mostrar max y min a los lados (centradas verticalmente en el centro de la pantalla redonda)
  spr.setTextFont(2);
  spr.setTextSize(1);
  
  String taminStr = String(cw.tamin, 1);
  spr.setTextColor(TFT_CYAN, TFT_BLACK);
  spr.drawString(taminStr, centerX - 95, centerY + 40);
  int taminWidth = spr.textWidth(taminStr);
  spr.drawCircle(centerX - 95 + taminWidth / 2 + 5, centerY + 40 - 6, 2, TFT_CYAN);
  
  String tamaxStr = String(cw.tamax, 1);
  spr.setTextColor(TFT_ORANGE, TFT_BLACK);
  spr.drawString(tamaxStr, centerX + 95, centerY + 10);
  int tamaxWidth = spr.textWidth(tamaxStr);
  spr.drawCircle(centerX + 95 + tamaxWidth / 2 + 5, centerY + 10 - 6, 2, TFT_ORANGE);
  
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  String extra = tr("HR: ", "RH: ") + String(cw.hr, 0) + "%";
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
  spr.drawString(tr("N", "N"), aX, aY - 14);
  spr.drawString(tr("S", "S"), aX, aY + 14);
  spr.drawString(tr("E", "E"), aX + 14, aY);
  spr.drawString(tr("O", "W"), aX - 14, aY);

  if (cw.prec > 0) {
    spr.setTextColor(spr.color565(0, 200, 255), TFT_BLACK);
    spr.drawString(tr("Prec: ", "Rain: ") + String(cw.prec, 1) + "mm", centerX, centerY + 100);
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

void drawArtificialHorizon() {
  spr.fillSprite(TFT_BLACK);
  std::vector<Airplane> localPlanes;
  if (dataMutex != NULL) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    localPlanes = planes;
    xSemaphoreGive(dataMutex);
  } else {
    localPlanes = planes;
  }

  float minDist = 999999;
  Airplane* target = nullptr;
  for (int i=0; i<localPlanes.size(); i++) {
    if (localPlanes[i].distanceKm < minDist) {
      minDist = localPlanes[i].distanceKm;
      target = &localPlanes[i];
    }
  }

  // Fake pitch and roll animation to make it look alive
  float roll = sin(millis() / 1500.0) * 15.0 * M_PI / 180.0; // +/- 15 degrees
  int pitchOffset = sin(millis() / 800.0) * 15; // +/- 15 pixels

  spr.fillSprite(spr.color565(50, 150, 255)); // Sky background
  
  // Calculate ground polygon based on roll and pitch
  int yL = centerY + pitchOffset + sin(roll) * 120;
  int yR = centerY + pitchOffset - sin(roll) * 120;
  
  // Ground
  spr.fillTriangle(0, yL, 240, yR, 0, 240, spr.color565(139, 69, 19));
  spr.fillTriangle(240, yR, 0, 240, 240, 240, spr.color565(139, 69, 19));
  // Horizon Line
  spr.drawLine(0, yL, 240, yR, TFT_WHITE);
  
  // Center Crosshair
  spr.drawLine(centerX - 40, centerY, centerX - 10, centerY, TFT_GREEN);
  spr.drawLine(centerX + 10, centerY, centerX + 40, centerY, TFT_GREEN);
  spr.drawLine(centerX - 40, centerY, centerX - 40, centerY + 10, TFT_GREEN);
  spr.drawLine(centerX + 40, centerY, centerX + 40, centerY + 10, TFT_GREEN);
  spr.fillCircle(centerX, centerY, 2, TFT_GREEN);

  spr.setTextDatum(MC_DATUM);

  if (target != nullptr) {
    String altStr = "";
    String altLabel = "";
    if (pref_units == "ft") {
      altStr = String((int)(target->altitude * 3.28084));
      altLabel = "ALT (FT)";
    } else {
      altStr = String((int)target->altitude);
      altLabel = "ALT (M)";
    }
    
    // Altitude
    spr.setTextSize(1);
    spr.setTextColor(TFT_WHITE, spr.color565(50, 150, 255));
    spr.drawString(altLabel, 40, centerY - 30);
    spr.setTextSize(2);
    spr.setTextColor(TFT_GREEN, spr.color565(50, 150, 255));
    spr.drawString(altStr, 40, centerY - 15);
    
    // Speed
    String spdStr = String((int)target->velocity);
    spr.setTextSize(1);
    spr.setTextColor(TFT_WHITE, spr.color565(50, 150, 255));
    spr.drawString("SPD (KM/H)", 200, centerY - 30);
    spr.setTextSize(2);
    spr.setTextColor(TFT_GREEN, spr.color565(50, 150, 255));
    spr.drawString(spdStr, 200, centerY - 15);
    
    // Heading
    spr.setTextSize(1);
    spr.setTextColor(TFT_WHITE, spr.color565(50, 150, 255));
    spr.drawString("HDG", centerX, 10);
    spr.setTextSize(2);
    spr.setTextColor(TFT_GREEN, spr.color565(50, 150, 255));
    spr.drawString(String((int)target->heading), centerX, 25);
    
    // Callsign
    spr.setTextSize(2);
    spr.setTextColor(TFT_GREEN, spr.color565(139, 69, 19));
    spr.drawString(target->callsign, centerX, 200);
  } else {
    spr.setTextSize(2);
    spr.setTextColor(TFT_GREEN, spr.color565(139, 69, 19));
    spr.drawString("NO TARGET", centerX, 200);
  }
  
  spr.pushSprite(0, 0);
}


void drawISS() {
  spr.fillSprite(TFT_BLACK);

  // ── Título ────────────────────────────────────────────────────────────────
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(spr.color565(0, 200, 255));
  spr.setTextSize(2);
  spr.drawString("ISS TRACKER", centerX, 26);

  // ── Mapa mundial centrado ─────────────────────────────────────────────────
  int mapX = centerX - (WORLD_MAP_WIDTH / 2);
  int mapY = centerY - (WORLD_MAP_HEIGHT / 2);

  spr.drawBitmap(mapX, mapY, world_map_bitmap, WORLD_MAP_WIDTH, WORLD_MAP_HEIGHT, spr.color565(0, 100, 0));

  // ── Órbita teórica de la ISS ──────────────────────────────────────────────
  if (iss_lat != 0.0) {
    // La inclinación de la ISS es aprox 51.6 grados
    float val = asin(constrain(iss_lat / 51.6, -1.0, 1.0));
    float phase;
    // Determinar si es nodo ascendente o descendente comparando con la última lectura
    if (iss_lat >= iss_last_lat) {
      phase = (iss_lon * M_PI / 180.0) - val;
    } else {
      phase = (iss_lon * M_PI / 180.0) - (M_PI - val);
    }

    uint16_t orbitColor = spr.color565(80, 80, 80);
    for (int x = 0; x < WORLD_MAP_WIDTH; x++) {
      if (x % 3 == 0) { // Línea punteada
        float l_lon = (x / (float)WORLD_MAP_WIDTH) * 360.0 - 180.0;
        float l_lat = 51.6 * sin((l_lon * M_PI / 180.0) - phase);
        int px = mapX + x;
        int py = mapY + ((90.0 - l_lat) / 180.0) * WORLD_MAP_HEIGHT;
        spr.fillCircle(px, py, 1, orbitColor);
      }
    }
  }

  // Posición del usuario (punto verde)
  int hX = (int)(mapX + ((pref_lon + 180.0) / 360.0) * WORLD_MAP_WIDTH);
  int hY = (int)(mapY + ((90.0 - pref_lat) / 180.0) * WORLD_MAP_HEIGHT);
  spr.fillCircle(hX, hY, 3, TFT_GREEN);

  // Posición de la ISS en el mapa
  int iX = (int)(mapX + ((iss_lon + 180.0) / 360.0) * WORLD_MAP_WIDTH);
  int iY = (int)(mapY + ((90.0 - iss_lat) / 180.0) * WORLD_MAP_HEIGHT);

  // ── Silueta ISS en su posición del mapa ──────────────────────────────────
  // Tamaño: truss de 24 px, paneles de 5×4 px. Centro = (iX, iY)
  uint16_t silver = spr.color565(210, 210, 220);
  uint16_t gold   = spr.color565(220, 185,  30);
  uint16_t dkgold = spr.color565(130, 100,  10);
  uint16_t lgray  = spr.color565(180, 180, 190);
  uint16_t cyan   = spr.color565(0,   210, 255);

  // Halo pulsante animado
  unsigned long now_ms = millis();
  uint8_t glow = (uint8_t)(40 + 40 * sin(now_ms / 600.0));
  uint16_t glowCol = spr.color565(0, glow, glow + 40);
  spr.drawCircle(iX, iY, 10, glowCol);

  // Truss horizontal
  spr.drawLine(iX - 12, iY, iX + 12, iY, silver);
  spr.drawLine(iX - 12, iY + 1, iX + 12, iY + 1, silver);

  // Módulo central (body)
  spr.fillRect(iX - 5, iY - 3, 10, 7, lgray);
  spr.drawRect(iX - 5, iY - 3, 10, 7, silver);
  // Ventanilla
  spr.fillRect(iX - 2, iY - 1, 4, 3, cyan);

  // Panel solar izquierdo superior
  spr.fillRect(iX - 12, iY - 5, 6, 4, gold);
  spr.drawRect(iX - 12, iY - 5, 6, 4, dkgold);
  // Panel solar izquierdo inferior
  spr.fillRect(iX - 12, iY + 2, 6, 4, gold);
  spr.drawRect(iX - 12, iY + 2, 6, 4, dkgold);

  // Panel solar derecho superior
  spr.fillRect(iX + 6,  iY - 5, 6, 4, gold);
  spr.drawRect(iX + 6,  iY - 5, 6, 4, dkgold);
  // Panel solar derecho inferior
  spr.fillRect(iX + 6,  iY + 2, 6, 4, gold);
  spr.drawRect(iX + 6,  iY + 2, 6, 4, dkgold);

  // ── Coordenadas y distancia ───────────────────────────────────────────────
  float dy_d = iss_lat - pref_lat;
  float dx_d = iss_lon - pref_lon;
  float dist  = sqrt(dx_d * dx_d + dy_d * dy_d) * 111.0;

  spr.setTextSize(1);
  spr.setTextColor(spr.color565(160, 160, 160));
  char coordBuf[32];
  snprintf(coordBuf, sizeof(coordBuf), "%.1f%c %.1f%c  %.0fkm",
           fabs(iss_lat), iss_lat >= 0 ? 'N' : 'S',
           fabs(iss_lon), iss_lon >= 0 ? 'E' : 'W',
           dist);
  spr.drawString(coordBuf, centerX, 197);

  // ── Próximo paso visible ───────────────────────────────────────────────────
  if (pref_n2yo_key == "") {
    spr.setTextColor(spr.color565(100, 100, 100));
    spr.drawString(tr("Configura n2yo key", "Set n2yo key"), centerX, 210);
    spr.drawString(tr("para ver proximos pasos", "to see next passes"), centerX, 221);
  } else if (iss_next_pass_time == 0) {
    spr.setTextColor(spr.color565(100, 150, 100));
    spr.drawString(tr("Buscando paso...", "Looking for pass..."), centerX, 212);
  } else if (iss_next_pass_time == -1) {
    spr.setTextColor(spr.color565(130, 130, 130));
    spr.drawString(tr("No visible en 2 dias", "Not visible in 2 days"), centerX, 212);
  } else {
    // Calcular segundos hasta el paso (convertir timestamp UTC a local)
    struct tm now_tm;
    long secsLeft = 0;
    if (getLocalTime(&now_tm, 10)) {
      time_t local_unix = mktime(&now_tm);
      time_t now_utc    = local_unix - pref_offset - (pref_dst ? 3600 : 0);
      secsLeft = (long)iss_next_pass_time - (long)now_utc;
    }

    if (secsLeft <= 0) {
      // Ya pasó o está pasando ahora
      spr.setTextColor(spr.color565(0, 255, 100));
      spr.drawString(tr(">> VISIBLE AHORA! <<", ">> VISIBLE NOW! <<"), centerX, 208);
      char elBuf[24];
      snprintf(elBuf, sizeof(elBuf), tr("Elev max %d", "Max elev %d").c_str(),
               iss_next_pass_max_el);
      spr.setTextColor(TFT_WHITE);
      spr.drawString(elBuf, centerX, 220);
    } else {
      // Formatear tiempo restante
      char timeBuf[32];
      long h = secsLeft / 3600;
      long m = (secsLeft % 3600) / 60;
      if (h > 0)
        snprintf(timeBuf, sizeof(timeBuf), tr("Prox paso: %ldh %ldm", "Next pass: %ldh %ldm").c_str(), h, m);
      else
        snprintf(timeBuf, sizeof(timeBuf), tr("Prox paso: %ldmin", "Next pass: %ldmin").c_str(), m);

      spr.setTextColor(spr.color565(80, 200, 255));
      spr.drawString(timeBuf, centerX, 208);

      char elBuf[32];
      snprintf(elBuf, sizeof(elBuf), tr("Elev %d  Dur %ds", "Elev %d  Dur %ds").c_str(),
               iss_next_pass_max_el, iss_next_pass_duration);
      spr.setTextColor(spr.color565(180, 180, 180));
      spr.drawString(elBuf, centerX, 220);
    }
  }

  spr.pushSprite(0, 0);

}



void drawSunArc(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  spr.setTextFont(1);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_ORANGE);
  spr.setTextSize(2);
  spr.drawString(tr("SOL Y LUNA", "SUN & MOON"), centerX, 35);
  
  int r = 80;
  // Draw Day Arc (top half)
  for(int a=180; a<=360; a+=2) {
    float rad = a * M_PI / 180.0;
    spr.drawPixel(centerX + cos(rad)*r, centerY + 15 + sin(rad)*r, TFT_DARKGREY);
  }
  // Draw Night Arc (bottom half)
  for(int a=0; a<180; a+=2) {
    float rad = a * M_PI / 180.0;
    spr.drawPixel(centerX + cos(rad)*r, centerY + 15 + sin(rad)*r, spr.color565(0, 0, 100));
  }
  spr.drawLine(centerX - 90, centerY + 15, centerX + 90, centerY + 15, spr.color565(100,50,0));
  
  
  if (sunriseTimeStr != "--:--" && sunsetTimeStr != "--:--") {
    int sr_h = sunriseTimeStr.substring(0, 2).toInt();
    int sr_m = sunriseTimeStr.substring(3, 5).toInt();
    int ss_h = sunsetTimeStr.substring(0, 2).toInt();
    int ss_m = sunsetTimeStr.substring(3, 5).toInt();
    int sr_mins = sr_h * 60 + sr_m;
    int ss_mins = ss_h * 60 + ss_m;
    int now_mins = timeinfo->tm_hour * 60 + timeinfo->tm_min;
    
    if (now_mins >= sr_mins && now_mins <= ss_mins) {
      sun_progress = ((float)(now_mins - sr_mins) / (float)(ss_mins - sr_mins)) * 0.5;
    } else {
      int night_elapsed = (now_mins > ss_mins) ? (now_mins - ss_mins) : ((1440 - ss_mins) + now_mins);
      int night_total = (1440 - ss_mins) + sr_mins;
      sun_progress = 0.5 + ((float)night_elapsed / (float)night_total) * 0.5;
    }
  }

  spr.setTextColor(TFT_YELLOW);
  spr.setTextSize(1);
  if (sun_progress >= 0.5 && sunriseTimeStr != "--:--") {
    int sr_h = sunriseTimeStr.substring(0, 2).toInt();
    int sr_m = sunriseTimeStr.substring(3, 5).toInt();
    int sr_mins = sr_h * 60 + sr_m;
    int now_mins = timeinfo->tm_hour * 60 + timeinfo->tm_min;
    int remaining_mins = (sr_mins >= now_mins) ? (sr_mins - now_mins) : (1440 - now_mins + sr_mins);
    char buf[30];
    sprintf(buf, "Faltan: %dh %dm", remaining_mins / 60, remaining_mins % 60);
    spr.drawString(String(buf), centerX, centerY - 15);
  } else {
    spr.drawString("Zenit: " + solarNoonTimeStr, centerX, centerY - 15);
  }
  
  // Calculate Sun position (sun_progress goes from 0.0 to 1.0)
  float sun_angle = 180.0 + (sun_progress * 360.0);
  if (sun_angle > 360.0) sun_angle -= 360.0;
  
  float sun_rad = sun_angle * M_PI / 180.0;
  int sx = centerX + cos(sun_rad)*r;
  int sy = centerY + 15 + sin(sun_rad)*r;
  
  // Calculate Moon position based on Sun and Phase
  float moon_fraction = getMoonPhaseFraction(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  float moon_angle = sun_angle - (moon_fraction * 360.0);
  if (moon_angle < 0.0) moon_angle += 360.0;
  
  float moon_rad = moon_angle * M_PI / 180.0;
  int mx = centerX + cos(moon_rad)*r;
  int my = centerY + 15 + sin(moon_rad)*r;
  
  // Draw Moon
  int phase = getMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  spr.fillCircle(mx, my, 7, TFT_LIGHTGREY);
  
  uint16_t shadowColor = TFT_BLACK;
  switch(phase) {
    case 0: spr.fillCircle(mx, my, 7, shadowColor); break;
    case 1: spr.fillCircle(mx - 3, my, 7, shadowColor); break;
    case 2: spr.fillRect(mx - 7, my - 7, 7, 14, shadowColor); break;
    case 3: spr.fillCircle(mx - 6, my, 7, shadowColor); break;
    case 4: break; // Full
    case 5: spr.fillCircle(mx + 6, my, 7, shadowColor); break;
    case 6: spr.fillRect(mx, my - 7, 7, 14, shadowColor); break;
    case 7: spr.fillCircle(mx + 3, my, 7, shadowColor); break;
  }
  
  spr.drawCircle(mx, my, 7, TFT_WHITE);
  
  // Draw Sun with intensity based on height
  float elevation = -sin(sun_rad);
  
  uint16_t nightFill = spr.color565(40, 40, 40);
  uint16_t nightStroke = spr.color565(70, 70, 70);
  
  float pos_elev = elevation > 0 ? elevation : 0;
  uint8_t day_r = 255;
  uint8_t day_g = 64 + (uint8_t)(191.0 * pos_elev);
  uint8_t day_b = (uint8_t)(200.0 * pos_elev);
  uint16_t dayFill = spr.color565(day_r, day_g, day_b);
  uint16_t dayStroke = spr.color565(255, 128 + (uint8_t)(127.0 * pos_elev), 0);
  
  // Draw Night Sun (full circle)
  spr.fillCircle(sx, sy, 8, nightFill);
  spr.drawCircle(sx, sy, 10, nightStroke);
  
  // Overwrite with Day Sun for the part above the horizon
  int horizonY = centerY + 15;
  for (int dy = -8; dy <= 8; dy++) {
    if (sy + dy <= horizonY) {
      int dx = round(sqrt(64 - dy * dy));
      spr.drawFastHLine(sx - dx, sy + dy, dx * 2 + 1, dayFill);
    }
  }
  for (int a = 0; a < 360; a++) {
    float rad = a * M_PI / 180.0;
    int px = sx + round(10 * cos(rad));
    int py = sy + round(10 * sin(rad));
    if (py <= horizonY) {
      spr.drawPixel(px, py, dayStroke);
    }
  }
  
  // Draw times at the very end so they are always on top
  spr.setTextSize(1);
  spr.setTextColor(TFT_WHITE);
  spr.drawString(sunriseTimeStr, centerX - 80, centerY + 25);
  spr.drawString(sunsetTimeStr, centerX + 80, centerY + 25);

  // Daylight duration calculation
  if (sunriseTimeStr != "--:--" && sunsetTimeStr != "--:--") {
    int sr_h = sunriseTimeStr.substring(0, 2).toInt();
    int sr_m = sunriseTimeStr.substring(3, 5).toInt();
    int ss_h = sunsetTimeStr.substring(0, 2).toInt();
    int ss_m = sunsetTimeStr.substring(3, 5).toInt();
    
    int total_sr = sr_h * 60 + sr_m;
    int total_ss = ss_h * 60 + ss_m;
    String luzStr = "";
    
    if (sun_progress >= 0.5) {
      int night_mins = (1440 - total_ss) + total_sr;
      luzStr = tr("Noche: ", "Night: ") + String(night_mins / 60) + "h " + String(night_mins % 60) + "m";
    } else {
      int diff = total_ss - total_sr;
      if (diff < 0) diff += 24 * 60;
      luzStr = tr("Luz solar: ", "Daylight: ") + String(diff / 60) + "h " + String(diff % 60) + "m";
    }
    
    spr.setTextColor(spr.color565(150, 200, 255));
    spr.drawString(luzStr, centerX, centerY + 65);
  }

  spr.pushSprite(0, 0);
}

// ─── ZODIAC SCREEN ────────────────────────────────────────────────────────
struct ZodiacSign {
  const char* name_es;
  const char* name_en;
  const char* symbol;
  const char* dateRange_es;
  const char* dateRange_en;
  int starCount;
  int8_t stars[15][2]; // x, y (relative to center)
  int lineCount;
  int8_t lines[15][2]; // idx1, idx2
};

const ZodiacSign zodiacs[12] = {
  // 0: Capricornio
  {"Capricornio", "Capricorn", "♑", "22 Dic - 19 Ene", "Dec 22 - Jan 19", 9, 
    {{-30,-20}, {-10,-30}, {10,-20}, {20,0}, {10,20}, {-10,15}, {-25,5}, {25,-25}, {15,30}}, 
    8, {{0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,6}, {2,7}, {4,8}}},
  // 1: Acuario
  {"Acuario", "Aquarius", "♒", "20 Ene - 18 Feb", "Jan 20 - Feb 18", 10,
    {{-40,-10}, {-20,-20}, {0,-10}, {20,-20}, {40,-10}, {-40,10}, {-20,0}, {0,10}, {20,0}, {40,10}},
    8, {{0,1}, {1,2}, {2,3}, {3,4}, {5,6}, {6,7}, {7,8}, {8,9}}},
  // 2: Piscis
  {"Piscis", "Pisces", "♓", "19 Feb - 20 Mar", "Feb 19 - Mar 20", 11,
    {{-30,20}, {-40,0}, {-30,-20}, {-20,0}, {0,5}, {20,10}, {30,25}, {40,15}, {30,5}, {30,-5}, {15,-15}},
    10, {{0,1}, {1,2}, {2,3}, {3,0}, {3,4}, {4,5}, {5,6}, {6,7}, {7,8}, {8,5}}},
  // 3: Aries
  {"Aries", "Aries", "♈", "21 Mar - 19 Abr", "Mar 21 - Apr 19", 4,
    {{-30,20}, {-10,-10}, {10,0}, {30,-15}},
    3, {{0,1}, {1,2}, {2,3}}},
  // 4: Tauro
  {"Tauro", "Taurus", "♉", "20 Abr - 20 May", "Apr 20 - May 20", 8,
    {{-30,-30}, {-10,0}, {10,-10}, {30,-30}, {0,10}, {5,25}, {10,40}, {-5,15}},
    7, {{0,1}, {1,2}, {2,3}, {1,4}, {2,4}, {4,7}, {7,5}}},
  // 5: Géminis
  {"Géminis", "Gemini", "♊", "21 May - 20 Jun", "May 21 - Jun 20", 9,
    {{-20,-30}, {-10,-10}, {-20,10}, {-30,30}, {20,-25}, {10,-5}, {20,15}, {30,35}, {0,-15}},
    8, {{0,1}, {1,2}, {2,3}, {4,5}, {5,6}, {6,7}, {1,5}, {2,6}}},
  // 6: Cáncer
  {"Cáncer", "Cancer", "♋", "21 Jun - 22 Jul", "Jun 21 - Jul 22", 5,
    {{-30,-20}, {-10,0}, {10,-10}, {30,10}, {10,30}},
    4, {{0,1}, {1,2}, {2,3}, {1,4}}},
  // 7: Leo
  {"Leo", "Leo", "♌", "23 Jul - 22 Ago", "Jul 23 - Aug 22", 9,
    {{-30,-20}, {-20,-40}, {0,-35}, {10,-15}, {-10,0}, {20,10}, {40,0}, {30,20}, {10,25}},
    8, {{0,1}, {1,2}, {2,3}, {3,4}, {3,5}, {5,6}, {6,7}, {7,8}}},
  // 8: Virgo
  {"Virgo", "Virgo", "♍", "23 Ago - 22 Sep", "Aug 23 - Sep 22", 10,
    {{-40,0}, {-20,10}, {-10,-10}, {10,-20}, {20,0}, {30,-15}, {40,10}, {30,30}, {10,20}, {0,35}},
    9, {{0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,6}, {6,7}, {7,8}, {8,9}}},
  // 9: Libra
  {"Libra", "Libra", "♎", "23 Sep - 22 Oct", "Sep 23 - Oct 22", 6,
    {{-30,10}, {-15,-10}, {15,-15}, {30,0}, {0,15}, {-10,25}},
    5, {{0,1}, {1,2}, {2,3}, {1,4}, {4,5}}},
  // 10: Escorpio
  {"Escorpio", "Scorpio", "♏", "23 Oct - 21 Nov", "Oct 23 - Nov 21", 12,
    {{-30,-10}, {-20,-20}, {0,-25}, {20,-15}, {10,5}, {0,20}, {-10,35}, {10,40}, {25,30}, {35,15}, {45,25}, {50,10}},
    11, {{0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,6}, {6,7}, {7,8}, {8,9}, {9,10}, {10,11}}},
  // 11: Sagitario
  {"Sagitario", "Sagittarius", "♐", "22 Nov - 21 Dic", "Nov 22 - Dec 21", 8,
    {{-30,20}, {-10,0}, {10,-20}, {30,-40}, {0,-10}, {-20,-20}, {20,-10}, {40,0}},
    7, {{0,1}, {1,2}, {2,3}, {1,4}, {4,5}, {2,6}, {6,7}}}
};

int getZodiacIndex(int month, int day) {
  if ((month == 12 && day >= 22) || (month == 1 && day <= 19)) return 0;
  if ((month == 1 && day >= 20) || (month == 2 && day <= 18)) return 1;
  if ((month == 2 && day >= 19) || (month == 3 && day <= 20)) return 2;
  if ((month == 3 && day >= 21) || (month == 4 && day <= 19)) return 3;
  if ((month == 4 && day >= 20) || (month == 5 && day <= 20)) return 4;
  if ((month == 5 && day >= 21) || (month == 6 && day <= 20)) return 5;
  if ((month == 6 && day >= 21) || (month == 7 && day <= 22)) return 6;
  if ((month == 7 && day >= 23) || (month == 8 && day <= 22)) return 7;
  if ((month == 8 && day >= 23) || (month == 9 && day <= 22)) return 8;
  if ((month == 9 && day >= 23) || (month == 10 && day <= 22)) return 9;
  if ((month == 10 && day >= 23) || (month == 11 && day <= 21)) return 10;
  return 11;
}

void drawZodiacUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  int month = timeinfo->tm_mon + 1;
  int day = timeinfo->tm_mday;
  int zIdx = getZodiacIndex(month, day);
  
  const ZodiacSign& z = zodiacs[zIdx];
  
  unsigned long now = millis();

  // Background stars with twinkling
  for (int i = 0; i < 30; i++) {
    int sx = (i * 137) % 240;
    int sy = (i * 93) % 240;
    float phase = (now * 0.003) + (i * 2.0);
    uint8_t b = 70 + 50 * sin(phase); // brightness between 20 and 120
    spr.drawPixel(sx, sy, spr.color565(b, b, b));
  }
  
  // Shooting stars
  static float ss_x = -100, ss_y = -100, ss_vx = 0, ss_vy = 0;
  static unsigned long last_ss_time = 0;
  static bool ss_active = false;

  if (!ss_active && (now - last_ss_time > 2000)) { // 2s cooldown
    if (random(100) < 3) { // chance to spawn
      ss_x = random(20, 220);
      ss_y = 0;
      ss_vx = random(4, 10) * (random(2) == 0 ? 1 : -1);
      ss_vy = random(4, 10);
      ss_active = true;
    }
  }

  if (ss_active) {
    ss_x += ss_vx;
    ss_y += ss_vy;
    spr.drawLine(ss_x, ss_y, ss_x - ss_vx * 1.5, ss_y - ss_vy * 1.5, TFT_WHITE);
    if (ss_x < -20 || ss_x > 260 || ss_y > 260) {
      ss_active = false;
      last_ss_time = now;
    }
  }
  
  // Drawing constellation
  int cx = centerX;
  int cy = centerY + 15; // offset slightly down
  float scale = 1.8;
  
  // Lines
  for (int i = 0; i < z.lineCount; i++) {
    int idx1 = z.lines[i][0];
    int idx2 = z.lines[i][1];
    int x1 = cx + z.stars[idx1][0] * scale;
    int y1 = cy + z.stars[idx1][1] * scale;
    int x2 = cx + z.stars[idx2][0] * scale;
    int y2 = cy + z.stars[idx2][1] * scale;
    spr.drawLine(x1, y1, x2, y2, spr.color565(80, 80, 150));
  }
  
  // Stars with subtle twinkling
  for (int i = 0; i < z.starCount; i++) {
    int x = cx + z.stars[i][0] * scale;
    int y = cy + z.stars[i][1] * scale;
    
    // twinkle effect based on time and star index
    float phase = (now * 0.002) + (i * 1.5);
    int r = 2 + sin(phase); // radius 1 to 3
    
    spr.fillCircle(x, y, r, spr.color565(200, 220, 255));
    spr.drawCircle(x, y, r + 1, spr.color565(100, 120, 200));
  }
  
  // Text
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Name
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.drawString(tr(z.name_es, z.name_en), centerX, 20);
  
  // Date Range
  spr.setTextColor(spr.color565(150, 150, 150), TFT_BLACK);
  spr.drawString(tr(z.dateRange_es, z.dateRange_en), centerX, 40);
  
  // Symbol
  spr.setTextFont(4);
  spr.setTextSize(1);
  spr.setTextColor(spr.color565(255, 200, 100), TFT_BLACK);
  spr.drawString(String(z.symbol), centerX, 240 - 20);
}

// ─── ELECTRICITY SCREEN ───────────────────────────────────────────────────
void drawElectricityUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);
  
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE);
  spr.setTextFont(1);
  spr.setTextSize(2);
  spr.drawString(tr("PRECIO LUZ", "ELEC PRICE"), centerX, 25);
  
  if (lastElectricityFetch == 0) {
    spr.setTextSize(1);
    spr.drawString(tr("Cargando...", "Loading..."), centerX, centerY);
    return;
  }
  
  int cur_h = timeinfo->tm_hour;
  float p_cur = electricity_prices[cur_h];
  float p_prev = (cur_h > 0) ? electricity_prices[cur_h - 1] : electricity_prices[0];
  float p_next = (cur_h < 23) ? electricity_prices[cur_h + 1] : electricity_prices[23];
  
  // Find min/max for scaling and coloring
  float p_min = 999.0;
  float p_max = -999.0;
  for (int i=0; i<24; i++) {
    if (electricity_prices[i] < p_min) p_min = electricity_prices[i];
    if (electricity_prices[i] > p_max) p_max = electricity_prices[i];
  }
  if (p_max == p_min) p_max = p_min + 0.01; // Avoid div by zero
  
  // Function to get color based on price
  auto getPriceColor = [&](float p) -> uint16_t {
    float norm = (p - p_min) / (p_max - p_min);
    if (norm < 0) norm = 0;
    if (norm > 1) norm = 1;
    uint8_t r, g, b = 0;
    if (norm < 0.5) {
      r = norm * 2.0 * 255.0;
      g = 255;
    } else {
      r = 255;
      g = (1.0 - norm) * 2.0 * 255.0;
    }
    return spr.color565(r, g, b);
  };
  
  // Draw current price
  spr.setTextColor(getPriceColor(p_cur));
  spr.setTextFont(4); // large font
  spr.setTextSize(1);
  char buf[30];
  sprintf(buf, "%.3f", p_cur);
  spr.drawString(String(buf), centerX, 70);
  
  spr.setTextFont(1);
  spr.setTextSize(1);
  spr.drawString("Euros/kWh", centerX, 95);
  
  // Draw previous/next
  spr.setTextFont(1);
  spr.setTextSize(1);
  spr.setTextColor(TFT_LIGHTGREY);
  sprintf(buf, "%02d:00 -> %.3f", (cur_h > 0 ? cur_h-1 : 0), p_prev);
  spr.drawString(String(buf), centerX - 60, 120);
  sprintf(buf, "%02d:00 -> %.3f", (cur_h < 23 ? cur_h+1 : 23), p_next);
  spr.drawString(String(buf), centerX + 60, 120);
  
  // Draw Graph
  int g_x = 10;
  int g_w = 220;
  int g_y = 225;
  int g_h = 75;
  
  float bar_w = (float)g_w / 24.0;
  for (int i=0; i<24; i++) {
    int display_h = (cur_h - 11 + i + 24) % 24;
    float norm = (electricity_prices[display_h] - p_min) / (p_max - p_min);
    int bh = max(2.0f, norm * g_h);
    int bx = g_x + i * bar_w;
    int by = g_y - bh;
    
    uint16_t b_color = getPriceColor(electricity_prices[display_h]);
    if (display_h == cur_h) {
      spr.fillRect(bx, by, bar_w - 1, bh, TFT_WHITE);
      spr.fillRect(bx + 1, by + 1, bar_w - 3, bh - 2, b_color);
      // Indicator line above
      spr.drawLine(bx + (bar_w/2), by - 2, bx + (bar_w/2), by - 6, TFT_WHITE);
      // Label current hour
      spr.setTextColor(TFT_WHITE);
      spr.setTextDatum(TC_DATUM);
      spr.drawString(String(display_h), bx + bar_w/2, g_y + 2);
    } else {
      spr.fillRect(bx, by, bar_w - 1, bh, b_color);
      // Draw sparse hours (e.g. every 6 hours) but don't overlap center
      if (display_h % 6 == 0 && abs(i - 11) > 2) { 
        spr.setTextColor(TFT_DARKGREY);
        spr.setTextDatum(TC_DATUM);
        spr.drawString(String(display_h), bx + bar_w/2, g_y + 2);
      }
    }
  }
}

// ─── ELECTRICITY CLOCK SCREEN ──────────────────────────────────────────────
void drawElecClockUI(struct tm* timeinfo) {
  spr.fillSprite(TFT_BLACK);

  if (lastElectricityFetch == 0) {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(TFT_WHITE);
    spr.setTextFont(1);
    spr.setTextSize(1);
    spr.drawString(tr("Cargando reloj...", "Loading clock..."), centerX, centerY);
    return;
  }

  // Find min/max for scaling and coloring
  float p_min = 999.0;
  float p_max = -999.0;
  for (int i=0; i<24; i++) {
    if (electricity_prices[i] < p_min) p_min = electricity_prices[i];
    if (electricity_prices[i] > p_max) p_max = electricity_prices[i];
  }
  if (p_max == p_min) p_max = p_min + 0.01;

  auto getPriceColor = [&](float p) -> uint16_t {
    float norm = (p - p_min) / (p_max - p_min);
    if (norm < 0) norm = 0;
    if (norm > 1) norm = 1;
    uint8_t r, g, b = 0;
    if (norm < 0.5) {
      r = norm * 2.0 * 255.0;
      g = 255;
    } else {
      r = 255;
      g = (1.0 - norm) * 2.0 * 255.0;
    }
    return spr.color565(r, g, b);
  };

  // Draw 24h Ring
  for (int h=0; h<24; h++) {
    int start_a = (h * 15 + 180) % 360;
    int end_a = start_a + 15;
    spr.drawArc(centerX, centerY, 115, 95, start_a, end_a, getPriceColor(electricity_prices[h]), TFT_BLACK, false);
  }
  
  // Highlight current hour ring segment
  int cur_h = timeinfo->tm_hour;
  int cur_start_a = (cur_h * 15 + 180) % 360;
  int cur_end_a = cur_start_a + 15;
  uint16_t cur_color = getPriceColor(electricity_prices[cur_h]);
  spr.drawArc(centerX, centerY, 118, 92, cur_start_a, cur_end_a, cur_color, TFT_BLACK, false);

  // Draw ticks to separate hours
  for (int i=0; i<24; i++) {
    float a = i * 15 * DEG_TO_RAD - PI/2.0;
    int x1 = centerX + cos(a) * 95;
    int y1 = centerY + sin(a) * 95;
    int x2 = centerX + cos(a) * 115;
    int y2 = centerY + sin(a) * 115;
    spr.drawLine(x1, y1, x2, y2, TFT_BLACK);
  }

  // Draw hour labels
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setTextFont(1);
  spr.setTextSize(1);
  for (int i=0; i<24; i+=6) {
    float a = i * 15 * DEG_TO_RAD - PI/2.0;
    int lx = centerX + cos(a) * 82;
    int ly = centerY + sin(a) * 82;
    spr.drawString(String(i), lx, ly);
  }

  // Draw Sunlight Inner Arc
  if (sunriseTimeStr != "--:--" && sunsetTimeStr != "--:--") {
    float sunriseHour = sunriseTimeStr.substring(0, 2).toFloat() + sunriseTimeStr.substring(3, 5).toFloat() / 60.0;
    float sunsetHour = sunsetTimeStr.substring(0, 2).toFloat() + sunsetTimeStr.substring(3, 5).toFloat() / 60.0;
    int sun_start = ((int)(sunriseHour * 15) + 180) % 360;
    int sun_end = ((int)(sunsetHour * 15) + 180) % 360;
    
    if (sun_start > sun_end) {
      spr.drawArc(centerX, centerY, 94, 88, sun_start, 360, TFT_YELLOW, TFT_BLACK, false);
      spr.drawArc(centerX, centerY, 94, 88, 0, sun_end, TFT_YELLOW, TFT_BLACK, false);
    } else {
      spr.drawArc(centerX, centerY, 94, 88, sun_start, sun_end, TFT_YELLOW, TFT_BLACK, false);
    }
  }

  // (Pizza slice highlight has been removed as requested)
  // Info in center
  spr.setTextDatum(MC_DATUM);
  spr.setTextFont(1);
  spr.setTextSize(1);
  
  // Date
  char dbuf[20];
  strftime(dbuf, sizeof(dbuf), "%d/%m/%Y", timeinfo);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.drawString(String(dbuf), centerX, centerY - 25);
  
  // Max/Min
  char buf[20];
  spr.setTextColor(TFT_RED);
  sprintf(buf, "Max: %.3f", p_max);
  spr.drawString(String(buf), centerX, centerY - 10);
  
  spr.setTextColor(TFT_GREEN);
  sprintf(buf, "Min: %.3f", p_min);
  spr.drawString(String(buf), centerX, centerY + 10);
  
  // Current Price
  spr.setTextColor(getPriceColor(electricity_prices[cur_h]));
  spr.setTextSize(2);
  sprintf(buf, "%.3f", electricity_prices[cur_h]);
  spr.drawString(String(buf), centerX, centerY + 30);
  
  // Small center dot
  spr.fillCircle(centerX, centerY, 3, getPriceColor(electricity_prices[cur_h]));
}

// Returns AQI color matching AQICN standard palette
static uint16_t getAqiColor(int aqi) {
  if (aqi <= 50)  return spr.color565(0,   153,  102);
  if (aqi <= 100) return spr.color565(255, 220,  50);
  if (aqi <= 150) return spr.color565(255, 153,  50);
  if (aqi <= 200) return spr.color565(200,  0,   50);
  if (aqi <= 300) return spr.color565(100,  0,  153);
  return spr.color565(126,  0,   35);
}

static const char* getAqiLabel(int aqi) {
  if (aqi <= 50)  return "BUENO";
  if (aqi <= 100) return "MODERADO";
  if (aqi <= 150) return "SENSIBLES";
  if (aqi <= 200) return "MALO";
  if (aqi <= 300) return "MUY MALO";
  return "PELIGROSO";
}

void drawAirQualityUI() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);

  if (!currentAQI.valid) {
    spr.fillCircle(centerX, centerY, radarRadius, spr.color565(20, 20, 30));
    spr.drawCircle(centerX, centerY, radarRadius, spr.color565(60, 60, 80));
    spr.setTextColor(spr.color565(180, 180, 180));
    spr.setTextFont(2);
    spr.setTextSize(1);
    spr.drawString(tr("Calidad del Aire", "Air Quality"), centerX, centerY - 20);
    spr.setTextColor(spr.color565(120, 120, 120));
    spr.setTextFont(1);
    spr.drawString(tr("Obteniendo datos...", "Fetching data..."), centerX, centerY + 10);
    return;
  }

  int aqi = currentAQI.aqi;
  uint16_t aqiColor = getAqiColor(aqi);
  uint16_t bgColor  = spr.color565(10, 10, 15);

  spr.fillCircle(centerX, centerY, radarRadius, bgColor);

  // Arco de progreso (0-300 AQI → 0-270°)
  int arcAqi   = aqi > 300 ? 300 : aqi;
  int arcDeg   = (int)((arcAqi / 300.0f) * 270);
  int arcStart = 135;
  // Background arc track
  spr.drawArc(centerX, centerY, radarRadius - 2, radarRadius - 10,
              arcStart, arcStart + 270, spr.color565(40, 40, 50), bgColor, false);
  // Colored progress
  if (arcDeg > 0) {
    spr.drawArc(centerX, centerY, radarRadius - 2, radarRadius - 10,
                arcStart, arcStart + arcDeg, aqiColor, bgColor, false);
  }

  // Border
  spr.drawCircle(centerX, centerY, radarRadius,     aqiColor);
  spr.drawCircle(centerX, centerY, radarRadius - 1, aqiColor);

  // AQI number
  spr.setTextColor(aqiColor);
  spr.setTextFont(4);
  spr.setTextSize(aqi >= 100 ? 2 : 3);
  spr.drawString(String(aqi), centerX, centerY - 18);

  // Level label
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextColor(aqiColor);
  spr.drawString(getAqiLabel(aqi), centerX, centerY + 18);

  spr.setTextColor(spr.color565(160, 160, 160));
  spr.drawString(tr("CALIDAD AIRE", "AIR QUALITY"), centerX, 24);
  spr.drawString(tr("(AQI)", "(AQI)"), centerX, 36);

  // Station name
  spr.setTextColor(spr.color565(130, 180, 255));
  int spaceIdx = currentAQI.stationName.indexOf(' ', currentAQI.stationName.length() / 2 - 2);
  if (currentAQI.stationName.length() > 18 && spaceIdx > 0) {
    spr.drawString(currentAQI.stationName.substring(0, spaceIdx), centerX, 204);
    spr.drawString(currentAQI.stationName.substring(spaceIdx + 1), centerX, 216);
  } else {
    spr.drawString(currentAQI.stationName, centerX, 210);
  }

  // Sub-pollutants in quadrants
  const int sx = 55;
  spr.setTextFont(1);
  spr.setTextSize(1);

  // PM2.5 top-left
  spr.setTextColor(spr.color565(180, 180, 255));
  spr.drawString("PM2.5", centerX - sx, centerY - 42);
  spr.setTextColor(currentAQI.pm25 >= 0 ? TFT_WHITE : spr.color565(70, 70, 70));
  spr.drawString(currentAQI.pm25 >= 0 ? String(currentAQI.pm25, 0) : "N/A", centerX - sx, centerY - 30);

  // PM10 top-right
  spr.setTextColor(spr.color565(180, 180, 255));
  spr.drawString("PM10",  centerX + sx, centerY - 42);
  spr.setTextColor(currentAQI.pm10 >= 0 ? TFT_WHITE : spr.color565(70, 70, 70));
  spr.drawString(currentAQI.pm10 >= 0 ? String(currentAQI.pm10, 0) : "N/A", centerX + sx, centerY - 30);

  // O3 bottom-left
  spr.setTextColor(spr.color565(180, 255, 180));
  spr.drawString("O3",    centerX - sx, centerY + 30);
  spr.setTextColor(currentAQI.o3 >= 0 ? TFT_WHITE : spr.color565(70, 70, 70));
  spr.drawString(currentAQI.o3 >= 0   ? String(currentAQI.o3, 0)   : "N/A", centerX - sx, centerY + 42);

  // NO2 bottom-right
  spr.setTextColor(spr.color565(255, 210, 160));
  spr.drawString("NO2",   centerX + sx, centerY + 30);
  spr.setTextColor(currentAQI.no2 >= 0 ? TFT_WHITE : spr.color565(70, 70, 70));
  spr.drawString(currentAQI.no2 >= 0  ? String(currentAQI.no2, 0)  : "N/A", centerX + sx, centerY + 42);

  // Center dot
  spr.fillCircle(centerX, centerY - 3, 2, aqiColor);
}


 
 