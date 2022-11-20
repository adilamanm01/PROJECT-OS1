#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "helper.h"

#define CUSTOMER_MODE_SHOPPING 32
#define CUSTOMER_MODE_SOFA 33
#define CUSTOMER_MODE_WAITING 34
#define CUSTOMER_MODE_UNDEFINED 35
#define CUSTOMER_MODE_SHOPPING_WAITING 36
#define CUSTOMER_EXIT 40




int nass=0;
int nsofa=0;
int nmax=0;
int sockfd;
int event=0;
int assistNo=0;

int waitingNo=0;
int sofaNo=0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t event_lock = PTHREAD_MUTEX_INITIALIZER;


struct sockaddr_in serveraddr;

void *loopfunction(void *arg);
void *recvmainsocket(void *arg);
void addtoClientInfoBase(struct Customer val);

int main(int argc, char *argv[]){



	printf("This is jewellery server\n");fflush(stdout);

	if(argc!=4){
		printf("Wrong arguments; please enter correct arguments [Na] [Ns] [Nmax]\n");exit(1);
	}

	if(readFile()==-1){
		printf("Error in Reading Cartier_catalog.txt\n");exit(1);
	}

	front = NULL;
	rear = NULL;

	int serverport=50012;

	pthread_t recvth_id,loop_tid;

	nass=(atoi)(argv[1]);
	nsofa=(atoi)(argv[2]);
	nmax=(atoi)(argv[3]);

	printf("Nass:%d\tNs:%d\tNmax:%d\n",nass,nsofa,nmax);fflush(stdout);

	// Creating socket file descriptor
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	const int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	    perror("setsockopt(SO_REUSEADDR) failed");

	memset(&serveraddr, 0, sizeof(serveraddr));

	// Filling server information
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(serverport);

	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0 )
	{
		perror("Bind Gmae");
		exit(1);
	}
	pthread_create(&recvth_id, NULL, recvmainsocket, NULL);
	pthread_create(&loop_tid, NULL, loopfunction, NULL);

	pthread_join(recvth_id, NULL);
	pthread_join(loop_tid, NULL);

	return 0;
}



