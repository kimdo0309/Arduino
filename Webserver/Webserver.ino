#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

int sensorPin = D5;
ESP8266WebServer myHttpServer(80);
int ifMotorIsOn=0;
int motorPower = 1023;
int MPU_Address=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void fnRoot(void){
  char tmpBuffer[2000];
  char tmpbuffer[200];
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

void setup() {
  Serial.begin(74880);
  WiFi.begin("wevo","dong0309");
  while(1){
    if(WiFi.status()==WL_CONNECTED)
      break;
      delay(500);
  }
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

void loop() {
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

  myHttpServer.handleClient();
  
  delay(1000);
}
