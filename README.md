
# Smart Classroom With IoT and Fuzzy Logic

This project using ESP32 S3 to sensing DHT22, MQ135, LDR Sensor, Flame Sensor, HCSR-04 activate Solenoid Door Lock, Buzzer, LED, Cooling Fan 5 Volts, Exhaust Fan 5 Volts, Motor Servo SG90


## Lessons Learned

How to use AWS IoT Core, S3 Database using ESP32 S3. Deployment Website Static using S3 Static Hosting.
How to use Thingsboard, and Making The Dashboard 

## Tech Stack

**Client:** HTML,TailwindCSS, Arduino IDE (ino)

**Server for AWS Code:** AWS IoT Core, AWS S3 Bucket Server

**Server for Thingsboard Code:** Thingsboard

**Library Used for AWS Code:** 
```
#include "secret.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>
#include <MQ135.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
```

**Library Used for Thingsboard Code:** 
```
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ135.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
```

## Video Tutorial

[Click To Show Tutorial](https://www.youtube.com/watch?v=FjvjNx1LCvQ)
## Screenshots

![Web Screenshot](https://iotpribadi.s3.ap-southeast-1.amazonaws.com/SS_WebsitePribadi.png)

![Dashboard Thingsboard Screenshot](https://iotpribadi.s3.ap-southeast-1.amazonaws.com/SS_ThingsBoardPribadi.png)


## Support By
![Logo](https://iotkelompok2.s3.ap-southeast-1.amazonaws.com/logo-instrumentasi-biru+(2).png)


## License

[MIT](https://github.com/zakruzz/Simulation-Smart-Classrom-With-IoT-and-Fuzzy-Logic/blob/main/LICENSE)

