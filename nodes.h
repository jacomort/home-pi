#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED



#endif // NODES_H_INCLUDED

typedef enum {
    pi_node = 0,
    node1 = 1,
    node2 = 2,
    node3 = 3,
    node4 = 4
} node_id ;

typedef enum {
    rx_fmt_0 = 0,
    rx_fmt_1 = 1,
    rx_fmt_2 = 2,
    rx_fmt_3 = 3
} messafe_fmt ;

struct message_t1 {
	float temperature;
	float humidity;
	bool dooropen;
};

void print_message_1 (message_t1 sensormessage) {
    printf("Temp: %f\n", sensormessage.temperature);
    printf("Humidity: %f\n", sensormessage.humidity);
    printf("Door: %i\n", sensormessage.dooropen);
    sprintf (buffer, "mosquitto_pub -t home/bedroom/temperature -m \"%f\"", sensormessage.temperature);
    system(buffer);
    sprintf (buffer, "mosquitto_pub -t home/bedroom/humidity -m \"%f\"", sensormessage.humidity);
    system(buffer);
    sprintf (buffer, "mosquitto_pub -t home/bedroom/dooropen -m \"%i\"", sensormessage.dooropen);
    system(buffer
} ;
