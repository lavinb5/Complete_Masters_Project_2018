#include "thread.h"
#include <QtCore>
#include <string>
#include <iostream>
using namespace std;

Thread *handle;

Thread::Thread(QObject *parent) : QThread(parent)
{
    ::handle=this;
}

void Thread::run()
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        qDebug() << "Callbacks set correctly";
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        qDebug() << "Failed to connect, return code " << rc;
    }
    qDebug() << "Subscribing to topic " << TOPIC << " for client " << CLIENTID;
    int x = MQTTClient_subscribe(client, TOPIC, QOS);
    qDebug() << "Result of subscribe is " << x <<" (0=success)";
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    (void)context;
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;

    char *payptr;
    payptr =(char*) message->payload;
    unsigned char delimiter = *payptr;
    unsigned char clientID = *(payptr+1);
    if(delimiter == 1 && clientID != 0)
    {
        qDebug() << "Client ID: " << (int)clientID;
        unsigned char frametype = *(payptr+2);
        qDebug() << "Fame type: " << (char)frametype;
        unsigned char paylen = *(payptr+3);
        qDebug() << "Payload length: " << (int)paylen;
        unsigned char pay[255];
        int inc=0;
        for(int i=4; i<paylen+4;i++)
        {
            qDebug() << (int)*(payptr+i);
            pay[inc] = *(payptr+i);
            inc++;
        }
        unsigned char checksum = *(payptr+(paylen+4));
        qDebug() << "checksum: " << checksum;

        if(checksum == (paylen + delimiter))
        {
            qDebug() << "Frame ok";
            // send signal to slot in main
           //handle->emitHandler(frametype, paylen, pay);
            emit handle->mqqt_frame_pass(frametype, paylen, pay);
        }
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    (void)context; (void)*cause;
    qDebug() << "Connection Lost" << endl;
}
