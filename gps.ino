
#define _USE_MATH_DEFINES
#include <Adafruit_GPS.h>
#include <math.h>
#include "src/GpsPosition/GpsPosition.h"
// what's the name of the hardware serial port?
#define GPSSerial Serial1

#define GpsPositionsLength 10

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false
const float log_mindistance = .1;
const float log_distance = 10;
const float log_speed = 10;
const float log_angle = 5;

float last_latitude = 0;
float last_longitude = 0;
float last_speed = 0;
float last_angle = 0;
uint32_t gpsTimer = millis();



GpsPosition gpsPositions[GpsPositionsLength];
GpsPosition currentPosition = GpsPosition();

int gpsPositionsIndex = 0;

void getGpsDataHandler(){
  String Date = "";
  String Data = "";

  if (Server.arg("Date")== ""){     //Parameter not found
    Server.send(200, "text/json", "{\"valid\":false, \"exist\":true}"  );
    Serial.println("date is not selected");
    return;
  }else{     //Parameter found
    Date = Server.arg("Date");     //Gets the value of the query parameter
  }

  File file = SD.open("/" + Date + ".json");
  if(!file){
      Serial.println("No Data for " + Date);
      Server.send(200, "text/json", "{\"valid\":true, \"exist\":false}");
      return;
  }
  Serial.print("Read from file");
  while(file.available()){
      Data += char(file.read());
  }
  file.close();
  Server.send(200, "text/json", "[" + Data + "]");
  Serial.println("[" + Data + "]");
}

void gpsGetCurrentPosition(){
    Server.send(200, "application/json", currentPosition.getJson());
}

void gpsGetCurrentPath(){
  String data = "[";
  
  for (int i = gpsPositionsIndex-1; i > 0; i--){
    if(i != gpsPositionsIndex -1){
      data = data + ","; 
    }
    data = data + gpsPositions[i].getJson();
  }
  for (int i = GpsPositionsLength-1; i > gpsPositionsIndex; i--){
    if(i != gpsPositionsIndex -1){
      data = data + ","; 
    }
    data = data + gpsPositions[i].getJson();
  } 
  data = data + "]";
  Serial.print("gpsGetCurrentPath " + String(gpsPositionsIndex) + data);
  Server.send(200, "application/json", data);
}

float degreesToRadians(float degrees) {
  return degrees * M_PI / 180;
}

float distanceInKmBetweenEarthCoordinates(float lat1,float lon1,float lat2,float lon2) {
  int earthRadiusKm = 6371;
  float dLat = degreesToRadians(lat2-lat1);
  float dLon = degreesToRadians(lon2-lon1);
  lat1 = degreesToRadians(lat1);
  lat2 = degreesToRadians(lat2);
  float a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
  float c = 2 * atan2(sqrt(a), sqrt(1-a)); 
  return earthRadiusKm * c;
}


void gpsSetup() {
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  delay(100);
  //PMTK_SET_BAUD_115200
  GPS.sendCommand(PMTK_SET_BAUD_115200);
  delay(100);
  GPS.begin(115200);
  delay(100);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // 200 miliHz update rate
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_200_MILLIHERTZ);
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  //init the gpsPositions Array

  for(int i = 0; i < GpsPositionsLength; i ++){
    gpsPositions[i] = GpsPosition();
  }
}

void gpsHttpSetup() {
   Server.on("/getGpsData", getGpsDataHandler);
   Server.on("/getCurrentPosition", gpsGetCurrentPosition);
   Server.on("/getCurrentPath", gpsGetCurrentPath);
}


String currentFilename = "";
void gpsLoop(){


  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  //if (GPSECHO)
  // if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    char* NmeaString = GPS.lastNMEA();    // this GPS.lastNMEA() sets the newNMEAreceived() flag to false
    GPS.parse(NmeaString); //return; // we can fail to parse a sentence in which case we should just wait for another
    if (GPSECHO){
      Serial.println(NmeaString); // this also sets the newNMEAreceived() flag to false
    }    
  }
  // if millis() or gpsTimer wraps around, we'll just reset it
  if (gpsTimer > millis()) gpsTimer = millis();
  // approximately every 10 seconds or so, print out the current stats
  if (millis() - gpsTimer > 10000) {
    gpsTimer = millis();  // reset the gpsTimer
    if(GPS.fix){
      currentPosition = GpsPosition(GPS);
      if (currentFilename == ""){
        currentFilename = currentPosition.getFilename() + ".json";
        writeFile("/gpsLogs/" + currentFilename, currentPosition.getJson());
        gpsPositions[gpsPositionsIndex] = currentPosition;
         gpsPositionsIndex++;
      }
      float distanceMoved =  distanceInKmBetweenEarthCoordinates(last_latitude, last_longitude, GPS.latitudeDegrees, GPS.longitudeDegrees);
      if ( distanceMoved >= log_mindistance && ( distanceMoved >= log_distance || abs(last_angle - GPS.angle) >= log_angle ||  abs(last_speed - GPS.speed) >= log_speed)){ 
         appendFile( "/gpsLogs/" + currentFilename, currentPosition.getJson());
         last_latitude = GPS.latitudeDegrees;
         last_longitude = GPS.longitudeDegrees;
         last_speed = GPS.speed;
         last_angle = GPS.angle;
         gpsPositions[gpsPositionsIndex] = currentPosition;
         gpsPositionsIndex++;
         if (gpsPositionsIndex >= GpsPositionsLength){
          gpsPositionsIndex = 0;
         }
      }else{
        Serial.println("distance Moved was only " + String(distanceMoved));
      }
    }else{
        Serial.println("No GPS Fix!~");
    }
 }
}
