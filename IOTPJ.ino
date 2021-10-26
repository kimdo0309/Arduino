#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <avr/wdt.h>

#define DHT11PIN 14
/* D5-DHT11 
 * D6-MORTOR
 * D1,D2 - MPU6050
 * D3- PIEZO
 * LED- Builtin 내장된 led 사용
*/
int sensorPin = D6;
ESP8266WebServer myHttpServer(80);
int ifMotorIsOn=0;
int motorPower = 1023;
int MPU_Address=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
HTTPClient myClient;
DynamicJsonDocument doc(2048);
const char* city;
const char* whe;
const char* detailwhe;
int downp= 16;//아래 LED
PubSubClient myMQTTClient;
WiFiClient myTCPClient;
int piezo = D7;
int numTones = 8;
int tones[] = {261, 294, 330, 349, 392, 440, 494, 523};


void fnRoot(void){
  char tmpBuffer[2000];
  char tmpbuffer[200];
  char tmpbuff[200];
  strcpy (tmpBuffer, "<html>\r\n");
  strcat (tmpBuffer, "IOT Device Experiment<br>\r\n");
  strcat (tmpBuffer, "<a href=/status>Check Motor</a><br>\r\n");
  strcat (tmpBuffer, "<a href=/on>Rotate Motor</a><br>\r\n");
  strcat (tmpBuffer, "<a href=/off>Stop Motor</a><br>\r\n");
  strcat (tmpBuffer, "<form method=\"get\" action=\"input\">");
  strcat (tmpBuffer, "Motor <input type=\"text\" name=\"power\">");
  strcat (tmpBuffer, "<input type=\"submit\" ></form>\r\n");
  snprintf (tmpBuffer, sizeof(tmpBuffer), "%s%s",tmpBuffer,"</html>");
  strcat(tmpBuffer,"<meta charset=utf-8>");
  snprintf(tmpbuffer,sizeof(tmpBuffer),"온도 : %f<br>\r\n",Tmp/340.00+36.53);
  strcat (tmpBuffer, tmpbuffer);
  snprintf(tmpbuff,sizeof(tmpBuffer),"%s의 날씨 : %s , %s<br>\r\n",city,whe,detailwhe);
  strcat (tmpBuffer, tmpbuff);
  
  myHttpServer.send(200,"text/html",tmpBuffer);
}

void fnStatus(void){
  if(ifMotorIsOn){
    myHttpServer.send(200,"text/html","Motor is running.");
  }
  else{
    myHttpServer.send(200,"text/html","motor is stopped.");  
  }
}

void fnCheck(void){
  myHttpServer.sendHeader("Location",String("/status"),true);
  myHttpServer.send(302,"text/plain","");
}

void fnNotFound(void){
  myHttpServer.send(404,"text/plain","WRONG!!!");
}

void fnOn(void){
  if(myHttpServer.authenticate("kim","1234")){
    ifMotorIsOn=1;
    char tmpBuffer[200];
    snprintf(tmpBuffer,sizeof(tmpBuffer),"Motor is running as %d",motorPower);
    myHttpServer.send(200,"text/html",tmpBuffer);
    analogWrite(sensorPin,motorPower);
  }
  else{
    myHttpServer.requestAuthentication();
  }
}

void fnOff(void){
  if(myHttpServer.authenticate("kim","1234")){
    ifMotorIsOn=0;
    myHttpServer.send(200,"text/html","motor is Stopped.");
    analogWrite(sensorPin,0);
  }
  else{
    myHttpServer.requestAuthentication();
  }
}

void fnInput(void){
  char motorSpeed[400];
  if (myHttpServer.hasArg("power")){
    strcpy(motorSpeed,"<meta charset=utf-8>");
    strcat(motorSpeed,"힘을내요 슈퍼파월! as...");
    strcat(motorSpeed, myHttpServer.arg("power").c_str());
    motorPower=atoi(myHttpServer.arg("power").c_str());

    if(motorPower>=1023)motorPower=1023;
    else if(motorPower<0)motorPower=0;

    myHttpServer.send(200,"text/html",motorSpeed);
    analogWrite(sensorPin,motorPower);
  }
  else
    myHttpServer.send(200,"text/plain","Something Wrong!");
}

int readDHT11(int* readTemp, int* readHumid) //온습도센서 값 읽기
{
 
  int dt[82]={0,};
  //phase 1
  digitalWrite(DHT11PIN,1);
  pinMode(DHT11PIN,OUTPUT);
  delay(1);
  digitalWrite(DHT11PIN,0);
  delay(20);
  pinMode(DHT11PIN,INPUT_PULLUP);

  while(1)
    if(digitalRead(DHT11PIN)==1) break;
  while(1) 
    if(digitalRead(DHT11PIN)==0) break;
    
    
  int cnt=0;
  for(cnt=0;cnt<41;cnt++)
  {
    dt[cnt*2]=micros();
    while(1)
      if(digitalRead(DHT11PIN)==1)break;
    dt[cnt*2]=micros()-dt[cnt*2];


    dt[cnt*2+1]=micros();
    while(1)
      if(digitalRead(DHT11PIN)==0)break;
    dt[cnt*2+1]=micros()-dt[cnt*2+1];
  }
  //phase 4
  *readHumid=0;
  for(cnt=1;cnt<9;cnt++) //humid
  { 
    *readHumid=*readHumid<<1;
    if(dt[cnt*2+1]>49) //1
    {
      *readHumid=*readHumid+1;
    }
    else
    {
      *readHumid=*readHumid+0;
    }
    
  }
  for(cnt=17;cnt<25;cnt++) //humid
  {  
    *readTemp=*readTemp<<1;
    if(dt[cnt*2+1]>49) //1
    {
      *readTemp=*readTemp+1;
    }
    else
    {
      *readTemp=*readTemp+0;
    }
  }
  return 1;
 }

