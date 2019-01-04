# MotionDetector
Ultra low power project, ESP32 Based Microcontroller motion sensor can be placed on any surface to detect very small motions using MPU6050 then notification sent to PushBullet via WiFi.

In this Configuration project on 4.2v draws only 640uA while asleep.

Devices Used:
------
-Lolin32 type ESP32

-GY-521 breakout for MPU6050

-2x 400mAh lipos in parallel 4.2v attached to Lolin32 battery port


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
