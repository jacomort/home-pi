#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <nodes.h>


// CE Pin, CSN Pin, SPI Speed
RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

// Time between checking for packets (in ms)
const unsigned long interval = 1000;

// Structure of our messages
struct message_action {
	bool state;
};


int main(int argc, char** argv)
{
	// Initialize all radio related modules
	radio.begin();
	delay(5);
	network.begin(90, pi_node);
	
	// Print some radio details (for debug purposes)
	radio.printDetails();
	
	network.update();
	
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
			if (header.type == '1') {
                            message_t1 sensormessage;
			    // Read the message
			    network.read(header, &sensormessage, sizeof(sensormessage));
			    // Print it out in case someone's watching
			    printf("Data received from node %i\n", header.from_node);
                            printf("node: %i, Temp: %f\n", header.from_node, sensormessage.temperature);
                            printf("node: %i, Humidity: %f\n", header.from_nod, sensormessage.humidity);
                            printf("node: %i, Door: %i\n", header.from_nod, sensormessage.dooropen);
		            // read other sensor data from node 2 here
		
			} else {
				// This is not a type we re
                                printf("Unknow header: %c\n", header.type);
				//printf("Unknown message received from node %i\n", header.from_node);
			}
		}
		
		// Check for messages on our subscribed channels
		delay(interval);
	}
	

	// last thing we do before we end things
	return 0;
}
