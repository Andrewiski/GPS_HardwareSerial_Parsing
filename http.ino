void handleMenuHtml(void){
  
   Server.send(200, "text/html", AUTOCONNECT_LINK(BAR_32)); 
}
void handleNotFound() {
  
  
  //Serial.println("not found: " + Server.uri());
  if (hasSDCard() ==true && handleFileRead(Server.uri())) {
    return;
  }
  
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += Server.uri();
  message += "\nMethod: ";
  message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += Server.args();
  message += "\n";
  for (uint8_t i = 0; i < Server.args(); i++) {
    message += " NAME:" + Server.argName(i) + "\n VALUE:" + Server.arg(i) + "\n";
  }
  Server.send(404, "text/plain", message);
  Serial.print(message);
}
void setupHttpServer(){
  Server.on("/", HTTP_GET, handleRoot); 
  //SERVER INIT
  //get all GPIO statuses in one json call
  Server.on("/autoconnectMenu", HTTP_GET, handleMenuHtml);
  //list directory
  Server.on("/list", HTTP_GET, handleFileList);
  //load editor
  Server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/webserver/edit.htm")) {
      Server.send(404, "text/plain", "FileNotFound");
    }
  });

  Server.on("/favicon.ico",HTTP_GET, []() {
    if (!handleFileRead("/webserver/favicon.ico")) {
      Server.send(404, "text/plain", "FileNotFound");
    }
  });

  
  //create file
  Server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  Server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  Server.on("/edit", HTTP_POST, []() {
    Server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from FILESYSTEM
  Server.onNotFound(handleNotFound);

  
}
