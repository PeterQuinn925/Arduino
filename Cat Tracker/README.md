**WORK IN PROGRESS**

## Problem statement
My daughter has an adventurous cat and we want to know where it goes. She lives in a ground floor apartment  in a small town in Oregon. The apartment has a small rocky hill and some fields behind it. The cat goes out to the rocks and it is impossible to see where he goes. 
	I want to build a collar that tracks his position and puts the locations on a map. He always comes in after a few hours so there’s no need to go find him. There’s no need for real time tracking.
This is a hobby project, so I want to do this as cheaply as possible with no recurring fees or subscriptions. Also, I intend to deploy it at my daughter’s. I live an 8 hour drive away and visit maybe two or three times a year. While I can get my daughter to reboot something and she will want to look at a map and see what her cat has been up to, she is a busy working adult. I need to be able to do any admin remotely.
*picture of archie the cat goes here*
## Architectural tradeoffs
The way this needs to work is there is a battery powered device in the cat’s collar that collects the location and transmits it to a server someplace, using some wireless technology. The server can be connected to via the web and show a map of where the cat has been.
There are commercially available options for this kind of thing. Some that may meet the requirements and some that don’t. Let’s look at some of them.
- The most obvious choice is to use a cat collar with an Apple airtag. Airtags work by using Bluetooth Low Energy (BLE). The tag itself is a BLE beacon and it is tracked by nearby Apple devices which transmit the location to Apple’s network. 
This would be a good solution except there is no coverage out in the fields where the cat roams. The range of Airtags is listed as around 30-40 feet.
- The next obvious choice is to go with GPS based subscription service such as Tractive GPS ($25 for the device, $5-10/month depending on length of subscription). There are a number of other commercial devices that use cellular networks for always on data connections. These would all work, except I want to avoid yet another subscription.
- I could build my own device that uses a cellular network such as a LILYGO T-A7670G R2 4G LTE CAT1 SIM Module ESP32 TTGO. Since I don’t need real time data, this is a bit overkill. Plus, I’d need to pay for the data for the SIM card. It’s really cheap but it is a recurring expense that I’d rather avoid.
- The last option I considered is using LoRa and/or Meshtastic networking. The hardware I ultimately bought has the capability for Mestastic, but I decided I didn’t need it. Caching the data and downloading via Wifi is good enough
- The last major decision is where and how to host the server. If I was going to implement this at my house, it would be easy. I would use an existing on-prem server in the form of a Raspberry PI. I have a spare Raspberry PI Zero 2 that I considered using. The other alternative is one of the cloud services. I considered the usual hyperscalers. These all required a subscription after a trial period. I looked at some other minor players and found that Oracle Cloud has an “always free” tier that would suit my needs. Since I wanted remote administration, hosting in a cloud service for free was the winner.
## System Design

