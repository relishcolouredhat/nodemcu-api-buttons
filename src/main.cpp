#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "espconn.h"
#include "lwip/inet.h"
//#include "lwip/dns.h"

// SRC: https://tttapa.github.io/ESP8266/Chap15%20-%20NTP.html
WiFiUDP UDP;                     // Create an instance of the WiFiUDP class to send and receive
IPAddress timeServerIP;          // time.nist.gov NTP server address
const char* NTPServerName = "pool.ntp.org";
const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

// D2(gpio4)
int r_in  = 4;
int r_out = 0;
int y_in  = 5;
int y_out = 2;
int g_in  = 16;
int g_out = 14;
int runCounter = 0;
String version = "0.3.2";
//int inputPins = {r_in,y_in,g_in}
//int outputPins = {r_out,y_out,g_out}
ESP8266WiFiMulti wlan;
IPAddress DNS_IP( 8,8,8,8 ); //then down in setup()

IPAddress dns(8,8,8,8);

int buttonPins[][2] = {{r_in,r_out},{y_in,y_out},{g_in,g_out}};

//example code
//int ledpin = 2; // D4(gpio2)
//int button = 16; //D0(gpio4)
//int buttonState=0;
int r_state =0;
int y_state =0;
int g_state =0;

bool debug = false;

void allToggle(){
  uint8_t instr;
  //if(buttonPins[0][1] = LOW){instr = HIGH;}else{instr = LOW;}
  int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
  for (byte i = 0; i < buttonCount; i++){
    digitalWrite(buttonPins[i][1], HIGH);
    delay(30);
    digitalWrite(buttonPins[i][1], LOW);
    delay(200);
    digitalWrite(buttonPins[i][1], HIGH);
    }
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

inline int getSeconds(uint32_t UNIXTime) {
  return UNIXTime % 60;
}

inline int getMinutes(uint32_t UNIXTime) {
  return UNIXTime / 60 % 60;
}

inline int getHours(uint32_t UNIXTime) {
  return UNIXTime / 3600 % 24;
}

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(123);                          // Start listening for UDP messages on port 123
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
  Serial.println();
}

void setup() {
  int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
  for (byte i = 0; i < buttonCount; i++){
    pinMode(buttonPins[i][0], INPUT);
    pinMode(buttonPins[i][1], OUTPUT);
    digitalWrite(buttonPins[i][1], LOW);
    digitalWrite(buttonPins[i][1], HIGH);
    digitalWrite(buttonPins[i][1], LOW);
  }
  Serial.begin(9600);
  if (debug){
    while (!Serial) {
      allToggle();
      delay(50); // wait for serial port to connect. Needed for native USB port only
    }
  }
  Serial.println("************************************************************************");
  Serial.println("Starting buttonTester Arduino Project v "+version+" Kelsey Comstock 2019");
  //ESP8266WiFiMulti wlan = setupWifi("NeoBadger","huemonsterventshiny");
  Serial.println("");
  Serial.setDebugOutput(true);
  Serial.println("Adding wlan: prettyflyforawifi...");
  wlan.addAP("prettyflyforawifi","h3mpr0p3");
  Serial.println("Adding wlan: NeoBadger...");
  wlan.addAP("NeoBadger","huemonsterventshiny");
  Serial.println("Trying to connect to wlan...");
  wlan.run();
  String mdnsHandle;
  mdnsHandle = "test-hostname";
  if (MDNS.begin(mdnsHandle)) { // Start the mDNS responder for esp8266.local
    //MDNS.setInstanceName("test-hostname");
    Serial.println(("mDNS responder started ("+String(mdnsHandle)+".local)"));
    //MDNS.addService("_ftp", "_tcp", 80);
    }
  else {
    Serial.println("Error setting up MDNS responder!");
    }

  WiFi.dnsIP(0).printTo(Serial); //to make sure


  espconn_dns_setserver(0, DNS_IP); //to set the primary DNS to 8.8.8.4
  WiFi.dnsIP(0).printTo(Serial); //to make sure


  Serial.println("");
  wlan.run();
  startUDP();
  //delay(5000);
  WiFi.hostByName(NTPServerName, timeServerIP);
  // if(!WiFi.hostByName(NTPServerName, timeServerIP)) { // Get the IP address of the NTP server
  //   Serial.println("DNS lookup failed. Rebooting.");
  //   Serial.println("");
  //   Serial.println("");
  //   Serial.flush();
  //   ESP.reset();
  // }
  timeServerIP = IPAddress(216,232,132,77);
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);

  Serial.println("\r\nSending NTP request ...");
  sendNTPpacket(timeServerIP);
}

void allLed(uint8_t instr){
  MDNS.update();
  int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
  for (byte i = 0; i < buttonCount; i++){
    digitalWrite(buttonPins[i][1], instr);
    delay(6);
    }
}


void blinkLed(int ledPin,int t_on,int t_off, int cycles){
  for (byte i =0; i < cycles; i++){
    digitalWrite(ledPin, HIGH);
    delay(t_on);
    digitalWrite(ledPin, LOW);
    delay(t_off);
  }
}

void notifyLed(int ledPin,int switchPin){
  blinkLed(ledPin,50,25,3);
  switch (digitalRead(switchPin)){
    case 1:
      //blinkLed(g_out,50,25,3);
      allLed(HIGH);
      delay(200);
      allLed(LOW);
      break;
    default:
      delay(50);
      notifyLed(ledPin,switchPin);
  }
}

void notifyLedCustom(int ledPin,int switchPin,int a,int b, int c){
  blinkLed(ledPin,a,b,c);
  switch (digitalRead(switchPin)){
    case 1:
      //blinkLed(g_out,50,25,3);
      allLed(HIGH);
      delay(200);
      allLed(LOW);
      break;
    default:
      delay(50);
      notifyLed(ledPin,switchPin);
  }
}

