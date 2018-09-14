#include <unistd.h>
#include<pthread.h>
#include<iostream>
using namespace std;

#include "PathPlanner.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

//#include "Writer.h"

bool Frame_MUTEX = false;
unsigned char Frame_TYPE = 0;
unsigned char Frame_ARRAY[255] = {0};
int Frame_LENGTH = 0;

//#define ADDRESS     "tcp://10.10.101.9"
//#define CLIENTID    "rpi1"
//#define AUTHMETHOD  "lavinb"
//#define AUTHTOKEN   "pass"
//#define TOPIC       "test"
//#define PAYLOAD     "Hello World!"
//#define QOS         1
//#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
	cout << "Message Received, topic: " << topicName << endl;
	cout << "Message length: " << message->payloadlen << endl;

	unsigned char *msgptr;
	msgptr =(unsigned char*) message->payload;
	unsigned char delimiter = *msgptr;
	unsigned char clientID = *(msgptr+1);
	if(delimiter == 1 && clientID != 1)
	{
		cout<< "Client ID: " << (int)clientID << endl;
		unsigned char frametype = *(msgptr+2);
		cout<< "Fame type: " << (char)frametype << endl;
		unsigned char paylen = *(msgptr+3);
		cout<< "Payload length: " << (int)paylen << endl;
		int inc =0;
		unsigned char pay[255] = {0};
		for(int i=4; i<paylen+4;i++)
		{
			cout<< (int)*(msgptr+i) << endl;
			pay[inc] = *(msgptr+i);
			inc++;
		}
		unsigned char checksum = *(msgptr+(paylen+4));
		cout<< "checksum: " << (int)checksum << endl;

		if(checksum == (paylen + delimiter))
		{
			cout<< "Frame ok: Unlocking Mutex for main code" << endl;
			Frame_MUTEX = true;
			Frame_TYPE = frametype;
			Frame_LENGTH = paylen;
			for(int i = 0; i< Frame_LENGTH; i++)
			{
				Frame_ARRAY[i] = pay[i];
			}
		}
	}
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause) {
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}


void *thread(void *ptr)
{

	int type = (int) ptr;
	cout << type << endl;

	MQTTClient client;
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	int rc;
	int ch;

	MQTTClient_create(&client, "tcp://10.10.101.9", "rpi1", MQTTCLIENT_PERSISTENCE_NONE, NULL);
	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.username = "lavinb";
	opts.password = "pass";

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
	if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
			"Press Q<Enter> to quit\n\n", "test", "rpi1", 1);
	MQTTClient_subscribe(client, "test", 1);

	do {
		ch = getchar();
	} while(ch!='Q' && ch != 'q');
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	return ptr;
}

void *newthread(void *ptr)
{
	PathPlanner path1;
	while(1){
		while(Frame_MUTEX == true){
			cout << "Main Code: Mutex unclocked" << endl;
			Frame_MUTEX = false;
			cout << "Main Frame type: " << Frame_TYPE << endl;
			cout << "Main Frame length: " << Frame_LENGTH << endl;
			for(int i=0; i<Frame_LENGTH;i++)
			{
				cout << "[" << i << "]: " <<  (int)Frame_ARRAY[i] << endl;
			}
			//Writer wr;
			//wr.write_frame(Frame_TYPE, Frame_LENGTH, Frame_ARRAY);
			path1.received_frame(Frame_TYPE, Frame_LENGTH, Frame_ARRAY);
			Frame_TYPE = 0;
			Frame_LENGTH = 0;
			for(int i=0; i<Frame_LENGTH;i++) Frame_ARRAY[i] = 0;
			cout << "Main: Data cleared and Mutex locked again" << endl;
		}
	}
	return ptr;
}

int main()
{

	pthread_t thread1;
	pthread_t thread2;
	int thr = 1;
	int thr2 = 2;
	pthread_create(&thread1, NULL, *thread, (void *) thr);
	pthread_create(&thread2, NULL, *newthread, (void *) thr2);

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	cout << "After thread" << endl;
	// testing
	return 0;
}
