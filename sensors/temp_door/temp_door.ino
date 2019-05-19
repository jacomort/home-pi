#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <DHT.h>

// The DHT data line is connected to pin 2 on the Arduino
#define DHTPIN 2

// Leave as is if you're using the DHT22. Change if not.
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);


// Magnetic Door Sensor variable
byte switchPin = 4;

// Radio with CE & CSN connected to pins 7 & 8
RF24 radio(7, 8);
RF24Network network(radio);

// Constants that identify this node and the node to send data to
const uint16_t this_node = 2;
const uint16_t parent_node = 0;

// Time between packets (in ms)
const unsigned long interval = 1000;  // every sec
const int message_delay = 300; //every 5 minutes
int message_delay_counter = 0; //increments every loop

// Structure of our message
struct message_1 {
  float temperature;
  float humidity;
  bool dooropen;
};
message_1 message;

// The network header initialized for this node
RF24NetworkHeader header(parent_node);

void setup(void)
{
  // Set up the Serial Monitor
  Serial.begin(9600);

  // Initialize all radio related modules
  SPI.begin();
  radio.begin();
  delay(5);
  network.begin(90, this_node);

  // Initialize the DHT library
  dht.begin();
  
  // Activate the internal Pull-Up resistor for the door sensor
  pinMode(switchPin, INPUT_PULLUP);

}

void loop() {

  // Update network data
  network.update();

  // Read humidity (percent)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
   
  // Read door sensor: HIGH means door is open (the magnet is far enough from the switch)
  bool d = (digitalRead(switchPin) == HIGH);
  
  // Headers will always be type 1 for this node
  // We set it again each loop iteration because fragmentation of the messages might change this between loops
  header.type = '1';

  // Only send values if any of them are different enough from the last time we sent or 5 minutes have passed:
  //  0.5 degree temp difference, 1% humdity or light difference, or different motion state
  if (abs(f - message.temperature) > 0.5 || 
      abs(h - message.humidity) > 1.0 ||
      d != message.dooropen ||
      message_delay_counter > message_delay) {
    // Construct the message we'll send
    message = (message_1){ f, h, d };
    int message_delay_counter = 1;
    // Writing the message to the network means sending it
    if (network.write(header, &message, sizeof(message))) {
      Serial.print("Message sent\n"); 
    } else {
      Serial.print("Could not send message\n"); 
    }
  }

  // Wait a bit before we start over again
  ++message_delay_counter; 
  delay(interval);
}
