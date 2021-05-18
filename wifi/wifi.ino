#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(16, OUTPUT);
  Serial.begin(74880);
  delay(1000);

  Serial.println("Hello world\r\n");
  WiFi.mode(WIFI_AP_STA);

  Serial.println(WiFi.getMode());
  int foundN=WiFi.scanNetworks(0,1);
  Serial.printf("I found ssid  %d\r\n",foundN);
  int i=0;
  for(i=0;i<foundN; i++)
  {//
    Serial.printf("SSID: %s, WifiStrength : %d, channel: %d, MAC: %s\r\n", WiFi.SSID(i).c_str(),WiFi.RSSI(i),WiFi.channel(i),WiFi.macAddress().c_str());
  }

  //Serial.printf("%d %d %d %d %d \r\n",WL_CONNECTED,WL_NO_SSID_AVAIL,WL_CONNECT_FAILED,WL_IDLE_STATUS,WL_DISCONNECTED);
  WiFi.begin("wevo","");
  WiFi.setAutoReconnect(1);
  while(1){
    int connResult=WiFi.status();
    if (connResult==WL_CONNECTED)
    {
      Serial.printf("connection OK!\r\n");
      break;
    }
    else
    {
      //Serial.printf("Connection Failed. reason:%d\r\n",connResult);
      delay(500);
    }
  }
  Serial.printf("My MAC Address: %s\r\n",WiFi.gatewayIP().toString().c_str());
  Serial.printf("My IP Address: %s\r\n",WiFi.localIP().toString().c_str());
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int i=0;
  Ping.ping("www.google.com");
  int returnMs=Ping.averageTime();

  Serial.printf("Now google ping time : %d ms \r\n",returnMs);
  for(i=0;i<returnMs;i++){
    digitalWrite(16,LOW);//밑에 led on
    delay(50);
    digitalWrite(16,HIGH);
    delay(50);
  }
  delay(5000);
}
