#include <Wire.h>

int MPU_Address=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float x,y,z;
int16_t who;

void setup() {
  Wire.begin(4,5);// GPIO 설정
  Wire.beginTransmission(MPU_Address);//주소값을 넣어준다
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);
  
  Serial.begin(115200);
  
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1B);
  Wire.write(0x18);//FS_SEL Full Scale Range= +-2000"/s
  Wire.endTransmission(true);
  
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1C);
  Wire.write(0x18);//Accel +-16g
  Wire.endTransmission(true);
  
  delay(1000);
  
  Serial.println("Hello World!");
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x75);
  Wire.endTransmission(true);
  Wire.requestFrom(MPU_Address,2,true);
  who=Wire.read();
  Serial.print("who am i ? : 0X");
  Serial.println(who,HEX);//만약 68이면 잘 접속된것이다.
  
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

  x=(float)AcX/2048;
  y=(float)AcY/2048;
  z=(float)AcZ/2048;

  Serial.print("AX = "); Serial.print(x); Serial.print("g");
  Serial.print(", AY = "); Serial.print(y); Serial.print("g");
  Serial.print(", AZ = "); Serial.print(z); Serial.print("g");
  Serial.print(", Tmp = "); Serial.print(Tmp/340.00+36.53);
  Serial.print(", GX = "); Serial.print(GyX);
  Serial.print(", GY = "); Serial.print(GyY);
  Serial.print(", GZ = "); Serial.println(GyZ);
  

  delay(1000);
}
