#include <UIPUdp.h>
#include <UIPServer.h>
#include <UIPClient.h>
#include <ethernet_comp.h>
#include <Dns.h>
#include <Dhcp.h>
#include <Arduino.h>

#include <SPI.h>

// When shifting between ethernet drivers in Eclipse environment, remember to set this:
// Arduino -> Add a library to the selected project -> Ethernet/UIPEthernet (NOT ethercard!)
// Project -> Properties -> Arduino -> Arduino board selection -> Board: Arduino/Genuino (remember the serial port)
//#include <Ethernet.h>
#include <UIPEthernet.h> // Used for Nano/Shield

// **** ETHERNET SETTING ****
#define ALARM_ID  0x03  // When using UIPEthernet.h (= not Ethernet.h) this value is reflected in the DHCP naming: ENC28JBE0003 (you can run 'ping ENC28JBE0003')

byte mac[] = { 0xCA, 0xFE, 0xBA, 0xBE, 0x00, ALARM_ID };
int port = 12900;

EthernetServer server(port);

typedef enum StateTypeTag {
  STATE_NULL,
  STATE_DISABLED,
  STATE_SHELL,
  STATE_ARMED
} StateType;

StateType state = STATE_NULL;

String stateString = "notInitialized";

typedef enum AlarmStateTypeTag {
  ALARM_STATE_NULL,
  ALARM_STATE_ACTIVE
} AlarmStateType;

AlarmStateType alarmState = ALARM_STATE_NULL;
String alarmStateString = "alarm_state_null";
unsigned long blackoutTime = 0;

void setup() {

	Serial.begin(9600);

	#define IN_ALARM	17 // = A3
	#define OUT_RED     16 // = A2
	#define OUT_YELLOW  15 // = A1
	#define OUT_GREEN   14 // = A0

	#define IN_ARMED    7
 	#define IN_SHELL	6
	#define IN_DISABLED 5
	#define OUT_Z1      4
	#define OUT_Z2      3
	#define IN_PUSH		9	// pin #2 seems reserved (for ENC28J60 based ethernet shield?)


	pinMode(IN_ALARM, INPUT);
	pinMode(OUT_RED, OUTPUT);
	pinMode(OUT_YELLOW, OUTPUT);
	pinMode(OUT_GREEN, OUTPUT);

	pinMode(IN_ARMED, INPUT);
	pinMode(IN_SHELL, INPUT);
	pinMode(IN_DISABLED, INPUT);
	pinMode(OUT_Z1, OUTPUT);
	pinMode(OUT_Z2, OUTPUT);
	pinMode(IN_PUSH, INPUT);

	digitalWrite(OUT_RED, HIGH);   // Light up, to show that we are starting
	digitalWrite(OUT_YELLOW, HIGH);// Light up, to show that we are starting
	digitalWrite(OUT_GREEN, HIGH); // Light up, to show that we are starting
	digitalWrite(OUT_Z1, HIGH);    // Pulse HIGH -> LOW -> HIGH triggers alarm on Z1 even if disabled. 220R series resistor is OK. 1K8 is too much.
	digitalWrite(OUT_Z2, HIGH);    // Pulse HIGH -> LOW -> HIGH triggers alarm on Z2 if shell is enabled

	// Turn on pull up
#define ALARM_ANALOG
	digitalWrite(IN_ARMED, HIGH);
	digitalWrite(IN_SHELL, HIGH);
	digitalWrite(IN_DISABLED, HIGH);
	digitalWrite(IN_PUSH, HIGH);  // Pulls low for 3s when alarm is disabled (to allow a push-door to open)


	// start the Ethernet connection and the server:
#ifndef UIPETHERNET_H
	Ethernet.hostName((char *)"Alar2");
	// When UIPETHERNET is used, the name of the platform is ENC28JBE + macLast4digits, eg.: ENC28JBE0003
#endif
	Ethernet.begin(mac);
	server.begin();

	Serial.print("IP Address: ");
	Serial.println(Ethernet.localIP());
}

