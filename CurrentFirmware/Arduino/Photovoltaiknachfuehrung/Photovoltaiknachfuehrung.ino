/*****************************************************************
   Anschlussplan
   
   IMU MPU-9250----- Arduino AIDE
   SCL ------------- A5
   SDA ------------- A4
   VDD ------------- 3V3
   GND ------------- GND

   L298N_1 --------- Arduino AIDE
   IN1 ------------- D5
   IN2 ------------- D4
   IN3 ------------- D3
   IN4 ------------- D2

   Servo_2 --------- Arduino AIDE
   Data ------------- D9
   
   Display ----- Arduino AIDE
   VCC ------------- +5V
   GND ------------- GND
   DIN ------------- D10
   CS -------------- D11
   CLK ------------- D12   
*****************************************************************/
// Bibliotheken
#include <Wire.h>
#include <SPI.h>
#include "I2Cdev.h"
#include "MPU9250.h"
#include "BMP280.h"
#include <Stepper.h>
#include <Servo.h>
#include "LedControl.h"

// Veränderbare Parameter
float SNull = 95;    //Winkel bei dem der Servoarm das PV-Modul auf 0° Höhenwinkel stellt
float SKorr = 1.2;  //Korrekturfaktor für Servo
int Hys = 5;        //Hysterese für Schrittmotor im IMU Modus

// Hardware und Variablen festlegen
Stepper stepper_1(200, 5, 4, 3, 2); //200 Schritte pro Umdrehung
Servo servo_2;
LedControl lc = LedControl(10,12,11,1);
MPU9250 accelgyro;
I2Cdev   I2C_M;
int Modus;
float Up;
float Pp;
int U;
int P;
int reg;
float azi = 180;  //Anfangswert für Azimut in Grad, 0 entspricht Norden
float gho = 0;   //Anfangswert für Höhenwinkel
float off1 = 0;   //Offset für Azimut
float off2 = 0;   //Offset für Höhenwinkel
float pitchdeg;
uint8_t buffer_m[6];
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
float heading;
float tiltheading;
float pitch;
float roll;
float Axyz[3];
float Gxyz[3];
float Mxyz[3];
#define sample_num_mdate  5000
volatile float mx_sample[3];
volatile float my_sample[3];
volatile float mz_sample[3];

//Kalibrierte Daten der IMU für Weingarten
static float mx_centre = 0.00;
static float my_centre = 13.00;
static float mz_centre = -7.00;

volatile int mx_max = 0;
volatile int my_max = 0;
volatile int mz_max = 0;
volatile int mx_min = 0;
volatile int my_min = 0;
volatile int mz_min = 0;
float temperature;
float pressure;
float atm;
float altitude;
BMP280 bmp280;

//Setup
void setup()
{
  Wire.begin(25); //Beginne I2C Übertragung und lege 25 (0x19 in Matlab) als Addresse fest
  Wire.setClock(100000); // 100kHz I2C clock. Comment this line if having compilation difficulties
  Wire.onReceive(getMatlab_Data);
  Serial.begin(115200);
  stepper_1.setSpeed(6);  //Geschwindigkeit in Umdrehungen pro Minute
  servo_2.attach(9);  //Signalleitung Servo ist D9
  lc.shutdown(0,false);
  lc.setIntensity(0,15);  //Helligkeit für Segmentanzeige festlegen, 15 ist max.
  lc.clearDisplay(0);     //Alle LEDs aus
    
  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  bmp280.init();
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");
  Serial.println("     ");
  //Mxyz_init_calibrated ();  //Auskommentieren um IMU zu kalibrieren
  
  Modus = digitalRead(7);    //Abfrage welcher Modus verwendet wird, mit oder ohne IMU

}
void loop()
{ 
    if (Modus == 0)     //Modus mit IMU
    {
    getMatlab_Data();
    getAccel_Data();
    getGyro_Data();
    getCompassDate_calibrated(); // compass data has been calibrated here
    getpitch();
    getroll();
    getHeading();               //before we use this function we should run 'getCompassDate_calibrated()' frist, so that we can get calibrated data ,then we can get correct angle .
    getTiltHeading();
    setStepper_1();
    setServo_2();
    readVoltages();
    convertVoltages();
    printVoltage();
    printPower();
    }
    else if (Modus == 1)  //Modus ohne IMU
    {
    getMatlab_Data();
    setStepper_2();
    setServo_2();
    readVoltages();
    convertVoltages();
    printVoltage();
    printPower();
    }
}
   
