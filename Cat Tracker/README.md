*WORK IN PROGRESS*

Problem statement
My daughter has an adventurous cat and we want to know where it goes. She lives in a ground floor apartment  in a small town in Oregon. The apartment has a small rocky hill and some fields behind it. The cat goes out to the rocks and it is impossible to see where he goes. 
	I want to build a collar that tracks his position and puts the locations on a map. He always comes in after a few hours so there’s no need to go find him. There’s no need for real time tracking.
This is a hobby project, so I want to do this as cheaply as possible with no recurring fees or subscriptions. Also, I intend to deploy it at my daughter’s. I live an 8 hour drive away and visit maybe two or three times a year. While I can get my daughter to reboot something and she will want to look at a map and see what her cat has been up to, she is a busy working adult. I need to be able to do any admin remotely.
<<picture of archie>>
Architectural tradeoffs
The way this works is there is a battery powered device in the cat’s collar that collects the GPS location and transmits it to a server someplace, using some wireless technology. The server can be connected to via the web and shows a map of where the cat has been.
There are commercially available options for this kind of thing. Some that may meet the requirements and some that don’t. Let’s look at some of them.
The most obvious choice is to use a cat collar with an Apple airtag. Airtags work by using Bluetooth Low Energy (BLE). The tag itself is a BLE beacon and it is tracked by nearby Apple devices which transmit the location to Apple’s network. 
This would be a good solution except there is no coverage out in the fields where the cat roams. The range of Airtags is listed as around 30-40 feet.
The next obvious choice is to go with GPS based subscription service such as Tractive GPS ($25 for the device, $5-10/month depending on length of subscription). There are a number of other commercial devices that use cellular networks for always on data connections. These would all work, except I want to avoid yet another subscription.
I could build my own device that uses a cellular network such as a LILYGO T-A7670G R2 4G LTE CAT1 SIM Module ESP32 TTGO. Since I don’t need real time data, this is a bit overkill. Plus, I’d need to pay for the data for the SIM card. It’s really cheap but it is a recurring expense that I’d rather avoid.
The last option I considered is using LoRa and/or Meshtastic networking. The hardware I ultimately bought has the capability for Mestastic, but I decided I didn’t need it. Caching the data and downloading via Wifi is good enough
The last major decision is where and how to host the server. If I was going to implement this at my house, it would be easy. I would use an existing on-prem server in the form of a Raspberry PI. I have a spare Raspberry PI Zero 2 that I considered using. The other alternative is one of the cloud services. I considered the usual hyperscalers. These all required a subscription after a trial period. I looked at some other minor players and found that Oracle Cloud has an “always free” tier that would suit my needs. Since I wanted remote administration, hosting in a cloud service for free was the winner.
System Design

The chosen implementation is to use a Heltec ESP32 Tracker that has GPS/GNSS, Wifi, LoRa radio, a display screen and Lipo battery management. https://heltec.org/project/wireless-tracker/ List price is $23.


The Heltec gets the location from GPS and if the Wifi is available, sends it to a cloud server via MQTT. If Wifi is not available, it stores it until the Wifi is available and then sends the stored data along to the server. The cloud server runs the Mosquitto MQTT server. There is another process on the server that runs the Mosquitto client and subscribes to these messages. When one arrives, it puts it in the database. A Grafana server is also running on this cloud server. At any time, a user can view the location data plotted on a map via a standard web browser. MQTT isn’t strictly needed here. A different transport mechanism could have been used since it’s a single message and single subscriber. However, MQTT is easy and convenient.
Security implications.
This is a low risk situation and security requirements are minimal. There is no real value or harm if someone gets the cat location data. While there is a TLS option for MQTT and we could implement HTTPS for the Grafana dashboards, there’s no need for it in this implementation.
Heltec device
Programming
Started with the example GPS code
Display the lat/long and time on the display
Next, copied code from a previous project to add Wifi and send the data via MQTT. Always online, no backfill yet.
Trouble getting it to use the serial output. Never solved.
Trouble getting it to use GPS without the display found out you had to turn the display on for it to work. I think the GPS and display use the same power pin. Found some workarounds to minimize the display for low power usage.
Implemented storing the lat/long/time data when no Wifi and backfilling when there is. Claude wrote most of this code. Critical that it survives deepsleep.
Getting the battery to work at all (e.g. solder the connector to a working battery, not a broken one)
Battery lasts ~1 hr when fully awake and constantly updating the display
Getting low power/deep sleep to work
Getting GPS to be more accurate after deep sleep
After optimizations, battery now lasts ~6 hrs. Might want to swap out the battery for a slightly larger one for more run time, but let’s deploy it first.
Trying to get it to detect battery vs USB power. Goal here was to show the battery fill level, connection stats, if it’s backfilling, etc when the device is plugged in. And not have it sleep when plugged in. Spent the better part of a day doing this before figuring it out I would need to do it with additional hardware. Granted, it would be a couple of resistors and some soldering, but it wasn’t worth the effort.
Test system - existing Mosquitto, InfluxDB and Grafana
Getting the timestamp to work right when backfilling
Getting Grafana to display a map with the dots on it.
Getting Grafana to display different colors for different timestamps (TODO!)
Writing code to subscribe to MQTT and push the data to InfluxDB
3D printing a case
Design in openSCAD
2 parts base and screws
Setting up a Cloud server (ip= 161.153.21.231)
Struggles with Oracle Linux, switch to Ubuntu which I’m much more familiar with
How to get to be able to SSH into the server (configuring it for public IP and generating a public/private key pair)
Install mosquitto, mosquitto-clients, Grafana OSS, sqlite3, pahoMQTT, sqlite plugin for Grafana
Struggles getting the holes in the firewall for Grafana
Open the firewall for MQTT and enable MQTT to accept data via the port. Reprogram the device to send data to the cloud server.
Write code to subscribe to MQTT and push the data to SQLite db <done, made into a service>
Create a dashboard in Grafana showing the SQLite data <half done, need to make the colors change based on time. TODO>
Give access to the Grafana instance to the ultimate user of the system and see how they like it.
Field test on the my local cat Murky
Fixing the inevitable problems <<picture of Murky as the Beta Tester>>
Installation on site with Archie



