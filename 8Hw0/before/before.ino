#include <Wire.h>

int MPU_Address=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup() {
  Wire.begin(4,5);
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);
  
  Serial.begin(115200);
  
  delay(1000);
  Serial.println("Hello World!");
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

  Serial.print("AX = "); Serial.print(AcX);
  Serial.print(", AY = "); Serial.print(AcY);
  Serial.print(", AZ = "); Serial.print(AcZ);
  Serial.print(", Tmp = "); Serial.print(Tmp/340.00+36.53);
  Serial.print(", GX = "); Serial.print(GyX);
  Serial.print(", GY = "); Serial.print(GyY);
  Serial.print(", GZ = "); Serial.println(GyZ);
  

  delay(1000);
}
