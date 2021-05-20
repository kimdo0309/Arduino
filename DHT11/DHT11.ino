#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

#define DHT11PIN 14

PubSubClient myMQTTClient;
HTTPClient myClient;
WiFiClient myTCPClient;

void setup() {
  Serial.begin(74880);
  delay(1000);

  WiFi.begin("wevo","dong0309");
  while(1)
  {
    if(WiFi.status()!=WL_CONNECTED)delay(500);
    else break;
  }
  Serial.printf("Connect...\r\n");
}

int readDHT11(int *readTemp, int* readHumid){
  int dt[82]={0,};
  //phase 1.
  digitalWrite(DHT11PIN,1);
  pinMode(DHT11PIN,OUTPUT);
  delay(1);
  digitalWrite(DHT11PIN,0);
  delay(20);
  pinMode(DHT11PIN, INPUT_PULLUP);

  while(1)
    if(digitalRead(DHT11PIN)==1)break;
  //Dataline '1'
  while(1)
    if(digitalRead(DHT11PIN)==0)break;

  //phase 2~ phase3
  int cnt=0;
  for(cnt=0;cnt<41;cnt++){
    dt[cnt*2]=micros();
    while(1)
      if(digitalRead(DHT11PIN)==1)break;
    dt[cnt*2]=micros()-dt[cnt*2];

    dt[cnt*2+1]=micros();
    while(1)
      if(digitalRead(DHT11PIN)==0)break;
    dt[cnt*2+1]=micros()-dt[cnt*2+1];
  }
  //phase 4.

  //{cnt==0 dt[cnt*2], dt[cnt*2+1] ==>phase 2. 80 us, 80us}.....
  *readHumid=0;
  *readTemp=0;
  for(cnt=1;cnt<9;cnt++)//Humid
  {
    *readHumid=*readHumid<<1;
    if(dt[cnt*2+1]>49)//->'1'
    {
      *readHumid=*readHumid+1;
    }
    else//->'0'
    {
      *readHumid=*readHumid+0;
    }
    //Serial.printf("cnt:%d,dt=[%d,%d]\r\n",cnt,dt[cnt*2],dt[cnt*2+1]);
  }
  for(cnt=17;cnt<25;cnt++)//Temp
  {
    *readTemp=*readTemp<<1;
    if(dt[cnt*2+1]>49)//->'1'
    {
      *readTemp=*readTemp+1;
    }
    else//->'0'
    {
      *readTemp=*readTemp+0;
    }
    //Serial.printf("cnt:%d,dt=[%d,%d]\r\n",cnt,dt[cnt*2],dt[cnt*2+1]);
  }
  //Serial.printf("Temp:%d\r\n",*readTemp);
  //Serial.printf("Humidity:%d\r\n",*readHumid);
  return 1;
}

unsigned long long prevMillis=0;

void loop() {
  int readTemp;
  int readHumid;

  if(millis()-prevMillis>60000){
    prevMillis=millis();

    readDHT11(&readTemp,&readHumid);
    char tempBuf[200];
    Serial.printf("Temp:%d\r\n",readTemp);
    snprintf(tempBuf,sizeof(tempBuf),"http://api.thingspeak.com/update?api_key=EZG3D79BKQRT5IAY&field1=%d",readTemp);
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
