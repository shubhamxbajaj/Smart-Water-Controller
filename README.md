# Smart-Water-Controller

Summary:
1. Prototype Description
2. Prototype Features
3. Prototype Requirements
4. Prototype Working
      1. Using the app
      2. Using Telegram
5. Video Demonstration



## Prototype Description: This project uses an ultrasonic sensor and nodemcu along with a relay to automatically control the water level in overhead tanks. ## 
The benefits of using this prototype over already available products in the market is that:
* This prototype is extremely cheap and easy to maintain over any other available product.
* We can monitor the live water level in the tank and the water pump status using the UI as displayed below.
* We can set the water level according to our preference.
* This prototype can be used on any water tank(maximum length 400cm), and the length of the water tank can be set by the app itself, as shown below.
* This prototype can be used with telegram messaging app for all the live updates.
* This prototype has an auto-connect feature, which allows it to work even after a power cut or any other possible disruption. 
* Multiple more features can be added to this prototype, such as integration with smart voice assistants.
      



## Prototype Features: ##
* Automatically maintains water level between 20%-95% of the length of the tank. 
* Allows users to view real-time water level graph using the app. 
* Allows users to set the desired water level by simply giving a command(shown in the working part).
* Allows users to view real-time water pump status using the app.
* Can be configured with Telegram messaging app to get real-time updates.


## Prototype Requirements: ##
* Nodemcu or Wemos d1 mini(any esp8266 based microcontrolller)
* Ultrasonic sensor(HC-SR04 used in this prototype)
* Computer with Arduino IDE and all the required libraries(Refer to code)
* Micro USB Cable for nodemcu
* 5V power supply
* WIFI siginal till Nodemcu
* 5V DC Relay
* Water pump for filling up water
* Smartphone with telegram messaging app installed
* Jumper wires 
* Normal electrical wires 
      
 

## Prototype Working(Step By Step): ##

### USING APP ###

* (STEP1) Set the length of the water tank and the number of divisions we want to see in the live tank graph by inputting the value in the app. Example: Assume the water tank length to be 1000mL, and we want to see 10 divisions in the live water level graph, then enter 10 in the number of divisions. (Screenshot below)

![alt text](https://github.com/shubhamxbajaj/Smart-Water-Controller/blob/main/screenshots/WhatsApp%20Image%202022-04-22%20at%2011.08.21%20PM.jpeg)

* (STEP2) We get two options on the app. We can either reset the length of the water tank or view the live water status in the tank. (Screenshot below)

![alt text](https://github.com/shubhamxbajaj/Smart-Water-Controller/blob/main/screenshots/main_menu.jpg)

* (STEP3) If we press the "RESET GRAPH PARAMETERS" button, we will be sent to step 1 page. If we press the "LIVE WATER GRAPH" we will be sent to the Live water graph page. (Screenshot Below)

![alt text](https://github.com/shubhamxbajaj/Smart-Water-Controller/blob/main/screenshots/current_water_level_graph.jpg)

* Here we have a square box(this depicts the water tank), and we have the number of divisions as inputted in step 1. We will be able to see the live water status in the tank. NOTE: The graph is not measured as per scale; hence divisions are not 100% accurate. 

* We have a water pump status indicator as well. This indicator will automatically turn red when the water pump is turned on.

* We can set the water level according to our requirement by just sending a command using the "Set Water Level" Button on the Live Water Graph page. NOTE: This command will only work when the set water level value is more than the current water level and when the water pump is turned off. 

## USING TELEGRAM BOT ##

* We can use Telegram Bot as well to monitor the water tank. The following screenshots show the working of it:








