// Author: AndreasVan 2015-02-16
// This code is designed for Arduino Ethernet Rev. 3 board by AndreasVan
// this code is public domain, enjoy!

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,199,121);
IPAddress dns1(192,168,199,1);
IPAddress gateway(192,168,199,1);
IPAddress subnet(255,255,255,0);
EthernetServer server(80);

File webFile;

void setup() {
    Ethernet.begin(mac, ip, dns1, gateway, subnet);
    server.begin();
    Serial.begin(9600);
    digitalWrite(10, HIGH);
    if (!SD.begin(4)) {
        return;
    }
}

void loop() {
    int BUFSIZ = 255;
    int index = 0;
    char clientline[BUFSIZ];
    EthernetClient client = server.available();
    if (client) {
        boolean currentLineIsBlank = true;
        index = 0;
        while (client.connected()) {
            if (client.available()) { 
                char c = client.read();
                if (c != '\n' && c != '\r') {
                  clientline[index] = c;
                  index++;
                  if (index >= BUFSIZ) 
                    index = BUFSIZ -1;

                  continue;
                }
                clientline[index] = 0;
                char* filename = processFile(clientline);
                Serial.print("Requested: ");
                Serial.println(filename);
                /* Setup AJAX Requests Here:
                 * These are just direct output values that are requested 
                 * with AJAX.
                 */
                if (strstr(clientline, "GET /?data=val1") != 0) {
                  code200(client);
                  client.println(1337);
                  break;
                }

                if (SD.exists(filename)) {
                   code200(client);
                   webFile = SD.open(filename);
                   if (webFile) {
                       while(webFile.available()) {
                           client.write(webFile.read());
                       }
                       webFile.close();
                   }
                   break;
                } else {
                    if (strlen(filename) < 2) {
                      webFile = SD.open("index.htm");
                       if (webFile) {
                           while(webFile.available()) {
                               client.write(webFile.read());
                           }
                           webFile.close();
                       }
                    } else {
                      client.println("HTTP/1.1 404 Not Found");
                      client.println("Content-Type: text/html");
                      client.println("Connection: close");
                      client.println();
                      client.println("<html><head><title>404 - Not Found</title></head><body><h1>404 - Not Found</h1></body></html>");
                      break;
                    }
                }
                break;
            }
        } 
        delay(1);
        client.stop();
    }
}

void code200(EthernetClient client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
}

char* processFile(char clientline[255]) {
   char *filename;
   filename = clientline + 5;
  (strstr(clientline, " HTTP"))[0] = 0;
  return filename;
}