![arch](https://github.com/user-attachments/assets/6b2ec311-65b2-4ed4-a4d0-cb4353a5dfa6)

The chosen implementation is to use a Heltec ESP32 Tracker that has GPS/GNSS, Wifi, LoRa radio, a display screen and Lipo battery management. https://heltec.org/project/wireless-tracker/ List price is $23.
![heltec](https://github.com/user-attachments/assets/0e6fd37c-e893-4b8e-9655-43119b7419b5)

The Heltec gets the location from GPS and if the Wifi is available, sends it to a cloud server via MQTT. If Wifi is not available, it stores it until the Wifi is available and then sends the stored data along to the server. The cloud server runs the Mosquitto MQTT server. There is another process on the server that runs the Mosquitto client and subscribes to these messages. When one arrives, it puts it in the database. A Grafana server is also running on this cloud server. At any time, a user can view the location data plotted on a map via a standard web browser. MQTT isn’t strictly needed here. A different transport mechanism could have been used since it’s a single message and single subscriber. However, MQTT is easy and convenient.
## Security implications.
This is a low risk situation and security requirements are minimal. There is no real value or harm if someone gets the cat location data. While there is a TLS option for MQTT and we could implement HTTPS for the Grafana dashboards, there’s no need for it in this implementation.
## Programming the Heltec
I started with the example GPS code that displays the lat/long and time from the GPS system on the built in display. Then, I copied some code I had from a previous project to add Wifi connection and send the data to a MQTT server that I have already. This was an always online option with no backfill or anything, just prove out sending the data and seeing the quality of the position.
I wasted some time trying to use the serial monitor in the Arduino UI. It doesn’t when using the GPS and I never figured out why. There were a number of solutions posted online or suggested by Claude.AI but none worked.
I also wasted some time trying to get the GPS to work with the display turned off. In the final design, I don’t want the display because I want to save power. While I was able to minimize the time the display is on, I wasn’t able to completely turn it off. It appears that the GPS and the display share the same power pin and you must initialize the display for the GPS to work. 
Next I implemented storing the lat/long/time data when there is no Wifi connection and backfilling when it comes back in range. One of the critical parts of the solution is that it survive ESP32 deep sleep. So, it couldn’t just store it in a C data structure. Claude.ai suggested using LittleFS and wrote most of this code.
## Test backend server
At this point I had MQTT lat/long/time data going to a local MQTT server. I could see the messages arriving but no way to visualize them. I did look up one lat/long pair on the map to validate that it was in the right place, but that would be super tedious to do for more than a single value. I already have a local server that has Mosquitto and have weather data going via this server to an Influx database which I then visualize with Grafana. This made a perfect test system.
![2026-04-05_15-14-40](https://github.com/user-attachments/assets/9fa3088c-d2a5-41c2-8d2c-3915a5e9dd2e)
Copying most of the code from my weather system, I created a little python program that would subscribe to the MQTT messages and write out the data to the Influx database. Once this was flowing I created a new Grafana dashboard to show dots on a map. I used Influxdb because that’s what I had handy. When I implemented it for the deployed system, I used SQLite.  I thought that Influx was overkill for this scenario and SQLite a better fit. There’s such a small amount of data and that the advantages of a specialized time series database didn’t make sense.
## Battery Power
Now that I had a system that would work without Wifi, I did some testing. I plugged it into a USB battery made for charging phones on the go and took it out of range. This all worked fine, but obviously not the optimum solution. 
I had a couple of 150mAh LiPo batteries left over from previous projects, so I grabbed one. The connector that the Heltec uses is different than the “standard?” ones that I had. One of the things that came with the device was a connector for the battery. It’s similar in form but much smaller. I cut off the old connector and soldered in wires to the new connector. Frustratingly, the battery did not work. It did not appear to charge or power the device. I was able to confirm with my multimeter that the battery never put out any voltage. I suspected the battery itself was bad, or possibly my soldering was faulty. I had a second battery and used that to confirm my suspictions. I cut off the Heltec battery connector from the suspected bad battery and, without soldering, twisted the wires of the new battery to the connector. When plugging this into the Heltec, the charging LED lit right up - and it ran off the battery at least for a few seconds. I gleefully unplugged it and made a permanent solder connection.
Without any optimization, the battery lasts ~1 hr when fully awake and updating the display once a minute. As I expected, this was not good enough and I needed to optimize it by using the ESP32 deep sleep mode and turning off the display.
## Low Power/Deep Sleep
Getting low power/deep sleep to work
Getting GPS to be more accurate after deep sleep
After optimizations, battery now lasts ~6 hrs. Might want to swap out the battery for a slightly larger one for more run time, but let’s deploy it first.
Trying to get it to detect battery vs USB power. Goal here was to show the battery fill level, connection stats, if it’s backfilling, etc when the device is plugged in. And not have it sleep when plugged in. Spent the better part of a day doing this before figuring it out I would need to do it with additional hardware. Granted, it would be a couple of resistors and some soldering, but it wasn’t worth the effort.
## 3D printing a case
Design in openSCAD
2 parts base and screws
## H2 Setting up a Cloud server
I created an account on Oracle Cloud. I first created an instance using Oracle Linux with basically the default options. When doing so, it prompted me to create and download a public and private keys. I struggled a bit to figure out how to get it to create a public IP address, but with some help from Claude.ai, I was able to do so. Then, using the public key, I was able to SSH in. I found that Oracle Linux didn’t have the tools that I needed to install Mosquitto - apt isn’t available, there’s yum instead, but I was unable to get Mosquitto installed using it. After a bit, I gave up and terminated that instance and created a new one using Ubuntu.
### Installing software
With Ubuntu I was able to use the tools I’m used to (apt) to install Mosquitto, mosquitto-clients, Grafana OSS, sqlite, pahoMQTT, and the SQLite plugin for Grafana. PahoMQTT is the library for using MQTT in Python.
I next tried to access the Grafana site via the URL and failed. There were multiple security hoops to jump through first. As I suck at network security stuff, I had Claude.ai help. I needed to open the ports in the Linux instance. I needed to create security rules in Oracle Cloud to allow MQTT and Grafana access.
### Processing MQTT Data
Now that I was able to see the Grafana site, I next needed some data. I reprogrammed the Heltec device to use the Oracle Cloud IP address instead of my local Mosquitto server address. Next, I created a copy of the python program that I was previously using to subscribe to MQTT and write out InfluxDB on this server and modified it for SQLite. It first creates the database file and table, if it doesn’t already exist and insert values for lat/long and time. After testing it, I made it into a service. I did run into some minor issues with permissions on the database file - it was a bit tricky to make it accessible to both my python code (user ubuntu) and Grafana (user grafana).
### Grafana Dashboard
put the query used here
Create a dashboard in Grafana showing the SQLite data <half done, need to make the colors change based on time. TODO>
Give access to the Grafana instance to the ultimate user of the system and see how they like it.
Field test on the my local cat Murky
Fixing the inevitable problems <<picture of Murky as the Beta Tester>>
Installation on site with Archie



