#include "Writer.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include <iostream>
using namespace std;


Writer::Writer() {
	// TODO Auto-generated constructor stub

}

Writer::~Writer() {
	// TODO Auto-generated destructor stub
}

void Writer::write_frame(unsigned char frametype, int paylen, unsigned char *pay){

	MQTTClient client;
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.username = AUTHMETHOD;
	opts.password = AUTHTOKEN;
	int rc;
	if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
		cout << "Failed to connect, return code " << rc << endl;
	}

	int framelen = paylen + 5;
	unsigned char frame[framelen] = {0};
	unsigned char delimter = 1;
	unsigned char checksum = paylen + delimter;
	unsigned char clientID = 1;
	int frameinc = 0;
	frame[frameinc] = delimter;
	frameinc++;
	frame[frameinc] = clientID;
	frameinc++;
	frame[frameinc] = frametype;
	frameinc++;
	frame[frameinc] = paylen;
	for(int i=0; i<paylen; i++)
	{
		frameinc++;
		frame[frameinc] = pay[i];
		cout<< (int)pay[i] << endl;
	}
	frameinc++;
	//cout<< "Frame inc: " << frameinc << endl;
	frame[frameinc] = checksum;
	cout<< "Checksum: " << (int)frame[frameinc] << endl;

	pubmsg.payload = frame;
	pubmsg.payloadlen = framelen;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
	cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
			" seconds for publication of " << pay <<
			" \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	cout << "Message with token " << (int)token << " delivered." << endl;
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
}


