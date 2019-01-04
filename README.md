# MotionDetector
Ultra low power project, ESP32 Based Microcontroller motion sensor can be placed on any surface to detect very small motions using MPU6050 then notification sent to PushBullet via WiFi.

In this Configuration project on 4.2v draws only 640uA while asleep.

Devices Used:
------
-Lolin32 type ESP32 ($10)

-GY-521 breakout for MPU6050 ($2)

-2x 400mAh lipos in parallel 4.2v attached to Lolin32 battery port ($10 total)


Wiring
======
<table>
  <tr>
    <th>Lolin32</th>     <th>GY-521(MPU6050)</th></tr>
  <tr><td>3.3V</td><td>VCC ( GY-521 has onboard regulator 5v tolerant)</td></tr>
  <tr><td>GND</td><td>GND</td></tr>
  <tr><td>21</td><td>SDA</td></tr>
  <tr><td>22</td><td>SCL</td></tr>
  <tr><td>15</td><td>INT - Note below on allowed RTC IO Pins</td></tr>

</table>

Project Flow
----
The MPU6050 is configured to pulse low on the INT pin.  Registers are also configured to power down Gyros and temp sensor to put the MPU into sleep mode.
The ESP32 is has interrupt setup on pin 15 to wake whenever MPU6050 motion breaks the configured threshold.
After waking the ESP32 does POST to PushingBox(http://pushingbox.com) who has Scenario configured to notify Pushbullet(http://pushbullet.com) and send a notification to cell device.
ESP32 is put into low power state and sleeps until interrupt is triggered again.


Todo
-----
Lower power consumption would be nice:
- Slow ESP32 clock speed to use less current while awake.
- Remove delays to shorten awake time.

Battery status of any kind:
- Timer based delay to wake up daily to notify battery isn't dead yet.
- Message about battery voltage using transistor to voltage divider in order to not have constant power draw.

Increase sensitivity beyond simply setting Motion detection threshold to lowest value of 1.
- Possibly increase MPU polling time if it doesn't increase current.

Timer interrupt to only notify once per period of time.


Shoulders of Giants
----
These people did all the hard work:

MPU Register configuration for interrupt with some changes by me for lower power consumption: https://arduino.stackexchange.com/a/48430

ESP32 deep sleep documentation found here: https://github.com/espressif/esp-idf/blob/master/docs/en/api-reference/system/sleep_modes.rst

Cell phone notifications guide to connect ESP with pushingbox with pushbullet: https://www.geekstips.com/android-push-notifications-esp8266-arduino-tutorial/
