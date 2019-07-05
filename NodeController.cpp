#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <mosquittopp.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <nodes.h>

// Constants that identify nodes
const uint16_t pi_node = 0;
const uint16_t node1 = 1;
const uint16_t node2 = 2;
const uint16_t node3 = 3;
const uint16_t node4 = 4;

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
			switch(header.type) {
			    case rx_fmt_1:
                    message_t1 sensormessage;
                    break;
                case rx_fmt_2:
                    message_t1 sensormessage;
                    break;
                default:
                    printf("Unrecognized message type %i\n", header.type);
                    break;
			}
            // Read the message
            network.read(header, &sensormessage, sizeof(sensormessage));
            // Print it out in case someone's watching
            printf("Data received from node %i\n", header.from_node);
            char buffer [50];
            switch (header.from_node) {
                case node1:
                    print_message_1 (sensormessage);
                    break;
                case node2:
                    print_message_1 (sensormessage);
                    break;
                // add as many case statements as you have nodes
                default:
                    printf("Unknown node %i\n", header.from_node);
                    break;
            }
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
