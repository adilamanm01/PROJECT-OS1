/*
 * helper.h
 *
 *  Created on: Nov 17, 2022
 *      Author: sanhcd
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <netinet/in.h>

struct Customer{
	int customer_identcode;	//id
	int customer_mode;	//for result
	int customer_assistNo;
	int customer_sofaNo;
	int customer_waitingNo;
	struct sockaddr_in customer_addr;	//connection stored
};

struct node
{
    struct Customer data;
    struct node *next;
};

struct Catalog{
	char ref[20];
	char category[20];
	char title[30];
	char tags[50];
	char description[100];
	int quantity;
	int price;
};

struct Catalog catalogstr[47];	//array of catalog


struct node *front;
struct node *rear ;

void addtoClientInfoBase(struct Customer val);

int readFile();

#endif /* HELPER_H_ */
