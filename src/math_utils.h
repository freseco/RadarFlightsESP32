#pragma once
#include "globals.h"
#include <math.h>

String getAirportName(String id);
float toRad(float deg);
float toDeg(float rad);
void calculatePolar(Airplane& plane);
int getMoonPhase(int year, int month, int day);
float getMoonPhaseFraction(int year, int month, int day);
