#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

HTTPClient myClient;
DynamicJsonDocument doc(2048);


void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.begin("wevo","dong0309");
  while(1){
    if(WiFi.status()==WL_CONNECTED) break;
    delay(500);
  }
  Serial.printf("\r\n Ready to internnet!\r\n");
}

void loop() {
  myClient.begin("http://api.openweathermap.org/data/2.5/weather?q=OKCHEON&appid=d32bbdbc4d6d6ce5105f7de8bd83e4ea");
  int getResult=myClient.GET();
  if(getResult==HTTP_CODE_OK){//200
    String receivedData=myClient.getString();
    Serial.printf("%s\r\n\r\nEND Transmission\r\n",receivedData.c_str());
    deserializeJson(doc,receivedData);
    const char* city=doc["name"];
    Serial.printf("이곳의 도시는 %s 입니다 \r\n",city);
    
    const char* whe=doc["weather"][0]["main"];
    const char* detailwhe=doc["weather"][0]["description"];
    float humidity=doc["main"]["humidity"];
    float feels=doc["main"]["feels_like"];
    
    Serial.printf("날씨는 %s 상황이고 그건 자세히는 %s 입니다.\r\n",whe,detailwhe);
    Serial.printf("현재온도는 %f 입니다 \r\n",(float)(doc["main"]["temp"])-273.0);
    Serial.printf("현재습도는 %f 입니다 \r\n",humidity);
    Serial.printf("체감온도는 %f 입니다 \r\n",feels-273.0);
    
  }
  else{
    Serial.printf("ERROR, code : %d \r\n",getResult);
  }
  delay(5000);
}
