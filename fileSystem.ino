
#include <SPI.h>
#include <SD.h>

File fsUploadFile;
uint8_t sdCardType = CARD_NONE;

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (Server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".json")) {
    return "application/json";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists( String path){
  bool yes = false;
  File file = SD.open(path, "r");
  if(file && file.isDirectory()== false){
    yes = true;
  }
  file.close();
  return yes;
}



bool handleFileRead(String path) {
  Serial.println("handleFileRead SD: " + path);
  if (path.endsWith("/")) {
    path += "webserver/index.htm";
    //Serial.println("handleFileRead: index.htm!");
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists( pathWithGz) || exists( path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
      Serial.println("handleFileRead SD with gz: " + path);
    }
    File file = SD.open(path, "r");
    if (path.startsWith("/gpsLogs/")){
      //this is a special case where we need to add the [] to the file.
      Serial.println("Send GPS Data File " + path);
        Server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
        Server.sendHeader("Pragma", "no-cache"); 
        Server.sendHeader("Expires", "-1"); 
        Server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
        Server.send(200, contentType, ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        Server.sendContent("[");
        char dataBuf[257];
        int dataBufIndex = 0;
        while(file.available()){
            dataBuf[dataBufIndex] = file.read();
            dataBufIndex++;
            if (dataBufIndex == 256){
              dataBuf[256] = 0;
              Server.sendContent(String(dataBuf));
              dataBufIndex = 0;
            }
        } 
        if (dataBufIndex > 0){
              dataBuf[dataBufIndex] = 0;
              Server.sendContent(String(dataBuf));
              dataBufIndex = 0;
            }
        Server.sendContent("]");
        Server.client().stop(); // Stop is needed because no content length was sent
    }else{
      Server.streamFile(file, contentType);
    }
    file.close();
    return true;
  }else{
    return false;  
  }
}


void handleFileUpload() {

  if (Server.uri() != "/edit") {
    return;
  }
  
  HTTPUpload& upload = Server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SD.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete() {

  if (Server.args() == 0) {
    return Server.send(500, "text/plain", "BAD ARGS");
  }
  String path = Server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return Server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return Server.send(404, "text/plain", "FileNotFound");
  }
  SD.remove(path);
  Server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {

  if (Server.args() == 0) {
    return Server.send(500, "text/plain", "BAD ARGS");
  }
  String path = Server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return Server.send(500, "text/plain", "BAD PATH");
  }
  if (exists( path)) {
    return Server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SD.open(path, "w");
  if (file) {
    file.close();
  } else {
    return Server.send(500, "text/plain", "CREATE FAILED");
  }
  Server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  
  if (!Server.hasArg("dir")) {
    Server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = Server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = SD.open(path);
  //path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"lw\":\"";
          output += String(file.getLastWrite());
          output += "\",\"name\":\"";
          output += String(file.name()).substring(path.length());
          output += "\"}";
          file = root.openNextFile();
      }
  }

  
  output += "]";
  Server.send(200, "text/json", output);
}



void listDir( String dirname, uint8_t levels){
    Serial.println("Listing directory: " + dirname);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    File root = SD.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
//            Serial.print("  LW: ");
//            Serial.println(file.getLastWrite());
            
        }
        file = root.openNextFile();
    }
}

void createDir( String path){
    Serial.println("Creating Dir: " + path);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    if(SD.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(String path){
    Serial.println("Removing Dir: "  + path);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    if(SD.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile( String path){
    Serial.println("Reading file: " + path);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    File file = SD.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile( String path, String message){
    Serial.println("Writing file: " +  path);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    File file = SD.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.println(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile( String path, String message){
    Serial.println("Appending to SD file: " + path);
    if(sdCardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    File file = SD.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.println("," + message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}


void setupFileSystem(){
  
    Serial.println("Checking for SD Card");
      if(SD.begin()){
        Serial.println("SD Card Adapter Found");
        sdCardType = SD.cardType();
        if(sdCardType == CARD_NONE){
            Serial.println("No SD card attached");
        }else{
          Serial.print("SD Card Type: ");
          if(sdCardType == CARD_MMC){
              Serial.println("MMC");
          } else if(sdCardType == CARD_SD){
              Serial.println("SDSC");
          } else if(sdCardType == CARD_SDHC){
              Serial.println("SDHC");
          } else {
              Serial.println("UNKNOWN");
          }
          Serial.printf("SD Card Size: %lluMB\n", SD.cardSize() / (1024 * 1024));
          Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
          Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
          //listSDDir(SD, "/", 0);
        }
      }else{
        Serial.println("SD Card Adapter Not Found");
      }
  Serial.printf("\n");
 
}

bool hasSDCard(){
  if (sdCardType == CARD_NONE){
    return false;
  }else{
    return true;
  }
}
void handleRoot(){
  File file = SD.open("/webserver/index.htm", "r");
  Server.streamFile(file, "text/html");
}
