#include <ESP8266WiFi.h>
//TCPClient
WiFiClient tcpIpClient;

void setup(){
  Serial.begin(115200);
  delay(1000);
  WiFi.begin("wevo","dong0309");
  while(1){
    if(WiFi.status()==WL_CONNECTED)
     break;
    else
     delay(100);
  }
  Serial.printf("WiFi Connected!\r\n");
}

void loop(){
  if(tcpIpClient.connected()==0){
    tcpIpClient.connect("www.mju.ac.kr",80);
    return;
  }
  tcpIpClient.printf("GET /sites/mjukr/intro/intro.html HTTP/1.1\r\n");
  tcpIpClient.printf("host: www.mju.ac.kr\r\n");
  tcpIpClient.printf("\r\n");
  tcpIpClient.printf("\r\n");
  delay(2000);

  Serial.printf("Now start........\r\n\r\n");
  while(1){
    int readChar=tcpIpClient.read();
    if(readChar==-1){
      delay(1000);
      if(tcpIpClient.available()==0){
        Serial.printf("\r\n\r\nThis is END....\r\n");
        break;
      }
    }
    Serial.printf("%c",readChar);
  }
  delay(5000);
}
