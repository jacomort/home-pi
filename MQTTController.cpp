#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <mosquittopp.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Constants that identify nodes
typedef enum {
	pi_node = 0,
	node1 = 1,
	node2 = 2,
	node3 = 3,
	node4 = 4,
} node_id;

const char* action_channel1 = "home/bedroom/light";
const char* action_channel2 = "home/kitchen/light";

// CE Pin, CSN Pin, SPI Speed
RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

// Time between checking for packets (in ms)
const unsigned long interval = 1000;

// Structure of our messages
struct message_action {
	bool state;
};

struct message_1 {
	float temperature;
	float humidity;
        bool dooropen;
};

// Mosquitto class
 class MyMosquitto : public mosquittopp::mosquittopp {
	public:
		MyMosquitto() : mosquittopp::mosquittopp ("PiBrain") { MyMosquitto::lib_init(); }
		
		virtual void on_connect (int rc) { printf("Connected to Mosquitto\n"); }
		
		virtual void on_disconnect () { printf("Disconnected\n"); }
		
		virtual void on_message(const struct mosquitto_message* mosqmessage) {
			// Message received on a channel we subscribe to
			printf("Message found on channel %s: %s\n", mosqmessage->topic, mosqmessage->payload);
			
			// Create message to send via RF24
			message_action actionmessage;
			if (strcmp((char*)mosqmessage->payload, "0") == 0) {
				actionmessage = (message_action){ false };
			} else if (strcmp((char*)mosqmessage->payload, "1") == 0) {
				actionmessage = (message_action){ true };
			} else {
				printf("Unknown message: %s\n", mosqmessage->payload); 
				return;
			}
			// Determine target node based on channel
			uint16_t target_node;
			if (strcmp(mosqmessage->topic, action_channel1) == 0) {
				target_node = node1;
			} else if (strcmp(mosqmessage->topic, action_channel2) == 0) {
				target_node = node2;
			}
			
			printf("Sending instructions to node %i\n", target_node);
			
			// Send message on RF24 network
			RF24NetworkHeader header(target_node);
			header.type = '2';
			if (network.write(header, &actionmessage, sizeof(actionmessage))) {
				printf("Message sent\n"); 
			} else {
				printf("Could not send message\n"); 
			}
		}
 };
 
 MyMosquitto mosq;

int main(int argc, char** argv)
{
	// Initialize all radio related modules
	radio.begin();
	delay(5);
	network.begin(90, pi_node);
	
	// Print some radio details (for debug purposes)
	radio.printDetails();
	
	network.update();
	
	mosq.connect("127.0.0.1");
	mosq.subscribe(0, action_channel1);
	mosq.subscribe(0, action_channel2);
	
	while (true) {
		// Get the latest network info
		network.update();
		printf(".\n");
		// Enter this loop if there is data available to be read,
		// and continue it as long as there is more data to read
		while ( network.available() ) {
			RF24NetworkHeader header;
			network.peek(header);
                        printf("header %c\n", header.type);
			// Have a peek at the data to see the heder
			// We can only handle type 1 sensor nodes for now should be a case statement
			switch (header.type)
			{
			case '1':
				message_1 sensormessage;
				network.read(header, &sensormessage, sizeof(sensormessage));
				char buffer [50];
				printf("Node: %i", header.from_node);
				printf("Temp: %i", sensormessage.temperature);
				printf("Humidity: %i", sensormessage.humidity);
				printf("Door: %i", sensormessage.dooropen);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,temperature,%f\"", header.from_node, sensormessage.temperature);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,humidity,%f\"", header.from_node, sensormessage.humidity);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,dooropen,%f\"", header.from_node, sensormessage.dooropen);
				break;
			case '2':
				message_1 sensormessage;
				network.read(header, &sensormessage, sizeof(sensormessage));
				char buffer [50];
				printf("Node: %i", header.from_node);
				printf("Temp: %i", sensormessage.temperature);
				printf("Humidity: %i", sensormessage.humidity);
				printf("Door: %i", sensormessage.dooropen);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,temperature,%f\"", header.from_node, sensormessage.temperature);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,humidity,%f\"", header.from_node, sensormessage.humidity);
				sprintf (buffer, "mosquitto_pub -t home/sensors -m \"%i,dooropen,%f\"", header.from_node, sensormessage.dooropen);
				break;
			default:
				printf("Unknow header: %c\n", header.type);
				break;
			}

		}
		
		// Check for messages on our subscribed channels
		mosq.loop();
		
		delay(interval);
	}
	
	mosq.unsubscribe(0, action_channel1);
	mosq.unsubscribe(0, action_channel2);

	// last thing we do before we end things
	return 0;
}
