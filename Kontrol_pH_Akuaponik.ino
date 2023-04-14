 //Library Firebase ESP32
#if defined(ESP32)
#include <WiFi.h> 
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

//Library BluetoothSerial
#include "BluetoothSerial.h" 

//Library LCD
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

//Library RTC DS3231
#include "RTClib.h"

//========== F I R E B A S E  C O N F I G U R A T I O N ===========
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/* 1. Define the WiFi credentials */
//#define WIFI_SSID "RIOOBEKTI"
//#define WIFI_PASSWORD "pe13no31"
#define WIFI_SSID "Angel wes"
#define WIFI_PASSWORD "americano"

#define API_KEY "API_KEY"

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "https://akuaponik-c8f3c-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "02X3nZkVlOo80l6mIf45V2dfE9ekBd0RODnGfQhc"

/* 3. Define the Firebase Data object */
FirebaseData databaseAkuaponik;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

//========== R T C  C O N F I G U R A T I O N ===========
//Declare Time
char hari[7][20] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
int tanggal, bulan, tahun, jam, menit, detik;
String dow; //days of week
String MyDate, MyTime;

//Mendefinisikan RTC DS3231
RTC_DS3231 rtc;

//========== L C D  C O N F I G U R A T I O N ===========
/* Declare LCD object for SPI
 Adafruit_PCD8544(CLK,DIN,D/C,CE,RST); */
Adafruit_PCD8544 display = Adafruit_PCD8544(18, 23, 4, 15, 2);

//========== B L U E T O O T H  C O N F I G U R A T I O N ===========
BluetoothSerial ESP_BT; //Object for Bluetooth

//========== V A R I A B E L ===========
//Time variable
unsigned long currentTime;
unsigned long currentRead;
unsigned long previousRead = 0;
unsigned long previousPump = 0;
unsigned long myTimer;
unsigned long myTimerDisplay;
const long timerReadSensor = 30000;
const long timeSampling = 50;
const long timeDisplay = 1000;

//Pump variable
String sistem;
String namaTanaman;
int pumpSatu = 0;
int pumpDua = 0;
int stateTimer = 0;
String statusPompa1;
String statusPompa2;
int pinRelay1 = 26; //Pompa Up
int pinRelay2 = 13; //Pompa Down

//pH variable
float koefisien1 = -4.539;
float koefisien2 = -4.0981;
float konstanta1 = 17.676;
float konstanta2 = 17.162;
float pH1 = 0;
float pH2 = 0;

//========== F U Z Z Y   V A R I A B E L ============
//Variabel Set Point
float setPoint1 = 0; 
float setPointAtas = 0;
float setPointBawah = 0;

//Variabel Nilai Linguistik
float sangat_asam1[4]={0, 0, 4, 5};
float asam1[4]={4, 5, 6, 7};
float stabil1[4]={6, 7, 7, 8};
float basa1[4]={7, 8, 9, 10};
float sangat_basa1[4]={9, 10, 14, 14};
float sangat_asam2[4]={0, 0, 4, 5};
float asam2[4]={4, 5, 6, 7};
float stabil2[4]={6, 7, 7, 8};
float basa2[4]={7, 8, 9, 10};
float sangat_basa2[4]={9, 10, 14, 14};
float mati1[4]={0, 0, 0, 0};
float cukup_lama1[4]={0, 1, 4, 5};
float lama1[4]={4, 5, 9, 10};
float lebih_lama1[4]={9, 10, 14, 15};
float sangat_lama1[4]={14, 15, 20, 20};
float mati2[4]={0, 0, 0, 0};
float cukup_lama2[4]={0, 1, 4, 5};
float lama2[4]={4, 5, 9, 10};
float lebih_lama2[4]={9, 10, 14, 15};
float sangat_lama2[4]={14, 15, 20, 20};
float sa1,a1,st1,b1,sb1,sa2,a2,st2,b2,sb2;

//Variabel Fuzzyfikasi
float input1 = 0;
float input2 = 0;

//Variabel Inferensi
float terbesarA, terbesarB, terbesarC, terbesarD, terbesarE, terbesarF, terbesarG, terbesarH, terbesarI, terbesarJ;

//Variabel Defuzzyfikasi
float sampel1, sampel2, sampel3, sampel4, sampel5, sampel6, sampel7, sampel8, sampel9;
float pengaliA, pengaliB, pengaliC, pengaliD, pengaliE, pengaliF, pengaliG, pengaliH, pengaliI;
float titik_sampelA, titik_sampelB, titik_sampelC, titik_sampelD, titik_sampelE, titik_sampelF, titik_sampelG, titik_sampelH, titik_sampelI;
float hasilPembilang1, hasilPenyebut1, hasilPembilang2, hasilPenyebut2;
float jumlah_sampel1, jumlah_sampel2, jumlah_sampel3, jumlah_sampel4, jumlah_sampel5, jumlah_sampel6, jumlah_sampel7, jumlah_sampel8, jumlah_sampel9;
float hasilDefuzzyfikasi1;
float hasilDefuzzyfikasi2;
float timer1;
float timer2;

