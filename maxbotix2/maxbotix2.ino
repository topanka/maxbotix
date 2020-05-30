/*Feel free to use this code.
Author: Tom Bonar
Date: 12-11-2013
Analog pin 1 for reading in the analog voltage from the MaxSonar device.
This variable is a constant because the pin will not change throughout execution of this code.*/
#include <Servo.h>

//#define SERVO_MIN       850
//#define SERVO_MAX      2150
#define SERVO_MIN       1100
#define SERVO_MAX       1900

#define SERVO_CENTER   1500

#define SERVO_STEP_NORMAL      100
#define SERVO_STEP_FINE        25

const int anPinL = 0;
const int anPinR = 2;
const int triggerPin = 2;
long anVoltL, anVoltR;
Servo servo;  // create servo object to control a servo
int spos=SERVO_CENTER,sdir=1;
unsigned long start_sensor_time=0;
unsigned long start_servo_time=0;
unsigned long state_time=0;

int max_state=1;
int max_pos=0;
int max_len=0;

void setup(void)
{
  //This opens up a serial connection to shoot the results back to the PC console
  Serial.begin(115200);
  pinMode(triggerPin,OUTPUT);
  servo.attach(9);  // attaches the servo on pin 9 to the servo object
  servo.writeMicroseconds(SERVO_CENTER);                  // sets the servo position according to the scaled value
  delay(200); //Gives time for the sensors to boot and calibrate
}

int start_sensor(void)
{
  if(start_sensor_time > 0) return(0);
  if(start_servo_time > 0) return(0);
  
  digitalWrite(triggerPin,HIGH);
//  delay(1);
  delayMicroseconds(25);
  digitalWrite(triggerPin,LOW);
  start_sensor_time=millis();

/*
Serial.print(start_sensor_time);  
Serial.print(" ");  
Serial.println("sensor  start");  
*/

  return(1);
}

int read_sensor(void)
{
  if(start_sensor_time == 0) return(0);
  if((millis()-start_sensor_time) < 200) return(0);
  //Used to read in the analog voltage output that is being sent by the XL-MaxSonar device.
  //Scale factor is (Vcc/1024) per centimeter. A 5V supply yields ~4.9mV/cm for standard range sensors
  anVoltL = analogRead(anPinL);
  anVoltR = analogRead(anPinR);
  start_sensor_time=0;

/*
Serial.print(millis());  
Serial.print(" ");  
Serial.println("sensor  read");  
*/
  
  return(1);
}

void printall(void)
{
  Serial.print(spos);
  Serial.print(": ");
  Serial.print("sL");
  Serial.print("=");
  Serial.print(anVoltL);
  Serial.print("cm");
  Serial.print(" ");
  Serial.print("sR");
  Serial.print("=");
  Serial.print(anVoltR);
  Serial.print("cm");
  Serial.print(" L");
  Serial.print("=");
  Serial.print((anVoltR+anVoltL)/2);
  Serial.print("cm");
  Serial.println();
}

int servo_pos(int pos)
{
  if(start_servo_time > 0) {
    if((millis()-start_servo_time) >= 100) start_servo_time=0;
    return(0);
  }
  if(start_sensor_time > 0) return(0);
  
  if(spos > SERVO_MAX) spos=SERVO_MAX;
  else if(spos < SERVO_MIN) spos=SERVO_MIN;
  servo.writeMicroseconds(spos);
  start_servo_time=millis();

/*
Serial.print(start_servo_time);  
Serial.print(" ");  
Serial.println("servo pos");  
*/  
  return(0);
}

int findmax(int *state)
{
  int rs;
  long len;
  static int minp=SERVO_MIN,maxp=SERVO_MAX;
  static int sstep=SERVO_STEP_NORMAL;

  if((*state == 4) && (state_time > 0)) {
    if((millis()-state_time) > 5000) {
//      *state=1;
      state_time=0;
      Serial.println("restarting scan ...");
    }
  }
  
  if(*state == 1) {
    spos=minp;
    start_sensor_time=0;
    start_servo_time=0;
    servo_pos(spos);
    *state=2;
  } else if((*state == 2) || (*state == 3)) {
    start_sensor();
    rs=read_sensor();
    if(rs == 1) {
      printall();
      if(((spos == minp) && (*state == 2)) ||
         ((spos == maxp) && (*state == 3))) {
        max_pos=spos;
        max_len=(anVoltL+anVoltR)/2;
      } else {
        len=(anVoltL+anVoltR)/2;
        if(len > max_len) {
          max_len=len;
          max_pos=spos;
        }
      }
      if(((spos >= maxp) && (*state == 2)) ||
         ((spos <= minp) && (*state == 3))) {
        if(*state == 2) {
  Serial.print("maxpos=");
  Serial.println(max_pos);
          *state=3;
          minp=max_pos-sstep;
          maxp=max_pos+sstep;
          if(minp < SERVO_MIN) minp=SERVO_MIN;
          if(maxp > SERVO_MAX) maxp=SERVO_MAX;
          sstep=-SERVO_STEP_FINE;
          spos=maxp+SERVO_STEP_FINE;
        } else if(*state == 3) {
  Serial.print("*** maxpos fine=");
  Serial.println(max_pos);
        servo.writeMicroseconds(max_pos);
          *state=4;
          minp=SERVO_MIN;
          maxp=SERVO_MAX;
          sstep=SERVO_STEP_NORMAL;
          state_time=millis();
          return(1);
        }
      }
      spos+=sstep;
      if(spos > maxp) spos=maxp;
      else if(spos < minp) spos=minp;
    }
    servo_pos(spos);
  }  
  
  return(0);
}

void loop ()
{
  int rs;

//delay(1000);
//return;

  findmax(&max_state);

/*  
  start_sensor();
  rs=read_sensor();
  if(rs == 1) {
    printall();
    
    if(spos >= SERVO_MAX) {
      sdir*=-1;
//      delay(500);
    } else if(spos <= SERVO_MIN) {
      sdir*=-1;
//      delay(500);
    }
//    spos+=(sdir*50);
    spos+=(sdir*30);
    
  }
  servo_pos(spos);
*/


  
//  delay(200);

/*
  if(spos > SERVO_MAX) spos=SERVO_MAX;
  else if(spos < SERVO_MIN) spos=SERVO_MIN;
//  Serial.println(spos);
//  servo.writeMicroseconds(spos);                  // sets the servo position according to the scaled value
  delay(20); // This delay time changes by 100 for every sensor in the chain.  For 4 sensors this will be 400
  if(spos >= SERVO_MAX) {
    sdir*=-1;
    delay(500);
  } else if(spos <= SERVO_MIN) {
    sdir*=-1;
    delay(500);
  }
  spos+=(sdir*50);
*/  
}