void getpitch(void) {
    pitch = asin(-Axyz[0]);
    pitchdeg = pitch * 180 / PI;
}

void getroll(void) {
    roll = asin(Axyz[1] / cos(pitch));
}

void getHeading(void) {
    heading = 180 * atan2(Mxyz[1], Mxyz[0]) / PI;
    if (heading < 0) heading += 360;
}

void getTiltHeading(void) {
    float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
    float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
    float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
    tiltheading = 180 * atan2(yh, xh) / PI;
    if (yh < 0)    tiltheading += 360;
    tiltheading = 360 - tiltheading;
}

void Mxyz_init_calibrated () {
    Serial.println(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
    Serial.print("  ");
    Serial.println(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
    Serial.print("  ");
    Serial.println(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
    while (!Serial.find("ready"));
    Serial.println("  ");
    Serial.println("ready");
    Serial.println("Sample starting......");
    Serial.println("waiting ......");

    get_calibration_Data ();

    Serial.println("     ");
    Serial.println("compass calibration parameter ");
    Serial.print(mx_centre);
    Serial.print("     ");
    Serial.print(my_centre);
    Serial.print("     ");
    Serial.println(mz_centre);
    Serial.println("    ");
}
void get_calibration_Data () {
    for (int i = 0; i < sample_num_mdate; i++)
    {
        get_one_sample_date_mxyz();
        
        Serial.print(mx_sample[2]);
        Serial.print(" ");
        Serial.print(my_sample[2]);                            //you can see the sample data here .
        Serial.print(" ");
        Serial.println(mz_sample[2]);
        

        if (mx_sample[2] >= mx_sample[1])mx_sample[1] = mx_sample[2];
        if (my_sample[2] >= my_sample[1])my_sample[1] = my_sample[2]; //find max value
        if (mz_sample[2] >= mz_sample[1])mz_sample[1] = mz_sample[2];

        if (mx_sample[2] <= mx_sample[0])mx_sample[0] = mx_sample[2];
        if (my_sample[2] <= my_sample[0])my_sample[0] = my_sample[2]; //find min value
        if (mz_sample[2] <= mz_sample[0])mz_sample[0] = mz_sample[2];

    }

    mx_max = mx_sample[1];
    my_max = my_sample[1];
    mz_max = mz_sample[1];

    mx_min = mx_sample[0];
    my_min = my_sample[0];
    mz_min = mz_sample[0];

    mx_centre = (mx_max + mx_min) / 2;
    my_centre = (my_max + my_min) / 2;
    mz_centre = (mz_max + mz_min) / 2;

}

void get_one_sample_date_mxyz() {
    getCompass_Data();
    mx_sample[2] = Mxyz[0];
    my_sample[2] = Mxyz[1];
    mz_sample[2] = Mxyz[2];
}

void getAccel_Data(void) {
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    Axyz[0] = (double) ax / 16384;
    Axyz[1] = (double) ay / 16384;
    Axyz[2] = (double) az / 16384;
}

void getGyro_Data(void) {
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    Gxyz[0] = (double) gx * 250 / 32768;
    Gxyz[1] = (double) gy * 250 / 32768;
    Gxyz[2] = (double) gz * 250 / 32768;
}

void getCompass_Data(void) {
    I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
    delay(10);
    I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);

    mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
    my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
    mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;

    Mxyz[0] = (double) mx * 1200 / 4096;
    Mxyz[1] = (double) my * 1200 / 4096;
    Mxyz[2] = (double) mz * 1200 / 4096;
}

void getCompassDate_calibrated () {
    getCompass_Data();
    Mxyz[0] = Mxyz[0] - mx_centre;
    Mxyz[1] = Mxyz[1] - my_centre;
    Mxyz[2] = Mxyz[2] - mz_centre;
}

void getMatlab_Data(void) {
  while(Wire.available())
  {
    reg = Wire.read();
    
    if (reg == 254)
    {
      azi = {Wire.read()};
      azi = 2 * azi;
    }
    else if (reg == 255)
    {
      gho = {Wire.read()};
    }
  }
}

void setStepper_1()
{
  float a = tiltheading + off1;
  if (azi > (a + Hys))          //+5 als Hysterese um ständiges wechseln zwischen zwei Stellungen zu vermeiden
    {stepper_1.step(1);
    off1 += 1.8;}               //Bei 200 Schritten pro Umdrehung entspricht ein Schritt 1.8°
  else if (azi < (a - Hys)) 
    {stepper_1.step(-1);
    off1 -= 1.8;}
}
void setStepper_2()
{
  if (azi > (off1 + 0.9 + 180))  //+180 um mit Südausrichtung zu starten
    {stepper_1.step(1);
    off1 += 1.8;}
  else if (azi < (off1 - 0.9 + 180)) //-0.9 als Hysterese
    {stepper_1.step(-1);
    off1 -= 1.8;}
  delay(60);    //Weil 1 loop ohne IMU kürzer ist, wird eine Pause benötigt, damit sich das Servo nicht zu schnell bewegt
}
void setServo_2()
{
  if (gho > off2)
    {off2 += 1;}
  else if (gho < off2) 
    {off2 -= 1;}
    servo_2.write(SNull + off2 * SKorr); //Servo auf Winkelabweichung + Höhenwinkel * Korrekturfaktor für ungenaues Servo einstellen
}
void readVoltages() {
  // read the input on analog pin 0:
  int u1 = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float U1 = u1 * (5.0 / 1023.0);

  // read the input on analog pin 1:
  int u2 = analogRead(A1);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float U2 = u2 * (5.0 / 1023.0);

  Up = (1 + 10000 / 2200) * U1; //Berechnung der Spannung in Volt des PV-Moduls über die Widerstände der Spannungsteilung im Messkreis mit 10 KOhm Plusseitig und 2,2 KOhm Masseseitig  Up = (1 + R2/R1) * U1

  Pp = Up * (U2 * 3); //Berechnung der Leistung in Watt des PV-Moduls über den 1/3 Ohm Widerstand (3 Parallele 1 Ohm Widerstände) im Verbraucherkreis Pp = Up * (U2/R3)
}
void convertVoltages() {
  U = round(Up);
  P = round(Pp);
}

void printVoltage() {  
    int ones;  
    int tens;  
    int hundreds; 

    boolean negative=false;

    if(U < -999 || U > 999)  
        return;  
    if(U<0) {  
        negative=true; 
        U=U*-1;  
    }
    ones=U%10;  
    U=U/10;  
    tens=U%10;  
    U=U/10; hundreds=U;  

    //LEDs für 'U=' anschalten
    lc.setLed(0,7,2,1);
    lc.setLed(0,7,3,1);
    lc.setLed(0,7,4,1);
    lc.setLed(0,7,5,1);
    lc.setLed(0,7,6,1);
    lc.setLed(0,6,4,1);
    lc.setLed(0,6,7,1);

    //LEDs für die Spannung in Volt anschalten
    lc.setDigit(0,5,(byte)tens,false); 
    lc.setDigit(0,4,(byte)ones,false); 
} 

void printPower() {  
    int ones;  
    int tens;  
    int hundreds; 

    boolean negative=false;

    if(P < -999 || P > 999)  
        return;  
    if(P<0) {  
        negative=true; 
        P=P*-1;  
    }
    ones=P%10;  
    P=P/10;  
    tens=P%10;  
    P=P/10; hundreds=P;  
    
    //LEDs für 'P=' anschalten
    lc.setChar(0,3,'P',false);
    lc.setLed(0,2,4,1);
    lc.setLed(0,2,7,1);
    
    //LEDs für die Leistung in Watt anschalten
    lc.setDigit(0,1,(byte)tens,false); 
    lc.setDigit(0,0,(byte)ones,false); 
}
