#ifndef _GPSPOSITION_H
#define _GPSPOSITION_H
#include "Arduino.h"
#include <Adafruit_GPS.h>
class GpsPosition 
{
  public:
    GpsPosition(); // Constructor
    GpsPosition(Adafruit_GPS GPS); // Constructor
    
    bool valid;
    uint8_t hour;          ///< GMT hours
    uint8_t minute;        ///< GMT minutes
    uint8_t seconds;       ///< GMT seconds
    uint16_t milliseconds; ///< GMT milliseconds
    uint8_t year;          ///< GMT year
    uint8_t month;         ///< GMT month
    uint8_t day;           ///< GMT day
    float lat;
    float lng;
    float altitude;
    float speed;
    float angle;
    float magvariation;
    uint8_t fixquality;    ///< Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
    uint8_t fixquality_3d; ///< 3D fix quality (1, 3, 3 = Nofix, 2D fix, 3D fix)
    uint8_t satellites;
    String getFilename();
    String getJsonTimestamp();
	String getJson();
  private:
    void common_init(void);
    String getTimestamp(String type);
};

#endif
