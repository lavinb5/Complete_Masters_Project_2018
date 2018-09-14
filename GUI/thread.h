#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include "MQTTClient.h"

#define ADDRESS     "tcp://10.10.101.9"
#define CLIENTID    "SubQt"
#define AUTHMETHOD  "lavinb"
#define AUTHTOKEN   "pass"
#define TOPIC       "test"
#define PAYLOAD     "Hello World!"
#define QOS         2
#define TIMEOUT     10000L


class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0);
    void run();

signals:
    void message_pass(QString payload);
    void mqqt_frame_pass(unsigned char, int, unsigned char*);
private:
    MQTTClient client;
    volatile MQTTClient_deliveryToken deliveredtoken;

    friend void delivered(void *context, MQTTClient_deliveryToken dt);
    friend int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    friend void connlost(void *context, char *cause);
};

void delivered(void *context, MQTTClient_deliveryToken dt);
int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);

#endif // THREAD_H