void lightUp() {
	if (millis() < blackoutTime) {
		return;
	}

	switch (state) {
		case STATE_NULL:
			digitalWrite(OUT_RED, LOW);
			digitalWrite(OUT_YELLOW, LOW);
			digitalWrite(OUT_GREEN, LOW);
			break;
		case STATE_DISABLED:
			digitalWrite(OUT_RED, LOW);
			digitalWrite(OUT_YELLOW, LOW);
			digitalWrite(OUT_GREEN, HIGH);
			break;
		case STATE_SHELL:
			digitalWrite(OUT_RED, LOW);
			digitalWrite(OUT_YELLOW, HIGH);
			digitalWrite(OUT_GREEN, LOW);
			break;
		case STATE_ARMED:
			digitalWrite(OUT_RED, HIGH);
			digitalWrite(OUT_YELLOW, LOW);
			digitalWrite(OUT_GREEN, LOW);
			break;
	}

}

void switchOffLeds() {
	digitalWrite(OUT_RED, LOW);
	digitalWrite(OUT_YELLOW, LOW);
	digitalWrite(OUT_GREEN, LOW);
}


void detectState() {

	StateType thisDetect = STATE_NULL;

	if (digitalRead(IN_ARMED)) {
		thisDetect = STATE_ARMED;
		stateString = "state_armed";
	} else if (digitalRead(IN_SHELL)) {
		thisDetect = STATE_SHELL;
		stateString = "state_shell";
	} else if (digitalRead(IN_DISABLED)) {
		thisDetect = STATE_DISABLED;
		stateString = "state_disabled";
	} else {
		thisDetect = STATE_NULL;
		stateString = "state_null";
	}

	state = thisDetect;

}

EthernetClient client;

void communicate() {
	#define CMD_STATE "state"
	#define CMD_INPUT "input"
	#define CMD_Z1    "z1"
	#define CMD_Z2    "z2"

	Ethernet.maintain(); // Keep my name updated in the DHCP server whenever the lease time expires

	// listen for incoming clients
	client = server.available();
	if (client)	{
		String command = "";

		while (client.connected()) {
			if (client.available()) {
				char c = client.read();

				if (c == '\n')	{
					// an http request ends with a blank line

					blackoutTime = millis() + 20; // Briefly blink to indicate communication detected
					switchOffLeds();

					String respons = CMD_STATE + String(" ") + CMD_INPUT + String(" ") + CMD_Z1 + String(" ") + CMD_Z2;

					if (command.equals(CMD_STATE)) {
						respons = stateString + String("  ") +  alarmStateString;
						Serial.println(respons);
						client.println(respons);
					} else if (command.equals(CMD_INPUT)) {
						returnInputStatus();
					} else if (command.equals(CMD_Z1)) {
						pulsePin(OUT_Z1);
						respons = "z1 OK";
						Serial.println(respons);
						client.println(respons);
					} else if (command.equals(CMD_Z2)) {
						pulsePin(OUT_Z2);
						respons = "z2 OK";
						Serial.println(respons);
						client.println(respons);
					} else {
						Serial.println(respons);
						client.println(respons);
					}
					command = String("");
				} else {
					command += String(c); // Build the command char by char until '\n' is found
				}
			}
		}

		// give the web browser time to receive the data
		delay(10);

		// close the connection:
		client.stop();
	}
}

void returnInputStatus() {
	int armed = digitalRead(IN_ARMED);
	int shell = digitalRead(IN_SHELL);
	int disa = digitalRead(IN_DISABLED);
	int alarm = analogRead(IN_ALARM);
	int door = digitalRead(IN_PUSH);

	char str[50];
	sprintf(str,  "armed:%d shell:%d disa:%d alarm(analog value):%d door:%d", armed, shell, disa, alarm, door);
	client.println(str);
	Serial.println(str);
}

void pulsePin(int pin) {
	digitalWrite(pin, LOW);    // Pulse HIGH -> LOW -> HIGH triggers alarm on Z1 even if disabled. 220R series resistor is OK. 1K8 is too much.
	delay(1000);
	digitalWrite(pin, HIGH);
}

void loop() {
	detectState();
	communicate();
	lightUp();
}
