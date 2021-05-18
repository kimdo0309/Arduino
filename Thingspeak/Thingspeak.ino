#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>

int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float x,y,z;
int MPU_Address=0x68;

PubSubClient myMQTTClient;
HTTPClient myClient;
WiFiClient myTCPClient;

void setup() {
  Serial.begin(74880);
  delay(1000);
  
  WiFi.begin("wevo","dong0309");
  while(1){
    if(WiFi.status()!=WL_CONNECTED)delay(500);
    else break;
  }

  Wire.begin(4,5);// GPIO 설정
  Wire.beginTransmission(MPU_Address);//주소값을 넣어준다
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1C);
  Wire.write(0x18);//Accel +-16g
  Wire.endTransmission(true);

  myMQTTClient.setClient(myTCPClient);
  myMQTTClient.setServer("mqtt.thingspeak.com",1883);
  myMQTTClient.connect("DOBBY_DEVICE");
}

unsigned long long prevMillis=0;

void loop() {
  if(millis()-prevMillis>40000){
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

    Serial.print("Tmp = ");Serial.println(Tmp/340.00+36.53);
    char tempBuf[200];
    snprintf(tempBuf,sizeof(tempBuf),"http://api.thingspeak.com/update?api_key=8A5AAUM597H3LCBF&field1=%f",Tmp/340.00+36.53);
    myClient.begin(tempBuf);
    myClient.GET();
    myClient.getString();
    myClient.end();
    delay(20000);

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
    z=(float)AcZ/2048;
    
    if(myMQTTClient.connected()==0){
      myMQTTClient.setClient(myTCPClient);
      myMQTTClient.setServer("mqtt.thingspeak.com",1883);
      myMQTTClient.connect("DOBBY_DEVICE");
    }
    Serial.printf("if MqttConnected? : %d\r\n",myMQTTClient.connected());
    
    Serial.print("AZ = "); Serial.print(z); Serial.println("g");
    char str[80];
    sprintf(str,"%lf",z);
    myMQTTClient.publish("channels/1390679/publish/fields/field2/8A5AAUM597H3LCBF",str);
  }
  myMQTTClient.loop();
}
