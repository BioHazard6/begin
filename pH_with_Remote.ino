// include the library code:
#include <IRremote.h>
#include <IRremoteInt.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
OneWire  ds(7);  // on pin 7 (a 4.7K resistor is necessary)

#define SensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
float pH = 6.15;
long previousMillis = 0;
long temptime = 0;
long tempstart = 0;
long start = 0;
long timer = 0;
long wait = 0;
long waitinterval1 = 60000;
long waitinterval2 = 60000;
long waitinterval3 = 60000;
long smalldown = 500;
long largedown = 1000;
long smallup = 300;
long largeup = 800;
float deviation1 = 0.05;
float deviation2 = 0.25;
float deviation3 = 1.00;

int receiver = 9;

IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

/*-----( Function )-----*/
void translateIR() // takes action based on IR code received

// describing Remote IR codes 


{

  switch(results.value)

  {
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP"); break;
  case 0xFF629D: Serial.println("VOL+"); break;
  case 0xFF22DD: Serial.println("FAST BACK");    break;
  case 0xFF02FD: Serial.println("PAUSE");    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");  break;
  case 0xFFA857: Serial.println("VOL-");  break;
  case 0xFF906F: Serial.println("UP");    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0");    break;
  case 0xFF30CF: Serial.println("1");    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;  

  default: 
    Serial.println(" other button   ");

  }// End Case
}
void setup()
{
  pinMode(13,OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(8, OUTPUT); 
  Serial.begin(9600);
  Serial.println("Ready");
  lcd.begin(2, 16);
  irrecv.enableIRIn(); // Start the receiver
}
void loop()
{
  if (irrecv.decode(&results)){

        switch(results.value){
          case 0xFF906F: //Keypad button "up"
          digitalWrite(10, HIGH);
          delay(2000);
          digitalWrite(10, LOW);
          }

        switch(results.value){
          case 0xFFE01F: //Keypad button "down"
          digitalWrite(8, HIGH);
          delay(2000);
          digitalWrite(8, LOW);
          }

        switch(results.value){
         case 0xFFA25D: //Keypad button "power"
            digitalWrite(6, HIGH);
                    }
        
        irrecv.resume(); 
    }
 {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    //delay(250);
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    //Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
     //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  unsigned long currentTime = millis();
  if (currentTime - temptime > 1000){ //delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

 // Serial.print("  Data = ");
 // Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
   //Serial.print("  Temperature = ");
   //Serial.print(celsius);
   //Serial.print(" Celsius, ");
  //Serial.print(fahrenheit);
  //Serial.println(" Fahrenheit");
  }
  unsigned long currentMillis = millis();
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    if ((currentMillis - wait) > 10){     //delay(10);
      wait = currentMillis;
    }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
    }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  
if (currentMillis - start > 5000){  
  Serial.print("pH:");  
  Serial.println(phValue,2);
  //Serial.print(" ");
  digitalWrite(13, HIGH);       
  //delay(500);
  digitalWrite(13, LOW); 
  Serial.print("Temperature:");
  Serial.print(celsius);
  Serial.println(" Celsius, ");
  lcd.setCursor(0, 0);
  lcd.print("pH:");  
  lcd.print(phValue,2);
  lcd.setCursor(0, 1);
  lcd.print("Temperature:");
  lcd.print(celsius);
  start = currentMillis;
}
if (currentMillis - previousMillis > waitinterval1){                              //if pH is within 0.05 pH. Check this every 15s
       if (((phValue) < (pH + deviation2)) && ((phValue) > (pH + deviation1))){
         Serial.println("Small Down");
         digitalWrite(8, HIGH);
         previousMillis = (currentMillis - 2200);
       }
       if (((phValue) > (pH - deviation2)) && ((phValue) < (pH - deviation1))){
         Serial.println("Small up");
         digitalWrite(10, HIGH);
         previousMillis = (currentMillis - 2200);
        }
}
if (currentMillis - previousMillis > waitinterval2){                             //if pH is within 0.25 pH. Check this every 30s
      if (((phValue) >= (pH + deviation2)) && ((phValue) < (pH + deviation3))){
         Serial.println("Medium Down");
         digitalWrite(8, HIGH);
         previousMillis = (currentMillis - 1500);
        }  
      if (((phValue) <= (pH - deviation2)) && ((phValue) > (pH - deviation3))){
         Serial.println("Medium up");
         digitalWrite(10, HIGH);
         previousMillis = (currentMillis - 1500);
        } 
}
if (currentMillis - previousMillis > waitinterval3){                          //if pH is within 1.00 pH. Check this every 45s
      if ((phValue) >= (pH + deviation3)){
         Serial.println("Large Down");
         digitalWrite(8, HIGH);
         previousMillis = (currentMillis - 000);
        }  
      if ((phValue) <= (pH - deviation3)){
         Serial.println("Large up");
         digitalWrite(10, HIGH);
         previousMillis = (currentMillis - 000);
        } 
}

if ((currentMillis - previousMillis >= 3000) && digitalRead(10 == HIGH)){
         digitalWrite(10, LOW); 
        }    
if ((currentMillis - previousMillis >= 3000) && digitalRead(8 == HIGH)){
         digitalWrite(8, LOW);
}
}
}
