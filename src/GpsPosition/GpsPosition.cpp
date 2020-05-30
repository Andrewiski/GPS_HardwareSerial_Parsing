#include "GpsPosition.h"


GpsPosition::GpsPosition() {
  common_init();
}

GpsPosition::GpsPosition(Adafruit_GPS GPS){
  common_init();
  valid = true;
  lat =  GPS.latitudeDegrees;
  lng = GPS.longitudeDegrees;
  hour = GPS.hour;          
  minute = GPS.minute;        
  seconds = GPS.seconds;       
  milliseconds = GPS.milliseconds; 
  year = GPS.year;          
  month = GPS.month;         
  day = GPS.day;           
  altitude = GPS.altitude;
  speed = GPS.speed;
  angle = GPS.angle;
  magvariation = GPS.magvariation;
  fixquality = GPS.fixquality;    
  fixquality_3d = GPS.fixquality_3d; 
  satellites = GPS.satellites;
}

void GpsPosition::common_init(void){
  hour = minute = seconds = year = month = day = fixquality = fixquality_3d = satellites = 0;  // uint8_t
  valid = false;         // bool
  milliseconds = 0;    // uint16_t
  lat = lng =  altitude = speed = angle = magvariation = 0.0;
}




String GpsPosition::getJsonTimestamp(){
  return getTimestamp("json");
}

String GpsPosition::getFilename(){
  return getTimestamp("filename");
}


String GpsPosition::getTimestamp(String type){
  String Year = "20" + String(year);
  String Month = String(month);
  if (Month.length() == 1){
    Month = "0" + Month;      
  }
  String Day = String(day);
  if (Day.length() == 1){
    Day = "0" + Day;      
  }
  String Hours = String(hour);
  if (Hours.length() == 1){
    Hours = "0" + Hours;      
  }
  String Minutes = String(minute);
  if (Minutes.length() == 1){
    Minutes = "0" + Minutes;      
  }
  String Seconds = String(seconds);
  if (Seconds.length() == 1){
    Seconds = "0" + Seconds;      
  }
  //"2012-04-23T18:25:43.511Z"
  float millisSec = float(milliseconds) / 10000.000;
  String millisSecond = String(millisSec,3);
  millisSecond = millisSecond.substring(1);

  if(type == "filename"){
    return Year + Month + Day + "_" + Hours + Minutes + Seconds; 
  }else if(type == "json"){
    return Year + "-" + Month + "-" + Day + "T" + Hours + ":" + Minutes + ":" + Seconds  +  millisSecond + "Z"; 
  }else{
    return "";
  }
}

String GpsPosition::getJson(){
    return "{\"valid\":true, \"lat\":"  + String(lat, 6) + ", \"lng\":"  + String(lng, 6) + ", \"time\":\"" + getJsonTimestamp() + "\", \"angle\": " + String(angle, 3) + "}";
}
