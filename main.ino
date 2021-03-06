#include <ArduinoJson.h>
#include <SparkTime.h>

#include "SparkFun_Photon_Weather_Shield_Library.h"

double humidity = 0;
double temperature = 0;
double pressure = 0;
double baroTemp = 0;
long lastPublish = 0;
int pingRate = 30000;
unsigned long currentTime;
String date;
String timeStr;

Weather sensor;
UDP UDPClient;
SparkTime rtc;

void setup()
{
    Particle.variable("humidity", humidity);
    Particle.variable("temperature", temperature);
    Particle.variable("pressurePascals", pressure);
    Particle.variable("baroTemp", baroTemp);

    rtc.begin(&UDPClient, "se.pool.ntp.org");
    rtc.setTimeZone(0);
    
    //Initialize the I2C sensors and ping them
    sensor.begin();

    /*You can only receive accurate barometric readings or accurate altitude
    readings at a given time, not both at the same time. The following two lines
    tell the sensor what mode to use. You could easily write a function that
    takes a reading in one made and then switches to the other mode to grab that
    reading, resulting in data that contains both accurate altitude and barometric
    readings. For this example, we will only be using the barometer mode. Be sure
    to only uncomment one line at a time. */
    sensor.setModeBarometer();
    //baro.setModeAltimeter();

    //These are additional MPL3115A2 functions that MUST be called for the sensor to work.
    sensor.setOversampleRate(7);
    /*Call with a rate from 0 to 7. See page 33 for table of ratios.
    Sets the over sample rate. Datasheet calls for 128 but you can set it
    from 1 to 128 samples. The higher the oversample rate the greater
    the time between data samples. */

     //Necessary register calls to enable temperature, baro and alt
    sensor.enableEventFlags();

}
void loop()
{
      readAllSensors();

      if(millis() - lastPublish > pingRate)
      {
    
        currentTime = rtc.now();
        date = rtc.yearString(currentTime) + "-" + rtc.monthString(currentTime) + "-" + rtc.dayString(currentTime);
        timeStr = rtc.hourString(currentTime) + ":" + rtc.minuteString(currentTime) + ":" + rtc.secondString(currentTime);
        
        const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(5);
        DynamicJsonDocument doc(capacity);

        JsonObject kitchen = doc.createNestedObject("kitchen");
        kitchen["date"] = date;
        kitchen["time"] = timeStr;
        kitchen["temperature"] = temperature;
        kitchen["pressure"] = pressure;
        kitchen["humidity"] = humidity;

        char data[100];
        serializeJson(doc, data);
        Particle.publish("data", data);
        lastPublish = millis();
      }
}

void readAllSensors()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  temperature = sensor.getTemp();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()

  //Measure the Barometer temperatureerature in F from the MPL3115A2
  baroTemp = sensor.readBaroTempF();

  //Measure Pressure from the MPL3115A2
  pressure = sensor.readPressure();

  //If in altitude mode, you can get a reading in feet with this line:
  //float altf = sensor.readAltitudeFt();
}
