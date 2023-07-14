# Grove-Wio-E5 Basic example
Basic Arduino-coded example of Seed Studio module Grove-Wio-E5, which uses the system on chip STM32WLE5JC from STM.\
This example was made based on Seed Studio's original example (https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version) with a few modifications in order to properly work with Arduino Nano 33 BLE Sense board (https://docs.arduino.cc/hardware/nano-33-ble-sense).
#### Notes:
  * Tested with Arduino 1.8.x IDE.
  * Example fully works with LoRa Gateways.
#### Advanced example:
If you are interested in a more complex application or use of the Grove-Wio-E5 module, an advanced example version will be released in the following weeks in another repository.
## Hardware Setup:
This project uses the Arduino Nano 33 BLE Sense board (it works with any of the revisions of this board)\
If you have a Grove shield (https://wiki.seeedstudio.com/Grove_System/#how-to-connect-grove-to-your-board),
then connect your Arduino Nano 33 BLE Sense to the shield and then plug your Grove-Wio-E5 into the UART slot of the Grove Shield\
If you don't have any Grove Shield, here is a picture of how to connect the board directly as shown:
<img src="https://github.com/andresoliva/Grove-Wio-E5/blob/main/pictures/nano33blesende_grove_e5_connection.png" width="450" align="center">
## Software codes:
In this project, there are two codes available:
Nano33BLE_Grove_LoRa: Same example as shown in the original Seed studio example ( https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-preparation) but compatible with Nano 33 BLE sense and without the code related to the display.
Nano33BLE_Grove_LoRa_time_measures: This project features some modifications to the "Nano33BLE_Grove_LoRa" in order to allow the user to:
  * Disable all the messages printed to the user with the UART port.
  * Measure and show the time it took to the Grove-Wio-E5 module to execute the commands sent and the time to get the ACK of the Gateway (if applicable, because some of the commands do not need a Gateway response).
  * Sends 12 bytes instead of 4. This was made to allow you to properly compare the time-on-Air of your sent message with tools that use this payload as a "standard" way to perform this calculation. See https://avbentem.github.io/airtime-calculator/ttn/eu868 (and the repository) for more information regarding this topic. 
     


