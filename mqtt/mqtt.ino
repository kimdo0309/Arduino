#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

int MPU_Address=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float x,y,z;
const char* s1="ACCEL";
const char* s2="TEMP";

char tempbuffer[20]={0,};

WiFiClient myTCPClient;
PubSubClient myMQTTClient;

void dobbyCB(const char topic[],byte *data,unsigned int length){
  memcpy(tempbuffer, data,(length>19?19:length));
  //if(length>19)19 else length
  Serial.printf("Topic:%s , new data comes: %s\r\n",topic,tempbuffer);
}

void setup() {
  Serial.begin(74880);
  delay(1000);

  Wire.begin(4,5);// GPIO 설정
  Wire.beginTransmission(MPU_Address);//주소값을 넣어준다
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1C);
  Wire.write(0x18);//Accel +-16g
  Wire.endTransmission(true);
  
  WiFi.begin("wevo","dong0309");

  while(1)
  {
    if(WiFi.status()!=WL_CONNECTED)delay(500);
    else break;
  }
  Serial.printf("Connect...\r\n");

  myMQTTClient.setClient(myTCPClient);
  myMQTTClient.setServer("test.mosquitto.org",1883);
  myMQTTClient.setCallback(dobbyCB);
  
  int mqttConnectResult = myMQTTClient.connect("DOBBY_DEVICE");
  Serial.printf("MQTT Connection Result : %d \r\n",mqttConnectResult);

  myMQTTClient.subscribe("MJU/IOT/60161797/#");
}

unsigned long long lastMs=0;
#define DELAY_MS  2000

void loop() {
  if(millis()-lastMs>DELAY_MS){
    lastMs=millis();
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

    x=(float)AcX/2048;
    y=(float)AcY/2048;
    z=(float)AcZ/2048;
  
    Serial.printf("if MQTTconnected? : %d \r\n",myMQTTClient.connected());
    
    char str[80];
    if(strcmp(tempbuffer, s1)==0){
        sprintf(str,"ACCEL : AX: %lf AY: %lf AZ: %lf \r\n",x,y,z);
        myMQTTClient.publish("MJU/IOTMQTT/kim",str);
    }
    else if(strcmp(tempbuffer, s2)==0){
    sprintf(str,"TEMP : %lf\r\n",Tmp/340.00+36.53);
    myMQTTClient.publish("MJU/IOTMQTT/kim",str);
    }
    else{
      sprintf(str,"TEMP : %lf\r\n",Tmp/340.00+36.53);
      myMQTTClient.publish("MJU/IOTMQTT/kim",str);
    }
  }
  myMQTTClient.loop();
}
