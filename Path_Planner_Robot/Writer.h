/*
 * Writer.h
 *
 *  Created on: 10 Aug 2018
 *      Author: root
 */

#ifndef WRITER_H_
#define WRITER_H_


#define ADDRESS    "tcp://10.10.101.9"
#define CLIENTID "pubRPI"
#define AUTHMETHOD "lavinb"
#define AUTHTOKEN  "pass"
#define TOPIC      "test"
#define QOS        1
#define TIMEOUT    10000L

class Writer {
public:
	Writer();
	virtual ~Writer();
	void write_frame(unsigned char, int, unsigned char*);
};

#endif /* WRITER_H_ */
