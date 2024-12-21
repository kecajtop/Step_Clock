#include <Wire.h>
#include "RTClib.h" //https://github.com/adafruit/RTClib
#include <AccelStepper.h> //http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <StepClock.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <WiFiUdp.h>

const char* ssid = "ssid";
const char* password = "password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
bool wifiFirstConnected = false;

#define motorHour1Pin1  27    // IN4 on the ULN2003 driver 1
#define motorHour1Pin2    14   // IN3 on the ULN2003 driver 1
#define motorHour1Pin3    12   // IN2 on the ULN2003 driver 1
#define motorHour1Pin4    13  // IN1 on the ULN2003 driver 1

// Minute pin definitions
#define motorMin1Pin1    5   // IN1 on the ULN2003 driver 2
#define motorMin1Pin2     23 // IN2 on the ULN2003 driver 2
#define motorMin1Pin3     19  // IN3 on the ULN2003 driver 2
#define motorMin1Pin4     18 // IN4 on the ULN2003 driver 2
#define hallMin1Pin       34
#define hallHour1Pin       36
#define HALFSTEP 8

#define OTA 1

#define CLK 2
#define DT 4

AccelStepper stepperHour1(HALFSTEP, motorHour1Pin1, motorHour1Pin3, motorHour1Pin2, motorHour1Pin4);
AccelStepper stepperMin1(HALFSTEP, motorMin1Pin1, motorMin1Pin3, motorMin1Pin2, motorMin1Pin4);
RTC_DS1307 rtc;

Step_Clock my_clock;

static unsigned long msTick =0;

int8_t seconds;

void onEvent (system_event_id_t event, system_event_info_t info) {
    Serial.printf ("[WiFi-event] event: %d\n", event);

    switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.printf ("Connected to %s\r\n", info.connected.ssid);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.printf ("Got IP: %s\r\n", IPAddress (info.got_ip.ip_info.ip.addr).toString ().c_str ());
        Serial.printf ("Connected: %s\r\n", WiFi.status () == WL_CONNECTED ? "yes" : "no");
        //digitalWrite (ONBOARDLED, LOW); // Turn on LED
        wifiFirstConnected = true;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.printf ("Disconnected from SSID: %s\n", info.disconnected.ssid);
        Serial.printf ("Reason: %d\n", info.disconnected.reason);
        //digitalWrite (ONBOARDLED, HIGH); // Turn off LED
        //NTP.stop(); // NTP sync can be disabled to avoid sync errors
        break;
     default:
        break;
    }
}


void setup() 
{
  Serial.begin(115200);
  Serial.println("Booting");
  #ifdef OTA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
      }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.onEvent (onEvent);
  #endif    
  //my_clock.Debug(&Serial);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //while (1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//this sets time based on the compilation time of the computer
  my_clock.Init(&stepperHour1, &stepperMin1, &rtc);
  my_clock.Calibration(hallHour1Pin,hallMin1Pin);
  my_clock.SetHomePosition();
  my_clock.MoveMotorsToHomePosition();
  my_clock.UpdateTime();
  my_clock.UpdateTimeAndMove();
}

void loop() 
{
      static int i = 0;
    static int last = 0;
    //my_clock.UpdateTime();
    #ifdef OTA
        if (wifiFirstConnected) {
        wifiFirstConnected = false;
        NTP.begin ("pool.ntp.org", timeZone, true, minutesTimeZone);
        NTP.setInterval (63);
        Serial.println("Starting NTP");
        }

     if ((millis () - last) > 5100) {
        //Serial.println(millis() - last);
        last = millis ();
        Serial.print (i); Serial.print (" ");
        Serial.print (NTP.getTimeDateString ()); Serial.print (" ");
        Serial.print (NTP.isSummerTime () ? "Summer Time. " : "Winter Time. ");
        Serial.print ("WiFi is ");
        Serial.print (WiFi.isConnected () ? "connected" : "not connected"); Serial.print (". ");
        Serial.print ("Uptime: ");
        Serial.print (NTP.getUptimeString ()); Serial.print (" since ");
        Serial.println (NTP.getTimeDateString (NTP.getFirstSync ()).c_str ());

        i++;
    }
    ArduinoOTA.handle();
    #endif
    my_clock.IncrementTime();
}