//========== G E T   S E T   P O I N T ============
//Input 1 Set Point
void setPoint(){
  if(Firebase.getString(databaseAkuaponik, "1 Set Point/Input/setPoint1")){
    String set1Point = databaseAkuaponik.stringData();
    char set1[set1Point.length()];
    set1Point.toCharArray(set1,set1Point.length()+1);
    float input1SetPoint = atof(set1);
    if((stabil1[1] && stabil1[2] && stabil2[1] && stabil2[2]) != input1SetPoint){
      if((input1SetPoint >= stabil1[0] && stabil2[0]) && (input1SetPoint <= stabil1[3] && stabil2[3])){
        Serial.printf("Set Point 1 = ");
        Serial.println(input1SetPoint);
        stabil1[1] = input1SetPoint;
        stabil1[2] = input1SetPoint;
        stabil2[1] = input1SetPoint;
        stabil2[2] = input1SetPoint;
        asam1[3] = input1SetPoint;
        asam2[3] = input1SetPoint;
        basa1[0] = input1SetPoint;
        basa2[0] = input1SetPoint;
        Serial.println("MF Asam1[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);
      }
      else if(input1SetPoint < stabil1[0] && stabil2[0]){
        input1SetPoint = 6;
        Serial.printf("Set Point 1 = ");
        Serial.println(input1SetPoint);
        float asam1[4] = {4,5,6,6};
        float asam2[4] = {4,5,6,6};
        float stabil1[4] = {6,6,6,8};
        float stabil2[4] = {6,6,6,8};
        float basa1[4] = {6,8,9,10};
        float basa2[4] = {6,8,9,10};
        Serial.println("MF Asam1[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);  
      }
      else if(input1SetPoint > stabil1[3] && stabil2[3]){
        input1SetPoint = 8;
        Serial.printf("Set Point 1 = ");
        Serial.println(input1SetPoint);
        float asam1[4] = {4,5,6,8};
        float asam2[4] = {4,5,6,8};
        float stabil1[4] = {6,8,8,8};
        float stabil2[4] = {6,8,8,8};
        float basa1[4] = {8,8,9,10};
        float basa2[4] = {8,8,9,10};
        Serial.println("MF Asam1[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);
      }
    }
  }
  else {
    setPoint2();
  }
}

//Input 2 Set Point
void setPoint2(){
  if(Firebase.getString(databaseAkuaponik, "2 Set Point/Input/setPoint2asam")){
    String setPoint2asam = databaseAkuaponik.stringData();
    char set2a[setPoint2asam.length()];
    setPoint2asam.toCharArray(set2a,setPoint2asam.length()+1);
    float inputSetPoint2a = atof(set2a);
    if((stabil1[1] && stabil2[1]) != inputSetPoint2a){
      if((inputSetPoint2a >= stabil1[0] && stabil2[0]) && (inputSetPoint2a <= stabil1[3] && stabil2[3])){
        Serial.printf("Set Point 2 asam = ");
        Serial.println(inputSetPoint2a);
        stabil1[1] = inputSetPoint2a;
        stabil2[1] = inputSetPoint2a;
        asam1[3] = inputSetPoint2a;
        asam2[3] = inputSetPoint2a;
        Serial.println("MF Asam[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
      }
      else if(inputSetPoint2a < stabil1[0] && stabil2[0]){
        inputSetPoint2a = 6;
        Serial.printf("Set Point 2 asam = ");
        Serial.println(inputSetPoint2a);
        float asam1[4] = {4,5,6,6};
        float asam2[4] = {4,5,6,6};
        stabil1[0] = 6;
        stabil2[0] = 6;
        stabil1[1] = 6;
        stabil2[1] = 6;
        Serial.println("MF Asam[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]); 
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
      }
      else if(inputSetPoint2a > stabil1[3] && stabil2[3]){
        inputSetPoint2a = 8;
        Serial.printf("Set Point 2 asam = ");
        Serial.println(inputSetPoint2a);
        float asam1[4] = {4,5,6,8};
        float asam2[4] = {4,5,6,8};
        stabil1[0] = 8;
        stabil2[0] = 8;
        stabil1[1] = 8;
        stabil2[1] = 8;
        Serial.println("MF Asam[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]); 
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
      }
    }
  }
  if(Firebase.getString(databaseAkuaponik, "2 Set Point/Input/setPoint2basa")){
    String setPoint2basa = databaseAkuaponik.stringData();
    char set2b[setPoint2basa.length()];
    setPoint2basa.toCharArray(set2b,setPoint2basa.length()+1);
    float inputSetPoint2b = atof(set2b);
    if((stabil1[2] && stabil2[2]) != inputSetPoint2b){
      if((inputSetPoint2b >= stabil1[0] && stabil2[0]) && (inputSetPoint2b <= stabil1[3] && stabil2[3])){
        Serial.printf("Set Point 2 basa = ");
        Serial.println(inputSetPoint2b);
        stabil1[2] = inputSetPoint2b;
        stabil2[2] = inputSetPoint2b;
        basa1[0] = inputSetPoint2b;
        basa2[0] = inputSetPoint2b;
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);
      }
      else if(inputSetPoint2b < stabil1[0] && stabil2[0]){
        inputSetPoint2b = 6;
        Serial.printf("Set Point 2 basa = ");
        Serial.println(inputSetPoint2b);
        stabil1[2] = 6;
        stabil2[2] = 6;
        stabil1[3] = 6;
        stabil2[3] = 6;
        float basa1[4] = {6,8,9,10};
        float basa2[4] = {6,8,9,10};
        Serial.println("MF Asam[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);
      }
      else if(inputSetPoint2b > stabil1[3] && stabil2[3]){
        inputSetPoint2b = 8;
        Serial.printf("Set Point 2 basa = ");
        Serial.println(inputSetPoint2b);
        stabil1[2] = 8;
        stabil2[2] = 8;
        stabil1[3] = 8;
        stabil2[3] = 8;
        float basa1[4] = {8,8,9,10};
        float basa2[4] = {8,8,9,10};
        Serial.println("MF Asam[4]");
        Serial.println(asam1[0]);
        Serial.println(asam1[1]);
        Serial.println(asam1[2]);
        Serial.println(asam1[3]);
        Serial.println("MF Asam2[4]");
        Serial.println(asam2[0]);
        Serial.println(asam2[1]);
        Serial.println(asam2[2]);
        Serial.println(asam2[3]);
        Serial.println("MF Stabil1[4]");
        Serial.println(stabil1[0]);
        Serial.println(stabil1[1]);
        Serial.println(stabil1[2]);
        Serial.println(stabil1[3]);
        Serial.println("MF Stabil2[4]");
        Serial.println(stabil2[0]);
        Serial.println(stabil2[1]);
        Serial.println(stabil2[2]);
        Serial.println(stabil2[3]);
        Serial.println("MF Basa1[4]");
        Serial.println(basa1[0]);
        Serial.println(basa1[1]);
        Serial.println(basa1[2]);
        Serial.println(basa1[3]);
        Serial.println("MF Basa2[4]");
        Serial.println(basa2[0]);
        Serial.println(basa2[1]);
        Serial.println(basa2[2]);
        Serial.println(basa2[3]);
      }
    } 
  }  
  else {
    float stabil1[4]={6, 7, 7, 8};
    float stabil2[4]={6, 7, 7, 8};
    Serial.print("MF Stabil1[4]");
    Serial.println(stabil1[0]);
    Serial.println(stabil1[1]);
    Serial.println(stabil1[2]);
    Serial.println(stabil1[3]);
    Serial.print("MF Stabil2[4]");
    Serial.println(stabil2[0]);
    Serial.println(stabil2[1]);
    Serial.println(stabil2[2]);
    Serial.println(stabil2[3]);
  }
}

//========== G E T   T A N A M A N ============
void getTanaman(){
  if(Firebase.getString(databaseAkuaponik, "1 Set Point/Input/setTanaman1")){
    namaTanaman = databaseAkuaponik.stringData();
    Serial.printf("Nama Tanaman : ");
    Serial.println(namaTanaman);
  }
  else if(Firebase.getString(databaseAkuaponik, "2 Set Point/Input/setTanaman2")){
    namaTanaman = databaseAkuaponik.stringData();
    Serial.printf("Nama Tanaman : ");
    Serial.println(namaTanaman);
  }
  else{
    namaTanaman = "Belum Diinput";
    Serial.printf("Nama Tanaman : ");
    Serial.println(namaTanaman);
  }
}

//========== B A C A   S E N S O R ============
void bacaSensor(){
  //Read ADC Value Sensor 1
  int pinADC1 = 0;
  int pinADC2 = 0;
  int looping = 10;
  for(int i =0; i<looping; i++){
    pinADC1 += analogRead(32);
    delay(10);
  }
  int avg1 = pinADC1 / looping;
  float vol_adc1 = avg1 * (3.3 / 4095.0);
  
  //Read ADC Value Sensor 2
  for(int j=0; j<looping; j++){
    pinADC2 += analogRead(33);
    delay(10);
  }
  int avg2 = pinADC2 / looping;
  float vol_adc2 = avg2 * (3.3 / 4095.0);

  //pH Value
  pH1 = (vol_adc1 * koefisien1) + konstanta1;
  pH2 = (vol_adc2 * koefisien2) + konstanta2;
//  pH1 = Serial.parseFloat();
//  pH2 = Serial.parseFloat();
//  pH1 = 7.3;
//  pH2 = 7.5;
    
  //Serial Monitor
  Serial.printf("ADC1 : ");
  Serial.println(avg1);
  Serial.printf("Voltage1 : ");
  Serial.println(vol_adc1,5);
  Serial.printf("ADC2 : ");
  Serial.println(avg2);
  Serial.printf("Voltage2 : ");
  Serial.println(vol_adc2,5);
  Serial.println("Sensor Mendeteksi");
  Serial.printf("Nilai Sensor pH 1 : ");
  Serial.println(pH1);
  if(Firebase.setFloat(databaseAkuaponik, "Sensor pH/Input1", pH1)){
    Serial.println("Nilai pH 1 Terkirim");
    }
  else{
    Serial.println("Nilai pH 1 tidak terkirim");
    Serial.println("Karena: " + databaseAkuaponik.errorReason());
    }
  Serial.printf("Nilai Sensor pH 2 : ");
  Serial.println(pH2);
  if(Firebase.setFloat(databaseAkuaponik, "Sensor pH/Input2", pH2)){
    Serial.println("Nilai pH 2 Terkirim");
    }
  else{
    Serial.println("Nilai pH 2 tidak terkirim");
    Serial.println("Karena: " + databaseAkuaponik.errorReason());
  }
  //Send data sensor to bluetooth serial
  ESP_BT.print(";");
  ESP_BT.print(pH1);
  ESP_BT.print(";");
  ESP_BT.print(pH2);
}

//========== S T A T U S   S I S T E M ============
void statusSistem(){
  if((pH1 <= stabil1[2]) && (pH1 >= stabil1[1]) && (pH2 <= stabil2[2]) && (pH2 >= stabil2[1])){
    sistem = "Stabil";
    Serial.printf("Status Sistem : ");
    Serial.println(sistem);
    if(sistem == "Stabil"){
      if(Firebase.setString(databaseAkuaponik, "Status Sistem/Status", sistem)){
        Serial.println("Status Sistem Terkirim");
        }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }  
    }
  }
  else{
    sistem = "Belum Stabil";
    Serial.printf("Status Sistem : ");
    Serial.println(sistem);
    if(sistem == "Belum Stabil"){
      if(Firebase.setString(databaseAkuaponik, "Status Sistem/Status", sistem)){
        Serial.println("Status Sistem Terkirim");
        }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }  
    }  
  }
}

//========== F U Z Z Y   L O G I C ============
//Membershpi function input sensor
void fzinput(){
  //Inisiasi awal nilai tiap membership function sama dengan nol
  sa1 = 0;
  a1 = 0;
  st1 = 0;
  b1 = 0;
  sb1 = 0;
  sa2 = 0;
  a2 = 0;
  st2 = 0;
  b2 = 0;
  sb2 = 0;
  
  //Membership function input sensor pH 1
  //sangat_asam1
  if(input1>=sangat_asam1[1]&& input1<=sangat_asam1[2]){
    sa1 = 1;
  }
  else if(input1>sangat_asam1[2] && input1<sangat_asam1[3]){
    sa1 = (float)((sangat_asam1[3]-input1)/(sangat_asam1[3]-sangat_asam1[2]));
  }
  else if(input1<sangat_asam1[0] && input1>sangat_asam1[3]){
    sa1 = 0;
  }

  //asam1
  if(input1 == asam1[2] && input1 == stabil1[1]){
    a1 = 0;
  }
  else{
    if(input1>=asam1[1] && input1<=asam1[2]){
    a1 = 1;
    }
    else if(input1>asam1[0] && input1<asam1[1]){
      a1 = (float)((input1-asam1[0])/(asam1[1]-asam1[0]));
    }
    else if(input1<asam1[3] && input1>asam1[2]){
      a1 = (float)((asam1[3]-input1)/(asam1[3]-asam1[2]));
    }
    else if(input1<asam1[0] && input1>asam1[3]){
      a1 = 0;
    }  
  }
  
  //stabil1
  if((input1 == asam1[2] && input1 == stabil1[1]) || (input1 == stabil1[2] && input1 == basa1[1])){
    st1 = 1;    
  }
  else{
    if(input1>=stabil1[1] && input1<=stabil1[2]){
      st1 = 1;
    }
    else if(input1>stabil1[0] && input1<stabil1[1]){
      st1 = (float)((input1-stabil1[0])/(stabil1[1]-stabil1[0]));
    }
    else if(input1<stabil1[3] && input1>stabil1[2]){
      st1 = (float)((stabil1[3]-input1)/(stabil1[3]-stabil1[2]));
    }
    else if(input1<stabil1[0] && input1>stabil1[3]){
      st1 = 0;
    }  
  }
  
  //basa1
  if(input1 == stabil1[2] && input1 == basa1[1]){
    b1 = 0;
  }
  else{
    if(input1>=basa1[1] && input1<=basa1[2]){
      b1 = 1;
    }
    else if(input1>basa1[0] && input1<basa1[1]){
      b1 = (float)((input1-basa1[0])/(basa1[1]-basa1[0]));
    }
    else if(input1<basa1[3] && input1>basa1[2]){
      b1 = (float)((basa1[3]-input1)/(basa1[3]-basa1[2]));
    }
    else if(input1<basa1[0] && input1>basa1[3]){
      b1 = 0;
    }  
  }

  //sangat_basa1
  if(input1>=sangat_basa1[1] && input1<=sangat_basa1[2]){
    sb1 = 1;
  }
  else if(input1>sangat_basa1[0] && input1<sangat_basa1[1]){
    sb1 = (float)((input1-sangat_basa1[0])/(sangat_basa1[1]-sangat_basa1[0]));
  }
  else if(input1<sangat_basa1[3] && input1>sangat_basa1[2]){
    sb1 = (float)((sangat_basa1[3]-input1)/(sangat_basa1[3]-sangat_basa1[2]));
  }
  else if(input1<sangat_basa1[0] && input1>sangat_basa1[3]){
    sb1 = 0;
  }

  //Membership function input sensor pH 2
  //sangat_asam2
  if(input2>=sangat_asam2[1]&& input2<=sangat_asam2[2]){
    sa2 = 1;
  }
  else if(input2>sangat_asam2[2] && input2<sangat_asam2[3]){
    sa2 = (float)((sangat_asam2[3]-input2)/(sangat_asam2[3]-sangat_asam2[2]));
  }
  else if(input2<sangat_asam2[0] && input2>sangat_asam2[3]){
    sa2 = 0;
  }

  //asam2
  if(input2 == asam2[2] && input2 == stabil2[1]){
    a2 = 0;
  }
  else{
    if(input2>=asam1[1] && input2<=asam2[2]){
      a2 = 1;
    }
    else if(input2>asam2[0] && input2<asam2[1]){
      a2 = (float)((input2-asam2[0])/(asam2[1]-asam2[0]));
    }
    else if(input2<asam2[3] && input2>asam2[2]){
      a2 = (float)((asam2[3]-input2)/(asam2[3]-asam2[2]));
    }
    else if(input2<asam1[0] && input2>asam2[3]){
      a2 = 0;
    }  
  }
  
  //stabil2
  if((input2 == asam2[2] && input2 == stabil2[1]) || (input2 == stabil2[2] && input2 == basa2[1])){
    st2 = 1;    
  }
  else{
    if(input2>=stabil2[1] && input2<=stabil2[2]){
      st2 = 1;
    }
    else if(input2>stabil2[0] && input2<stabil2[1]){
      st2 = (float)((input2-stabil2[0])/(stabil2[1]-stabil2[0]));
    }
    else if(input2<stabil2[3] && input2>stabil2[2]){
      st2 = (float)((stabil2[3]-input2)/(stabil2[3]-stabil2[2]));
    }
    else if(input2<stabil2[0] && input2>stabil2[3]){
      st2 = 0;
    }  
  }
  
  //basa2
  if(input2 == stabil2[2] && input2 == basa2[1]){
    b2 = 0;
  }
  else{
    if(input2>=basa2[1] && input2<=basa2[2]){
      b2 = 1;
    }
    else if(input2>basa2[0] && input2<basa2[1]){
      b2 = (float)((input2-basa2[0])/(basa2[1]-basa2[0]));
    }
    else if(input2<basa2[3] && input2>basa2[2]){
      b2 = (float)((basa2[3]-input2)/(basa2[3]-basa2[2]));
    }
    else if(input2<basa2[0] && input2>basa2[3]){
      b2 = 0;
    }
  }
  
  //sangat_basa2
  if(input2>=sangat_basa2[1] && input2<=sangat_basa2[2]){
    sb2 = 1;
  }
  else if(input2>sangat_basa2[0] && input2<sangat_basa2[1]){
    sb2 = (float)((input2-sangat_basa2[0])/(sangat_basa2[1]-sangat_basa2[0]));
  }
  else if(input2<sangat_basa2[3] && input1>sangat_basa2[2]){
    sb2 = (float)((sangat_basa2[3]-input2)/(sangat_basa2[3]-sangat_basa2[2]));
  }
  else if(input2<sangat_basa2[0] && input2>sangat_basa2[3]){
    sb2 = 0;
  }
}

//Fuzzyfikasi
void fuzzyfikasi(){
  input1 = pH1; //Inisialisasi input sensor 1
  input2 = pH2; //Inisialisasi input sensor 2
  fzinput();
  Serial.printf("Input pH 1 : ");
  Serial.println(input1);
  Serial.printf("Sangat Asam 1 : ");
  Serial.println(sa1);
  Serial.printf("Asam 1 : ");
  Serial.println(a1);
  Serial.printf("Stabil 1 : ");
  Serial.println(st1);
  Serial.printf("Basa 1 : ");
  Serial.println(b1);
  Serial.printf("Sangat Basa 1 : ");
  Serial.println(sb1);
  Serial.printf("Input pH 2 : ");
  Serial.println(input2);
  Serial.printf("Sangat Asam 2 : ");
  Serial.println(sa2);
  Serial.printf("Asam 2 : ");
  Serial.println(a2);
  Serial.printf("Stabil 2 : ");
  Serial.println(st2);
  Serial.printf("Basa 2 : ");
  Serial.println(b2);
  Serial.printf("Sangat Basa 2 : ");
  Serial.println(sb2);
}

//Inferensi
void inferensi(){
  //Inisiasi awal nilai tiap max value sama dengan nol
  terbesarA = 0; 
  terbesarB = 0;
  terbesarC = 0;
  terbesarD = 0;
  terbesarE = 0;
  terbesarF = 0;
  terbesarG = 0;
  terbesarH = 0;
  terbesarI = 0;
  terbesarJ = 0;
  int k;
  float nilaiKondisi[36];
  String kondisiOut1[36];
  String kondisiOut2[36];
  String kondisipH1[36];
  String kondisipH2[36];
  float nilaipH1[5] = {sa1,a1,st1,b1,sb1};
  float nilaipH2[5] = {sa2,a2,st2,b2,sb2};
  k = 0;

  //rules dan nilai minimum
  for(int i=0; i<5; i++){ //i = untuk sensor pH 1
    for(int j=0; j<5; j++){ //j = untuk sensor pH 2
      if((nilaipH1[i]>=0) && (nilaipH2[j]>=0)){
        //Nilai Minimal tiap kondisi
        nilaiKondisi[k]=nilaipH1[i];
        if(nilaipH2[j] < nilaiKondisi[k]){
          nilaiKondisi[k]=nilaipH2[j];
        }

        //Menenetukan nilai kondisi input1
        if(i==0){
          kondisipH1[k]="Sangat Asam";
        }
        else if(i==1){
          kondisipH1[k]="Asam";
        }
        else if(i==2){
          kondisipH1[k]="Stabil";
        }
        else if(i==3){
          kondisipH1[k]="Basa";
        }
        else if(i==4){
          kondisipH1[k]="Sangat Basa";
        }

        //Menentukan Nilai Kondisi Input 2
        if(j==0){
          kondisipH2[k]="Sangat Asam";
        }
        else if(j==1){
          kondisipH2[k]="Asam";
        }
        else if(j==2){
          kondisipH2[k]="Stabil";
        }
        else if(j==3){
          kondisipH2[k]="Basa";
        }
        else if(j==4){
          kondisipH2[k]="Sangat Basa";
        }

        //Penentuan Rules Output Sangat Asam
        if((i == 0) && (j == 0)){
          kondisiOut1[k] = "Sangat Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 0) && (j == 1)){
          kondisiOut1[k] = "Lebih Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 0) && (j == 2)){
          kondisiOut1[k] = "Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 0) && (j == 3)){
          kondisiOut1[k] = "Cukup Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 0) && (j == 4)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Sangat Lama Down";
        }
        
        //Penentuan Rules Output Asam
        else if((i == 1) && (j == 0)){
          kondisiOut1[k] = "Lebih Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 1) && (j == 1)){
          kondisiOut1[k] = "Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 1) && (j == 2)){
          kondisiOut1[k] = "Cukup Lama Up";
          kondisiOut2[k] = "Mati";
        }
       else if((i == 1) && (j == 3)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Cukup Lama Down";
        }
        else if((i == 1) && (j == 4)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Cukup Lama Down";
        }
        
        //Penentuan Rules Output Stabil
        else if((i == 2) && (j == 0)){
          kondisiOut1[k] = "Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 2) && (j == 1)){
          kondisiOut1[k] = "Cukup Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 2) && (j == 2)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 2) && (j == 3)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Cukup Lama Down";
        }
        else if((i == 2) && (j == 4)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Lama Down";
        }
        
        //Penentuan Rules Output Basa
        else if((i == 3) && (j == 0)){
          kondisiOut1[k] = "Cukup Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 3) && (j == 1)){
          kondisiOut1[k] = "Cukup Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 3) && (j == 2)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Cukup Lama Down";
        }
        else if((i == 3) && (j == 3)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Lama Down";
        }
        else if((i == 3) && (j == 4)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Lebih Lama Down";
        }
        
        //Penentuan Rules Output Sangat Basa
        else if((i == 4) && (j == 0)){
          kondisiOut1[k] = "Sangat Lama Up";
          kondisiOut2[k] = "Mati";
        }
        else if((i == 4) && (j == 1)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Cukup Lama Down";
        }
        else if((i == 4) && (j == 2)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Lama Down";
        }
        else if((i == 4) && (j == 3)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Lebih Lama Down";
        }
        else if((i == 4) && (j == 4)){
          kondisiOut1[k] = "Mati";
          kondisiOut2[k] = "Sangat Lama Down";
        }

      Serial.printf("Ketika Input pH1");
      Serial.println(kondisipH1[k]);
      Serial.printf("Fuzzy Input 1 : ");
      Serial.println(nilaipH1[i]);
      
      Serial.printf("Ketika Input pH2");
      Serial.println(kondisipH2[k]);
      Serial.printf("Nilai Fuzzy Input 2 : ");
      Serial.println(nilaipH2[j]);
      
      Serial.printf("Output Pompa Up : ");
      Serial.println(kondisiOut1[k]);
      Serial.printf("Output Pompa Down : ");
      Serial.println(kondisiOut2[k]);
      Serial.printf("Nilai Fuzzy Output : ");
      Serial.println(nilaiKondisi[k]);
      k++;
      }
    }
  }

  //Menentukan nilai max
  for(int i=0; i<k; i++){
    if((kondisiOut1[i]=="Sangat Lama Up") && (kondisiOut2[i]=="Mati")){
      if(i==0){
        terbesarA=nilaiKondisi[i];
      }
      else{
        if(terbesarA<nilaiKondisi[i]){
          terbesarA=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Lebih Lama Up") && (kondisiOut2[i]=="Mati")){
      if(i==0){
        terbesarB=nilaiKondisi[i];
        }
      else{
        if(terbesarB<nilaiKondisi[i]){
           terbesarB=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Lama Up") && (kondisiOut2[i]=="Mati")){
      if(i==0){
        terbesarC=nilaiKondisi[i];
        }
      else{
        if(terbesarC<nilaiKondisi[i]){
          terbesarC=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Cukup Lama Up") && (kondisiOut2[i]=="Mati")){
      if(i==0){
        terbesarD=nilaiKondisi[i];
      }
      else{
        if(terbesarD<nilaiKondisi[i]){
          terbesarD=nilaiKondisi[i];
        }
      }
    }  
    else if((kondisiOut1[i]=="Mati") && (kondisiOut2[i]=="Mati")){
      if(i==0){
        terbesarE=nilaiKondisi[i];
      }
      else{
        if(terbesarE<nilaiKondisi[i]){
          terbesarE=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Mati") && (kondisiOut2[i]=="Cukup Lama Down")){
      if(i==0){
        terbesarF=nilaiKondisi[i];
      }
      else{
        if(terbesarF<nilaiKondisi[i]){
          terbesarF=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Mati") && (kondisiOut2[i]=="Lama Down")){
      if(i==0){
        terbesarG=nilaiKondisi[i];
      }
      else{
        if(terbesarG<nilaiKondisi[i]){
          terbesarG=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Mati") && (kondisiOut2[i]=="Lebih Lama Down")){
      if(i==0){
        terbesarH=nilaiKondisi[i];
      }
      else{
        if(terbesarH<nilaiKondisi[i]){
          terbesarH=nilaiKondisi[i];
        }
      }
    }
    else if((kondisiOut1[i]=="Mati") && (kondisiOut2[i]=="Sangat Lama Down")){
      if(i==0){
        terbesarI=nilaiKondisi[i];
      }
      else{
        if(terbesarI<nilaiKondisi[i]){
          terbesarI=nilaiKondisi[i];
        }
      }
    }  
  }

  if(terbesarA>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Sangat Lama1 && Mati2 = ");
    Serial.println(terbesarA);
  }
  
  if(terbesarB>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Lebih Lama1 && Mati2 = ");
    Serial.println(terbesarB);
  }
  
  if(terbesarC>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Lama1 && Mati2 = ");
    Serial.println(terbesarC);
  }
  
  if(terbesarD>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Cukup Lama1 && Mati 2 = ");
    Serial.println(terbesarD);
  }
  
  if(terbesarE>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Mati1 && Mati2 = ");
    Serial.println(terbesarE);
  }
  
  if(terbesarF>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Mati1 && Cukup Lama 2 = ");
    Serial.println(terbesarF);
  }
  if(terbesarG>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Mati1 && Lama 2 = ");
    Serial.println(terbesarG);
  }
  
  if(terbesarH>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Mati1 && Lebih Lama 2 = ");
    Serial.println(terbesarH);
  }
  
  if(terbesarI>=0){
    Serial.println("====Nilai Output====");
    Serial.printf("Nilai Output Mati1 && Sangat Lama2 = ");
    Serial.println(terbesarI);
  }
}

//Defuzzyfikasi
void defuzzyfikasi(){
  //Inisiasi awal nilai tiap variabel sama dengan nol
  hasilPembilang1 = 0;
  hasilPenyebut1 = 0;
  hasilPembilang2 = 0;
  hasilPenyebut2 = 0;
  jumlah_sampel1 = 0;
  jumlah_sampel2 = 0;
  jumlah_sampel3 = 0;
  jumlah_sampel4 = 0;
  jumlah_sampel5 = 0;
  jumlah_sampel6 = 0;
  jumlah_sampel7 = 0;
  jumlah_sampel8 = 0;
  jumlah_sampel9 = 0;
  hasilDefuzzyfikasi1 = 0;
  hasilDefuzzyfikasi2 = 0;
  pengaliA = terbesarA; //pompa up sangat lama && pompa down mati
  pengaliB = terbesarB; //pompa up lebih lama && pompa down mati
  pengaliC = terbesarC; //pompa up lama && pompa down mati
  pengaliD = terbesarD; //pompa up cukup lama && pompa down mati
  pengaliE = terbesarE; //pompa up mati && pompa down mati
  pengaliF = terbesarF; //pompa up mati && pompa down cukup lama
  pengaliG = terbesarG; //pompa up mati && pompa down lama
  pengaliH = terbesarH; //pompa up mati && pompa down lebih lama
  pengaliI = terbesarI; //pompa up mati && pompa down sangat lama

  //defuzzyfikasi untuk pompa up & pompa down
    if(pengaliA>0){
      sampel1 = 5;
      titik_sampelA=1;
      
      for(int a=16; a<=20; a++){
        jumlah_sampel1 = ((titik_sampelA*a+jumlah_sampel1)+pengaliA);
      }
    }
    if(pengaliB>0){
      sampel2 = 6;
      titik_sampelB=1;
      for(int b=10; b<=15; b++){
        jumlah_sampel2 = ((titik_sampelB*b+jumlah_sampel2)+pengaliB);
      }
    }
    if(pengaliC>0){
      sampel3 = 5;
      titik_sampelC=1;
      for(int c=5; c<=9; c++){
        jumlah_sampel3 = ((titik_sampelC*c+jumlah_sampel3)+pengaliC);
      }
    }
    if(pengaliD>0){
      sampel4 = 4;
      titik_sampelD=1;
      for(int d=1; d<=4; d++){
        jumlah_sampel4 = ((titik_sampelD*d+jumlah_sampel4)+pengaliD);
      }
    }
    if(pengaliE>=0){
      sampel5 = 0;
      titik_sampelE=0;
      for(int e=1; e<=5; e++){
        jumlah_sampel5 = titik_sampelE*e+jumlah_sampel5;
      }
    }
    if(pengaliF>0){
      sampel6 = 4;
      titik_sampelF=1;
      for(int f=1; f<=4; f++){
        jumlah_sampel6 = ((titik_sampelF*f+jumlah_sampel6)+pengaliF);
      }
    }
    if(pengaliG>0){
      sampel7 = 5;
      titik_sampelG=1;
      for(int g=5; g<=9; g++){
        jumlah_sampel7 = ((titik_sampelG*g+jumlah_sampel7)+pengaliG);
      }
    }
    if(pengaliH>0){
      sampel8 = 6;
      titik_sampelH=1;
      for(int h=10; h<=15; h++){
        jumlah_sampel8 = ((titik_sampelH*h+jumlah_sampel8)+pengaliH);
      }
    }
    if(pengaliI>0){
      sampel9 = 5;
      titik_sampelI=1;
      for(int i=16; i<=20; i++){
        jumlah_sampel9 = ((titik_sampelI*i+jumlah_sampel9)+pengaliI);
      }
    }

  hasilPembilang1 = (jumlah_sampel1*pengaliA)+(jumlah_sampel2*pengaliB)+(jumlah_sampel3*pengaliC)+(jumlah_sampel4*pengaliD)+(jumlah_sampel5*pengaliE);
  hasilPembilang2 = (jumlah_sampel5*pengaliE)+(jumlah_sampel6*pengaliF)+(jumlah_sampel7*pengaliG)+(jumlah_sampel8*pengaliH)+(jumlah_sampel9*pengaliI);
  hasilPenyebut1 = ((sampel1+0.5)*pengaliA)+((sampel2+0.5)*pengaliB)+((sampel3+0.5)*pengaliC)+((sampel4+0.5)*pengaliD)+(sampel5*pengaliE);
  hasilPenyebut2 = (sampel5*pengaliE)+((sampel6+0.5)*pengaliF)+((sampel7+0.5)*pengaliG)+((sampel8+0.5)*pengaliH)+((sampel9+0.5)*pengaliI);
  hasilDefuzzyfikasi1=hasilPembilang1/hasilPenyebut1;
  hasilDefuzzyfikasi2=hasilPembilang2/hasilPenyebut2;

  if(isnan(hasilDefuzzyfikasi1)){
    hasilDefuzzyfikasi1 = 0;
  }
  if(isnan(hasilDefuzzyfikasi2)){
    hasilDefuzzyfikasi2 = 0;
  }
  
  Serial.printf("Hasil Pembilang 1 : ");
  Serial.println(hasilPembilang1);
  Serial.printf("Hasil Penyebut 1 : ");
  Serial.println(hasilPenyebut1);
  Serial.printf("Hasil Pembilang 2 : ");
  Serial.println(hasilPembilang2);
  Serial.printf("Hasil Penyebut 2 : ");
  Serial.println(hasilPenyebut2);
  Serial.printf("Defuzzyfikasi 1 : ");
  Serial.println(hasilDefuzzyfikasi1);
  Serial.printf("Defuzzyfikasi 2 : ");
  Serial.println(hasilDefuzzyfikasi2);
}

void fuzzy(){
  fuzzyfikasi();
  inferensi();
  defuzzyfikasi();
}

//========== O U T P U T   P O M P A ============
void pompaOn(){
  //Inisiasi awal nilai tiap variabel sama dengan nol
  timer1 = 0;
  timer2 = 0;
  timer1 = hasilDefuzzyfikasi1*1000;
  timer2 = hasilDefuzzyfikasi2*1000;
  Serial.printf("Timer 1 : ");
  Serial.println(timer1);
  Serial.printf("Timer 2 : "); 
  Serial.println(timer2);
  if((timer1>0) && (timer2>0)){
    Serial.println("Pompa Up dan Down Menyala");
    statusPompa1 = "On";
    statusPompa2 = "On";
    digitalWrite(pinRelay1, LOW);
    digitalWrite(pinRelay2, LOW);
    pumpSatu = 1;
    pumpDua = 1;
    stateTimer = 1;
    //Send data to Firebase
    if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
      Serial.println("Status Sistem Terkirim");
    }
    else{
      Serial.println("Status Sistem Tidak Terkirim");
      Serial.println("Karena: " + databaseAkuaponik.errorReason());
    }
    if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
      Serial.println("Status Sistem Terkirim");
    }
    else{
      Serial.println("Status Sistem Tidak Terkirim");
      Serial.println("Karena: " + databaseAkuaponik.errorReason());
    }
  }
  else if(timer1>0 && timer2 == 0){
    Serial.println("Pompa Up Menyala");
    statusPompa1 = "On";
    statusPompa2 = "Off";
    digitalWrite(pinRelay1, LOW);
    digitalWrite(pinRelay2, HIGH);
    pumpSatu = 1;
    pumpDua = 0;
    stateTimer = 1;
    //Send data to Firebase
    if(pumpSatu == 1 && pumpDua == 0 && stateTimer == 1){
      if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
      if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
    }
  }
  else if(timer1 == 0 && timer2>0){
    Serial.println("Pompa Down Menyala");
    statusPompa1 = "Off";
    statusPompa2 = "On";
    digitalWrite(pinRelay1, HIGH);
    digitalWrite(pinRelay2, LOW);
    pumpSatu = 0;
    pumpDua = 1;
    stateTimer = 1;
    //Send data to Firebase
    if(pumpSatu == 0 && pumpDua == 1 && stateTimer == 1){
      if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
      if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
    }
  }
  else{
    Serial.println("Pompa Mati");
    statusPompa1 = "Off";
    statusPompa2 = "Off";
    digitalWrite(pinRelay1, HIGH);
    digitalWrite(pinRelay2, HIGH);
    pumpSatu = 0;
    pumpDua = 0;
    stateTimer = 0;
    //Send data to Firebase
    if(pumpSatu == 0 && pumpDua == 0 && stateTimer == 0){
      if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
      if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
        Serial.println("Status Sistem Terkirim");
      }
      else{
        Serial.println("Status Sistem Tidak Terkirim");
        Serial.println("Karena: " + databaseAkuaponik.errorReason());
      }
    }
  }
  previousPump = millis();
}

//========== T A M P I L A N   L C D ============
void tampilLCD(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(dow + ",");
  display.setTextSize(1);
  display.setCursor(22,0);
  display.println(MyDate);  
  display.setTextSize(1);
  display.setCursor(20,10);
  display.println(MyTime);
  
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("pH:");
  display.setTextSize(1);
  display.setCursor(18,20);
  display.print(pH1);
  display.setTextSize(1);
  display.setCursor(45,20);
  display.print("&");
  display.setTextSize(1);
  display.setCursor(55,20);
  display.print(pH2);
  display.display();

  display.setTextSize(1);
  display.setCursor(0,30);
  display.print("P1:");
  display.setTextSize(1);
  display.setCursor(15,30);
  display.print(statusPompa1);
  display.setTextSize(1);
  display.setCursor(40,30);
  display.print("P2:");
  display.setTextSize(1);
  display.setCursor(55,30);
  display.print(statusPompa2);

  display.setTextSize(1);
  display.setCursor(10,40);
  display.print(sistem);
  
  display.display();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the database URL and database secret(required) */
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);


  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);

  //RTC Inisialisasi
#ifndef ESP32
  while (!Serial);
#endif

if (! rtc.begin()) {
Serial.println("Couldn't find RTC");
while (0);
}
  
rtc.adjust(DateTime(__DATE__, __TIME__));
 
  display.begin();
  display.setContrast(40);
  
  display.display();
  delay(2);
  display.clearDisplay();
 
  display.clearDisplay();
  display.setTextColor(BLACK);
  //display.startscrollright(0x00, 0x0F);
  display.setTextSize(2);
  display.setCursor(15,15);
  display.print("Smart");
  display.setTextSize(1);
  display.setCursor(17,30);
  display.print("Akuaponik");
  display.display();
  delay(1000);
  
  //Deklarasi pin Mode
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  digitalWrite(pinRelay1, HIGH);
  digitalWrite(pinRelay2, HIGH);
  
  //Waiting read data
  statusPompa1 = "Off";
  statusPompa2 = "Off";
  sistem = ".....";
}

void loop() {
  // put your main code here, to run repeatedly:
  currentTime=millis();
  if(currentTime - myTimerDisplay >= timeDisplay){
    myTimerDisplay = currentTime;
    DateTime now = rtc.now();
  
    tanggal = now.day();
    bulan = now.month();
    tahun = now.year();
    jam = now.hour();
    menit = now.minute();
    detik = now.second();
  
    dow = hari[now.dayOfTheWeek()];
    now.unixtime();
    now.unixtime()/86400L ;
  
    //Get the now day value
    Serial.print("Since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime()/86400L);
    Serial.println("d");
  
    MyDate = MyDate + tanggal + "/" + bulan + "/" + tahun;
    MyTime = MyTime + jam + ":" + menit + ":" + detik;
  
    //Send to serial monitor
    Serial.println(dow);
    Serial.println(MyDate);
    Serial.println(MyTime);
  
    tampilLCD();
    MyDate = " ";
    MyTime = " ";
    
    display.clearDisplay();
  }

  if(currentTime - myTimer >= timeSampling){
    myTimer = currentTime;
    bacaSensor();
    if(stateTimer == 0){
      if((currentTime - previousRead) >= timerReadSensor){
        previousRead = currentTime;
        setPoint();
        getTanaman();
        fuzzy();
        pompaOn();
        statusSistem(); 
      }
    }
    else {
      if(currentTime - previousPump >= timer1 && pumpSatu ==1){
        Serial.println("Pompa Up Mati");
        digitalWrite(pinRelay1, HIGH);
        pumpSatu = 0;
        statusPompa1 = "Off";
        //Send data to Firebase
        if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
          Serial.println("Status Pompa Terkirim");
        }
        else{
          Serial.println("Status Pompa Tidak Terkirim");
          Serial.println("Karena: " + databaseAkuaponik.errorReason());
        }     
      }
      if(currentTime - previousPump >= timer1 && pumpDua ==1){
        Serial.println("Pompa Down Mati");
        digitalWrite(pinRelay2, HIGH);
        pumpDua = 0;
        statusPompa2 = "Off";
        //Send data to Firebase
        if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
          Serial.println("Status Sistem Terkirim");
        }
        else{
          Serial.println("Status Sistem Tidak Terkirim");
          Serial.println("Karena: " + databaseAkuaponik.errorReason());
        }
      }
      if(pumpSatu == 0 && pumpDua == 0){
        Serial.println("Pompa Up & Down Mati");
        statusPompa1 = "Off";
        statusPompa2 = "Off";
        stateTimer = 0;
        previousPump = 0;
        previousRead = currentTime;        
        //send data to Firebase
        if(Firebase.setString(databaseAkuaponik, "Pompa Up/Status", statusPompa1)){
          Serial.println("Status Pompa Terkirim");
        }
        else{
          Serial.println("Status Pompa Tidak Terkirim");
          Serial.println("Karena: " + databaseAkuaponik.errorReason());
        }
        if(Firebase.setString(databaseAkuaponik, "Pompa Down/Status", statusPompa2)){
          Serial.println("Status Pompa Terkirim");
        }
        else{
          Serial.println("Status Pompa Tidak Terkirim");
          Serial.println("Karena: " + databaseAkuaponik.errorReason());
        }
      }
    }  
  }
}
