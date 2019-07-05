# home-pi
This is my home automation repo

Pysical setup
	Raspberry pi hub (nRF24L01+ Transceiver)
	Arduino nanos are nodes (nRF24L01+ Transceiver)
	
The Raspberry
	Listens on the RF interface
		recognized incoming messages are published to a MQTT broker
	Listens on MQTT
		sends recognized channels on the the corresponding node
		
		
# sensorDB
