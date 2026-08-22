#include "math_utils.h"

String getAirportName(String id) {
  if (id == "MAD") return "MADRID";
  if (id == "LHR") return "LONDRES";
  if (id == "JFK") return "NEW YORK";
  if (id == "DXB") return "DUBAI";
  if (id == "HND") return "TOKIO";
  if (id == "CDG") return "PARIS";
  if (id == "AMS") return "AMSTERDAM";
  if (id == "FRA") return "FRANKFURT";
  if (id == "ATL") return "ATLANTA";
  if (id == "SIN") return "SINGAPUR";
  return id;
}

float toRad(float deg) {
  return deg * M_PI / 180.0;
}

float toDeg(float rad) {
  return rad * 180.0 / M_PI;
}

void calculatePolar(Airplane& plane) {
  const float R = 6371.0; 
  float lat1 = toRad(pref_lat);
  float lon1 = toRad(pref_lon);
  float lat2 = toRad(plane.lat);
  float lon2 = toRad(plane.lon);
  
  float dLat = lat2 - lat1;
  float dLon = lon2 - lon1;
  
  float a = sin(dLat/2) * sin(dLat/2) + cos(lat1) * cos(lat2) * sin(dLon/2) * sin(dLon/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  plane.distanceKm = R * c;
  
  float y = sin(dLon) * cos(lat2);
  float x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
  float bearing = atan2(y, x);
  bearing = toDeg(bearing);
  if (bearing < 0) bearing += 360.0;
  
  plane.bearingDeg = bearing;
}

int getMoonPhase(int year, int month, int day) {
  int c = 0;
  int e = 0;
  double jd = 0;
  int b = 0;
  
  if (month < 3) {
    year--;
    month += 12;
  }
  ++month;
  c = 365.25 * year;
  e = 30.6 * month;
  jd = c + e + day - 694039.09; // JD relative to known new moon
  jd /= 29.5305882; // divide by moon cycle
  b = (int)jd; // integer part
  jd -= b; // leaving fractional part
  
  b = round(jd * 8); // scale fraction from 0-8
  if (b >= 8) {
    b = 0;
  }
  return b; // 0-7
}

float getMoonPhaseFraction(int year, int month, int day) {
  int c = 0;
  int e = 0;
  double jd = 0;
  
  if (month < 3) {
    year--;
    month += 12;
  }
  ++month;
  c = 365.25 * year;
  e = 30.6 * month;
  jd = c + e + day - 694039.09; 
  jd /= 29.5305882; 
  int b = (int)jd; 
  jd -= b; 
  
  return (float)jd; // 0.0 to 1.0
}
