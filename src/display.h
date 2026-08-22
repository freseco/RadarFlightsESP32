#pragma once
#include "globals.h"
#include "math_utils.h"
#include <time.h>

void drawRadarUI();
void drawPlanes();
void drawTimeUI(struct tm* timeinfo);
void drawAnalogTimeUI(struct tm* timeinfo);
void drawAnalog24hTimeUI(struct tm* timeinfo);
void drawTimeUI(struct tm* timeinfo);
void drawAnalogTimeUI(struct tm* timeinfo);
void drawAnalog24hTimeUI(struct tm* timeinfo);
void drawMoonUI(struct tm* timeinfo);
void drawArtificialHorizon();
void drawISS();
void drawSunArc(struct tm* timeinfo);
void drawWeatherUI(struct tm* timeinfo);

void drawGhostPlane();
void drawWeatherIcon(int x, int y, int type);
void drawSplashScreen(String wifiStatus, uint16_t wifiColor);
void animateZoom(bool zoomIn);
