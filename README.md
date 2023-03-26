# VAWT
Contains code of my vertical axis wind turbine capstone project. A brief write-up of this work can be found here (https://digital-library.theiet.org/content/conferences/10.1049/icp.2021.0923)

Here are a few flowcharts that summarize the work I did. 

![image](https://user-images.githubusercontent.com/47520410/227806123-272597d1-6830-4187-8a5f-c82e537be278.png)

The Arduino Mega station uses multiple hardware serial ports to communicate with different devices, with Serial0 for debugging, Serial1 for the GPS module, and Serial2 for the WiFi module. Setting up the baud rate is crucial for stable functioning. The LCD module is also set up with 19 I/O pins defined and variables declared. The station sends an SMS to the admin to notify them that it's active.

Data stored in the SD card includes a unique API key and the admin's phone number for security purposes. The station only interacts with the admin's phone number, and a variable called buf_state defines whether the station is stationary or moving. The function millis() is used to define the current time with the variable prevTime to control the frequency of data uploads.

The function gradualStartUp() is called to give the station a kick to overcome inertia when the blades are stationary. The program then checks the SMS inbox, instantaneous voltage, and temperature, triggering fail safes if necessary. The function totalShutDown() is triggered if any fail safes are activated, and the only way to exit a shutdown condition is if the user sends an SMS with the message "restart" using the function checkSMS().

The GSM module operates in SMS mode, and the AT command "AT+CNMI=2,2,0,0,0\r" ensures that any received SMS is moved to the Serial ports connected to the module. To prevent buffer overflow and loss of data, the AT command "AT+CNMI=0,0,0,0,0\r" is used to stop AT responses from reaching the serial ports of the controller, and the function checkSMS() periodically enters the AT command "AT+CNMI=2,2,0,0,0\r" to ensure that any SMS received is only received when checkSMS() is called.

In addition, checkRPM() function is  executed in several parts of the program. As the name suggests, it checks the RPM, and disconnects the load if the RPM is decreasing and connects the load once the RPM reaches a certain threshold. In case there is a lack of wind, the station shuts down and it also ensures that the slow wind is also used to generate power instead of letting it go to waste

When none of these failsafes are triggered, the function normalCondition() is executed. The flowchart and explanation are as follows:


![image](https://user-images.githubusercontent.com/47520410/227806350-c4dc8667-7df1-41bb-9f95-46f4605bba28.png)

The first task involves sampling and displaying data, which is then stored on an SD card in a text file named logs.txt. All the samples are calculated and physically stored by the controller, which enables users to later visualize the performance of the module on programs like Microsoft Excel using the “Text” option in the “Get external data” option on the program.

However, the data is uploaded only when the system is in the normalCondition() and at regular intervals, rather than sending every single bit of data. This is done to reduce the amount of data that will be billed and because sending every bit of data is redundant as the change in voltage and current is gradual and not spontaneous and/or rapid. Data gets uploaded when the time since the last upload (prevTime) exceeds uploadDelayTime, and every time the data is uploaded, the variable prevTime is updated to the current time. The data that is uploaded is an average of the samples taken.

Once the samples have been averaged, the program checks if the system is stationary or not. If it is, a few steps are omitted during execution to save time.

The user can upload data using two options, WiFi or GSM. When uploading using WiFi, the GSM is used for receiving SMS from the admin only. In both the methods the data is encapsulated into an HTTP request message with the data sent in a format that is understandable by Thingspeak.

For uploading data using GSM, several steps must be taken in a systematic manner, such as checking if the SIM is unlocked by the pin number, checking if the SIM is registered to the network, and testing if the SIM is attached to the GPRS service. The GPRS PDP (Packet Data Protocol) Context must be deactivated using the AT command AT+CIPSHUT, and the current connection status must be queried using AT command AT+CIPSTATUS. Afterward, a single IP connection must be started using AT command AT+CIPMUX=0, and the APN (Access Point Name) must be set to create a gateway between the GSM module and the server using GPRS connection using AT command AT+CSTT="myAPN". Once the wireless connection with GPRS is established using AT command AT+CIICR, the local IP address is obtained using AT command AT+CIFSR, and the prompt of "send ok" when the module sends data is set using AT command AT+CIPSPRT=0. Finally, the type of connection, server name, and port number must be set up using AT command AT+CIPSTART="TCP","api.thingspeak.com","80" before the data is sent through TCP or UDP Connection using AT command AT+CIPSEND. The connection is then closed using AT command AT+CIPSHUT.

For uploading data using WiFi, users must connect to the WiFi network, check if the incoming serial data is to be ignored or stored, parse incoming serial data and stored in different variables such as Instantaneous Voltage, Instantaneous Voltage, Latitude, and so on. Users must then connect to the server api.thingspeak.com at port 80, send the data as an HTTP request message along with the API Key, and close the connection. The parsing of data is inspired by Robin2’s tutorial on an Arduino forum, which involves storing chunks of data between start and end markers to ensure users receive the correct data in between the markers.

Another interesting feature of this project is the low-windspeed utilisation. The flowchart and explanation of it are as follows:


![image](https://user-images.githubusercontent.com/47520410/227806690-ee2409b5-c8cc-466e-9cd3-696507dbf718.png)

The process of starting up a power generation station, particularly a wind turbine, involves a series of procedures and control mechanisms to ensure efficient and safe operation. In the case of wind turbines, there are certain conditions that need to be met before the turbine can start generating electricity. These conditions include sufficient wind speed, proper blade rotation, and motor operation.

However, activating the motor for a long time without sufficient air flow can lead to a loss in the generated energy. This is because the motor is responsible for driving the blades, which in turn produce electrical energy. If the blades do not have enough wind to keep rotating, the motor will continue to consume energy without producing any output. This can cause significant financial losses for the power generation company.

To address this issue, a control mechanism has been developed that continuously monitors the RPM (rotations per minute) of the blades and their acceleration. Instead of pushing the blades using the motor, the control mechanism disconnects the load to allow the generator to move freely. This helps the generator to regain momentum and start producing energy again.

If the generator is unable to regain momentum after a certain period of time, it is switched off and the operator is notified. On the other hand, if the generator is able to regain enough speed to work, the control mechanism continues to the next part of the program. This helps to optimize the performance of the wind turbine and ensure that energy is generated efficiently and safely.

To further improve the efficiency of the wind turbine, the control mechanism is designed to disconnect the load when the RPM of the blades is decreasing rather than when it is low. This is because the power can still be utilized if the RPM is low and increasing, as it is unlikely that the blades would slow down and stop producing energy. However, it would not be wise to take power from the turbine and slow it down when the blades are slowing down and the RPM is very low. In such cases, it is best to shut down the turbine to prevent damage.

Overall, the startup process of a wind turbine is a complex and intricate process that requires careful monitoring and control. By implementing the proper control mechanisms, wind turbine operators can ensure that their turbines operate efficiently and safely, maximizing the amount of energy generated and minimizing financial losses.
