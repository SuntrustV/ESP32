#include <WiFi.h>
#include <Update.h>

WiFiClient client;

long contentLength = 0;
bool isValidContentType = false;


const char SSID[] = "TP-LINK_29DAFC";
const char PSWD[] = "44459055";

//String host = "192.168.0.103";
String host = "cj99729.tmweb.ru";
int port = 80;
String bin = "/update.bin";


String getHeaderValue(String header, String headerName) {
  return header.substring(strlen(headerName.c_str()));
}


void execOTA() {

  Serial.println("Connecting to: " + String(host));

  if (client.connect(host.c_str(), port)) {
    Serial.println("Fetching Bin: " + String(bin));

    client.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Cache-Control: no-cache\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client Timeout !");
        client.stop();
        return;
      }
    }


    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3

        {{BIN FILE CONTENTS}}

    */
    
    while (client.available()) {

      String line = client.readStringUntil('\n');
      line.trim();


      if (!line.length()) {
        break; 
      }

      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }


      if (line.startsWith("Content-Length: ")) {
        contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }

      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {

    Serial.println("Connection to " + String(host) + " failed. Please check your setup");
  }


  Serial.println("Content length : " + String(contentLength) + ", Is valid content type : " + String(isValidContentType));


  if (contentLength && isValidContentType) {

    bool canBegin = Update.begin(contentLength);


    if (canBegin) {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete.");

      size_t written = Update.writeStream(client);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      
      Serial.println("Not enough space to begin OTA");
      client.flush();
    }
  } else {
    Serial.println("There was no content in the response");
    client.flush();
  }
}

void setup() {

  Serial.begin(9200);
  delay(10);

  Serial.println("Connecting to " + String(SSID));


  WiFi.begin(SSID, PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); 
    delay(500);
  }

  Serial.println("");
  Serial.println("Connected to " + String(SSID));

  execOTA();
}

void loop() {
 
}