void *recvmainsocket(void *arg)
{
	char rcvbuf[1024];
	char sndbuff[4096];
	printf("I am recv thread\n");fflush(stdout);
	struct sockaddr_in raddr;
	socklen_t rlen=sizeof(raddr);
	int customerId=0;
	while(1)
	{
		int numbytes=recvfrom(sockfd, (char *)rcvbuf, sizeof(rcvbuf),0, (struct sockaddr *) &raddr,&rlen);
		rcvbuf[numbytes] = '\0';
		printf("CLIENT Send:%s\n", rcvbuf);
		if(strncmp(rcvbuf, "REG:",4)==0){
			printf("Its a registration request from client\n");fflush(stdout);
			if(customerId < nmax)
			{
				customerId++;
				struct Customer c1;
				c1.customer_identcode=customerId;
				c1.customer_addr=raddr;

				if(customerId <= nass){
					assistNo++;
					c1.customer_assistNo=assistNo;

					c1.customer_mode=CUSTOMER_MODE_SHOPPING;
					sprintf(sndbuff,"Welcome to the shop..\n\nI am Shop Assistant#%d currently serving you.\nWhat would you like to do today? Please choose the option below:\n1. Looking at the jewelry menu\n2. Making specific jewelry inquiry\n3. Making purchase\n4. Returning the purchase\n5. Exit",assistNo);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
				}else if(customerId <= nsofa){
					sofaNo++;
					waitingNo++;
					c1.customer_sofaNo=sofaNo;
					c1.customer_waitingNo=waitingNo;
					c1.customer_mode=CUSTOMER_MODE_WAITING;
					sprintf(sndbuff,"All the shop assistants are currently busy assisting the customers.\nYou are placed in the waiting room.\nYour current number in the waiting list is %d.\nSofa #: %d\nDo you want to wait or want to leave the shop?",waitingNo,waitingNo);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));


				}else{
					waitingNo++;
					c1.customer_waitingNo=waitingNo;
					c1.customer_mode=CUSTOMER_MODE_WAITING;
					sprintf(sndbuff,"All the shop assistants are currently busy assisting the customers.\nYou are placed in the waiting room.\nYour current number in the waiting list is %d.\nDo you want to wait or want to leave the shop?",waitingNo);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
				}



				if(assistNo>=nass)
				{
					pthread_mutex_lock(&event_lock);
						event=1;
					pthread_mutex_unlock(&event_lock);
				}

				pthread_mutex_lock(&lock);
					addtoClientInfoBase(c1);
				pthread_mutex_unlock(&lock);
			}
			else
			{
				//more customers than range
				printf("Going to drop this customer\n");
				sprintf(sndbuff,"Sorry, the waiting room is full.. We will not be able to serve you at the moment.\nPlease check back later.. Thank you..");
				sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
			}
		}else if(strncmp(rcvbuf,"exit",4)==0){
			printf("This customer has decdied to exit\n");
			customerId--;
		}else if(strncmp(rcvbuf,"5",1)==0){
			int buyerleft=0;

			int assista=0;
			waitingNo=0;
			printf("This customer has decdied to exit\n");
			customerId--;
			//need to traverse queue
			pthread_mutex_lock(&lock);
				struct node *temp = front;
				while(temp)
				{
					if(temp->data.customer_addr.sin_port==raddr.sin_port){
						if(temp->data.customer_mode==CUSTOMER_MODE_SHOPPING){
							printf("This leaving customer was attended by assistant\n");
							buyerleft=1;
							assista=temp->data.customer_assistNo;
						}
						temp->data.customer_mode=CUSTOMER_EXIT;
					}
					temp = temp->next;
				}
			pthread_mutex_unlock(&lock);

			pthread_mutex_lock(&lock);
				struct node *temp1 = front;
				while(temp1)
				{
					if(temp1->data.customer_mode==CUSTOMER_MODE_WAITING)
					{
						if(buyerleft==1){
							buyerleft=0;
							//make this enter into game
							printf("Adding it to assistant\n");
							temp1->data.customer_mode=CUSTOMER_MODE_SHOPPING;
							sprintf(sndbuff,"Welcome to the shop..\n\nI am Shop Assistant#%d currently serving you.\nWhat would you like to do today? Please choose the option below:\n1. Looking at the jewelry menu\n2. Making specific jewelry inquiry\n3. Making purchase\n4. Returning the purchase\n5. Exit",assista);
							sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &temp1->data.customer_addr,sizeof(temp1->data.customer_addr));
							continue;
						}
						waitingNo++;
						if(waitingNo<=nsofa){
							sprintf(sndbuff,"All the shop assistants are currently busy assisting the customers.\nYou are placed in the waiting room.\nYour current number in the waiting list is %d.\nSofa #: %d\nDo you want to wait or want to leave the shop?",waitingNo,waitingNo);
							sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &temp1->data.customer_addr,sizeof(temp1->data.customer_addr));
						}else{
							sprintf(sndbuff,"All the shop assistants are currently busy assisting the customers.\nYou are placed in the waiting room.Your current Number is%d\nDo you want to wait or want to leave the shop?",waitingNo);
							sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &temp1->data.customer_addr,sizeof(temp1->data.customer_addr));
						}
						//update all waiting buyers

					}
					temp1 = temp1->next;
				}
			pthread_mutex_unlock(&lock);

			///////////
		}else if(strncmp(rcvbuf,"1",1)==0){
			printf("This client wants to look at the jewellery menu\n");
//			int citr=0;
			for(int i=0;i<47;i++)
			{
				sprintf(sndbuff,"%s\t%s\t%s\t%s\t%d",catalogstr[i].ref,catalogstr[i].category,catalogstr[i].title,catalogstr[i].tags,catalogstr[i].price);
				sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
			}
			sprintf(sndbuff,"What would you like to do today? Please choose the option below:\n1. Looking at the jewelry menu\n2. Making specific jewelry inquiry\n3. Making purchase\n4. Returning the purchase\n5. Exit");
			sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));

		}else if(strncmp(rcvbuf,"ref:",4)==0){
			printf("This customer has decdied to make specific jewellery inquiry about");fflush(stdout);
			char refnewItem[20];
			memcpy(refnewItem, &rcvbuf[4],sizeof(refnewItem));fflush(stdout);
			printf("item:%s\n",refnewItem);
			for(int i=0;i<47;i++)
			{
				if(strcmp(catalogstr[i].ref,refnewItem)==0){
					printf("Client want to ask about this");
					sprintf(sndbuff,"Ref\tCategory\tTitle\tTags\tDescription\tPrice\n%s\t%s\t%s\t%s\t%s\t%d",catalogstr[i].ref,catalogstr[i].category,catalogstr[i].title,catalogstr[i].tags,catalogstr[i].description,catalogstr[i].price);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
				}

			}
		}else if(strncmp(rcvbuf,"buy:",4)==0){
			printf("This customer has decdied to Buy");fflush(stdout);
			char refnewItem[20];
			int item=1;
			memcpy(refnewItem, &rcvbuf[4],sizeof(refnewItem));fflush(stdout);
			printf("item:%s\n",refnewItem);
			for(int i=0;i<47;i++)
			{
				if(strcmp(catalogstr[i].ref,refnewItem)==0){
					printf("Client want to Buy this= decrementing its quantity");fflush(stdout);
					catalogstr[i].quantity=catalogstr[i].quantity-1;
					sprintf(sndbuff,"##***CARTIER CATALOG JEWELERY STORE***###\nItem\tCategory\tDescription\tPrice\n%d:\t%s\t%s\t%d\nTotal Bill Paid:%d\n",item,catalogstr[i].category,catalogstr[i].description,catalogstr[i].price,catalogstr[i].price);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
					item++;
				}
			}
		}else if(strncmp(rcvbuf,"ret:",4)==0){
			printf("This customer has decdied to Return");fflush(stdout);
			char refnewItem[20];
			int item=1;
			memcpy(refnewItem, &rcvbuf[4],sizeof(refnewItem));fflush(stdout);
			printf("item:%s\n",refnewItem);
			for(int i=0;i<47;i++)
			{
				if(strcmp(catalogstr[i].ref,refnewItem)==0){
					printf("Client want to Return this= decrementing its quantity");fflush(stdout);
					catalogstr[i].quantity=catalogstr[i].quantity+1;
					sprintf(sndbuff,"##***CARTIER CATALOG JEWELERY STORE***###\nThis item%s has been returned by customer and add in list \nAmount Returned:%d\n",catalogstr[i].category,catalogstr[i].price);
					sendto(sockfd, (const char *)sndbuff, sizeof(sndbuff),0, (const struct sockaddr *) &raddr,sizeof(raddr));
					item++;
				}
			}
		}else{
			//
		}
	}
}

void *loopfunction(void *arg){
	int res=0;
	char sendbuff[1024];
	while(1){
		pthread_mutex_lock(&event_lock);
			res=event;
			event=0;
		pthread_mutex_unlock(&event_lock);
		if(res==1){
			res=0;
			//need to read queue
			pthread_mutex_lock(&lock);

				struct node *temp = front;
				while(temp)
				{
					if(temp->data.customer_mode==CUSTOMER_MODE_SHOPPING){
						temp->data.customer_assistNo--;
					}
					temp = temp->next;
				}

			pthread_mutex_unlock(&lock);
		}
		usleep(250000);
	}
}