void gameBlink(){
    allLed(HIGH);
    delay(200);
    allLed(LOW);
    delay(150);
    allLed(HIGH);
    delay(200);
    allLed(LOW);
    delay(1000);

}

int notifyGameLoop(int roundCounter, int randomInt){
  Serial.print("Round ");
  Serial.println(roundCounter);
  r_state=digitalRead(r_in);
  y_state=digitalRead(y_in);
  g_state=digitalRead(g_in);
  roundCounter++;
  int randomButton = random(0,3);
  //int newRandomInt = random(-1,1)*(randomInt+(randomButton*roundCounter));
  //notifyLedCustom(buttonPins[randomButton][1],buttonPins[randomButton][0],50+newRandomInt,25+randomInt,5);
  notifyLed(buttonPins[randomButton][1],buttonPins[randomButton][0]);
  if (r_state == 1 && y_state == 1){
    Serial.println("***************************");
    Serial.println("GAME EXIT COMMAND DETECTED!");
    Serial.println("***************************");
    return 0;
  }
  notifyGameLoop(roundCounter,randomInt);
}

void notifyGame(){
  /*  Calls 'notifyGameLoop' function, a simple (lame) 'whack-a-mole'
      mode which could be useful as an LED test and demo mode.     */
  String funcVer = "0.1.3";
  gameBlink();
  Serial.print("Starting notifyGAME v "+funcVer);
  notifyGameLoop(1,0);
  gameBlink();
}

void printTakeoverTime(){
  if (getTime() != 0 ){
    //unsigned long currentMillis = millis();
    //sendNTPpacket(timeServerIP);
    //unsigned long lastNTPResponse = millis();
    //unsigned long prevNTP = millis() - 120000; //(intervalNTP + 1);
    //uint32_t timeUNIX = 0;
    uint32_t actualTime = millis()/1000;
    uint32_t futureTime = actualTime + 300; // + incrementSeconds
    //Serial.println("");
    //Serial.println("");
    Serial.printf("\tTakover Time:\t2019-09-08T%.2d:%.2d:%.2d-00:00   \t", getHours(futureTime), getMinutes(futureTime), getSeconds(futureTime));
    //Serial.println("");
    //Serial.println("");
  } else {Serial.println("No NTP sync yet!");}
}

void buttonTest(){
  if (r_state == 1 && g_state == 1)
  {
    Serial.println("Two button is pressed!!");
    //digitalWrite(r_out, HIGH);
    //delay(200);
    allLed(HIGH);
    delay(2000);
  }

   if (r_state == 1 && y_state == 1)
  {
  notifyGame();
  }

 if (r_state == 1)
  {
    Serial.println("Red button is pressed!!");
    digitalWrite(r_out, HIGH);
    delay(200);
    //blinkLed(r_out,50,25,3);
  }
 if (r_state == 0)
   {
    digitalWrite(r_out, LOW);
    delay(200);
    }
 if (y_state == 1)
  {
    Serial.println("Yellow button is pressed!!");
    digitalWrite(y_out, HIGH);
    delay(200);
  }
 if (y_state == 0)
   {
    digitalWrite(y_out, LOW);
    delay(200);
    }
 if (g_state == 1)
  {
    Serial.println("Green button is pressed!!");
    printTakeoverTime();
    digitalWrite(g_out, HIGH);
    delay(200);
  }
 if (g_state == 0)
   {
    digitalWrite(g_out, LOW);
    delay(200);
    }
}



unsigned long intervalNTP = 60000; // Request NTP time every minute
//unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
unsigned long prevNTP = millis() - 120000; //(intervalNTP + 1);
uint32_t timeUNIX = 0;

unsigned long prevActualTime = 0;

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevNTP > intervalNTP) { // If a minute has passed since last NTP request
    prevNTP = currentMillis;
    Serial.println("\r\nSending NTP request ...");
    sendNTPpacket(timeServerIP);               // Send an NTP request
  }

  uint32_t time = getTime();                   // Check if an NTP response has arrived and get the (UNIX) time
  if (time) {                                  // If a new timestamp has been received
      timeUNIX = time;
      Serial.print("NTP response:\t");
      Serial.println(timeUNIX);
      lastNTPResponse = currentMillis;
    } else if ((currentMillis - lastNTPResponse) > 3600000) {
        Serial.println("More than 1 hour since last NTP response. Rebooting.");
        Serial.flush();
        ESP.reset();
      }

  uint32_t actualTime = timeUNIX + (currentMillis - lastNTPResponse)/1000;
  uint32_t futureTime = actualTime + 300; // + incrementSeconds
  if (actualTime != prevActualTime && timeUNIX != 0) { // If a second has passed since last print
      prevActualTime = actualTime;
      Serial.printf("\rUTC time:\t2019-09-08T%.2d:%.2d:%.2d-00:00   \t", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
      //Serial.printf("Future time:\t2019-09-08T%.2d:%.2d:%.2d-00:00   ", getHours(futureTime), getMinutes(futureTime), getSeconds(futureTime));
    }
  wlan.run();
  //while (wlan.run() != WL_CONNECTED) {
  //  Serial.println("Disconnected- retrying to connect to wlan...");
  //  allToggle();
  //  delay(500);
  //  }

  MDNS.update();
  r_state=digitalRead(r_in);
  y_state=digitalRead(y_in);
  g_state=digitalRead(g_in);
  buttonTest();

 if (runCounter == 0){
    Serial.println("First run; testing notifications...");
    runCounter++;
    notifyLed(r_out,r_in);
    notifyLed(y_out,y_in);
    notifyLed(g_out,g_in);
 }

}
