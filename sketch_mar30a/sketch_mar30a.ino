String mystring="";
void setup() {
  Serial.begin(9600);
}

void loop() {
  char my;
  while(Serial.available())
  {
    my=(char)Serial.read();
    mystring=mystring+my;
    delay(5);
  }
  if(!mystring.equals(""))
  {
    Serial.println(mystring);
    mystring="";
  }
}