void whether(void){
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x3B);
  Wire.endTransmission();

  Wire.requestFrom(MPU_Address,14,true);

  AcX=Wire.read() << 8 | Wire.read();
  AcY=Wire.read() << 8 | Wire.read();
  AcZ=Wire.read() << 8 | Wire.read();
  Tmp=Wire.read() << 8 | Wire.read();
  GyX=Wire.read() << 8 | Wire.read();
  GyY=Wire.read() << 8 | Wire.read();
  GyZ=Wire.read() << 8 | Wire.read();
  
  Serial.print("Tmp = "); Serial.println(Tmp/340.00+36.53);
  float Temp=Tmp/340.00+36.53;

     if(Temp>30){
      digitalWrite(downp,LOW);
      for (int i = 0; i < numTones; i++) {
      tone(piezo, tones[i]);
      delay(500);
      }
    }
    else{
      digitalWrite(downp,HIGH);
      noTone(piezo);
    }
  myClient.begin("http://api.openweathermap.org/data/2.5/weather?q=OKCHEON&appid=d32bbdbc4d6d6ce5105f7de8bd83e4ea");
  int getResult=myClient.GET();
  if(getResult==HTTP_CODE_OK){//200
    String receivedData=myClient.getString();
    //Serial.printf("%s\r\n\r\nEND Transmission\r\n",receivedData.c_str());
    deserializeJson(doc,receivedData);
    city=doc["name"];
    Serial.printf("이곳의 도시는 %s 입니다 \r\n",city);
    
    whe=doc["weather"][0]["main"];
    detailwhe=doc["weather"][0]["description"];
    float humidity=doc["main"]["humidity"];
    float feels=doc["main"]["feels_like"];
    
    Serial.printf("날씨는 %s 상황이고 그건 자세히는 %s 입니다.\r\n",whe,detailwhe);
    //Serial.printf("현재온도는 %f 입니다 \r\n",(float)(doc["main"]["temp"])-273.0);
    //Serial.printf("현재습도는 %f 입니다 \r\n",humidity);
    //Serial.printf("체감온도는 %f 입니다 \r\n",feels-273.0);
   
  }
  else{
    Serial.printf("ERROR, code : %d \r\n",getResult);
  }
}

void setup() {
  Serial.begin(74880);

  pinMode(downp, OUTPUT);
  Serial.print("start\r\n");
  WiFi.begin("NSM824","74240824");
  while(1){
    if(WiFi.status()==WL_CONNECTED)
      break;
      delay(500);
  }
  Serial.printf("\r\n Ready to internnet!\r\n");
  
  analogWriteFreq(100000);
  pinMode(sensorPin, OUTPUT);
  Serial.printf("Connected!\r\n");
  Serial.printf("Please contact IP Addr...");
  Serial.println(WiFi.localIP());

  Wire.begin(4,5);// GPIO 설정
  Wire.beginTransmission(MPU_Address);//주소값을 넣어준다
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);

  myHttpServer.on("/",fnRoot);
  myHttpServer.on("/status",fnStatus);
  myHttpServer.on("/check",fnCheck);
  myHttpServer.on("/on",fnOn);
  myHttpServer.on("/off",fnOff);
  myHttpServer.on("/input",fnInput);
  myHttpServer.onNotFound(fnNotFound);
  myHttpServer.begin();
}

unsigned long long prevMillis=0;

void loop() {
  int readTemp;
  int readHumid;
  
  whether();

  wdt_reset();
  
  myHttpServer.handleClient();

  if(millis()-prevMillis>60000){
    prevMillis=millis();
    
    Wire.beginTransmission(MPU_Address);
    Wire.write(0x3B);
    Wire.endTransmission();

    Wire.requestFrom(MPU_Address,14,true);

    AcX=Wire.read() << 8 | Wire.read();
    AcY=Wire.read() << 8 | Wire.read();
    AcZ=Wire.read() << 8 | Wire.read();
    Tmp=Wire.read() << 8 | Wire.read();
    GyX=Wire.read() << 8 | Wire.read();
    GyY=Wire.read() << 8 | Wire.read();
    GyZ=Wire.read() << 8 | Wire.read();
  
    Serial.print("Tmp = "); Serial.println(Tmp/340.00+36.53);
    float TEMP=Tmp/340.00+36.53;
    char tempBuf[200];
    Serial.printf("Temp:%f\r\n",TEMP);
    snprintf(tempBuf,sizeof(tempBuf),"http://api.thingspeak.com/update?api_key=EZG3D79BKQRT5IAY&field1=%f",TEMP);
    myClient.begin(tempBuf);
    myClient.GET();
    myClient.getString();
    myClient.end();
    
    delay(30000);

    readDHT11(&readTemp,&readHumid);
    if(myMQTTClient.connected()==0){
      myMQTTClient.setClient(myTCPClient);
      myMQTTClient.setServer("mqtt.thingspeak.com",1883);
      myMQTTClient.connect("DOBBY_DEVICE");
    }
    Serial.printf("if MqttConnected? : %d\r\n",myMQTTClient.connected());
    Serial.printf("Humid:%d\r\n",readHumid);
    
    char str[80];
    sprintf(str,"%d",readHumid);
    myMQTTClient.publish("channels/1393922/publish/fields/field2/EZG3D79BKQRT5IAY",str);

  }
  myMQTTClient.loop();
}
