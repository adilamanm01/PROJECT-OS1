
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

struct sockaddr_in serveraddr;
struct sockaddr_in cliaddr;
int sockfd;

void *recvmainsocket(void *arg);
void *sendfuction(void *arg);


int main(int argc, char *argv[]){
	pthread_t recvth_id,sendth_id;
	printf("This is jewellery client\n");fflush(stdout);
	int serverport=50012;
	int cliport=0;
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	const int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	    error("setsockopt(SO_REUSEADDR) failed");

	memset(&serveraddr, 0, sizeof(serveraddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(serverport);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cliaddr.sin_port = htons(cliport);

	// Bind the socket with the client address
	if(bind(sockfd, (const struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0 )
	{
		perror("Bind Gmae");
		exit(1);
	}

	pthread_create(&recvth_id, NULL, recvmainsocket, NULL);
	//pthread_create(&sendth_id, NULL, sendfuction, NULL);

	char regmsg[150];
	sprintf(regmsg,"REG:");
	sendto(sockfd, (const char *)regmsg, sizeof(regmsg),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
	char buf[1024];
	while(1)
	{
		printf("Type to send:\n");
		scanf("%s", buf);

		if(strncmp("exit",buf,4)==0){
			printf("Client has decided to leave the shop\n");
			sendto(sockfd, (const char *)buf, sizeof(buf),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
			exit(0);
		}else if(strncmp("2",buf,1)==0){
			printf("Enter ref Item to get more details:\n");
			scanf("%s", buf);
			sprintf(regmsg,"ref:%s",buf);
			sendto(sockfd, (const char *)regmsg, sizeof(regmsg),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
		}else if(strncmp("3",buf,1)==0){
			printf("Enter ref Item to BUY\n");
			scanf("%s", buf);
			sprintf(regmsg,"buy:%s",buf);
			sendto(sockfd, (const char *)regmsg, sizeof(regmsg),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
		}else if(strncmp("4",buf,1)==0){
			printf("Enter ref Item to Return\n");
			scanf("%s", buf);
			sprintf(regmsg,"ret:%s",buf);
			sendto(sockfd, (const char *)regmsg, sizeof(regmsg),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
		}else if(strncmp("5",buf,1)==0){
			printf("Client has decided to leave the shop\n");
			sendto(sockfd, (const char *)buf, sizeof(buf),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
			exit(0);
		}else{
			sendto(sockfd, (const char *)buf, sizeof(buf),0, (const struct sockaddr *) &serveraddr,sizeof(serveraddr));
		}

	}
	pthread_join(recvth_id, NULL);
	return 0;
}

void *recvmainsocket(void *arg)
{
	char rcvbuf[1024];
	char buf[1024];
	printf("I am recv thread\n");fflush(stdout);
	struct sockaddr_in raddr;
	socklen_t rlen=sizeof(raddr);
	while(1){
		int numbytes=recvfrom(sockfd, (char *)rcvbuf, sizeof(rcvbuf),0, (struct sockaddr *) &raddr,&rlen);
		rcvbuf[numbytes] = '\0';

		//printf("%s\n", rcvbuf);fflush(stdout);

		if(strncmp("Sorry, the waiting room is full",rcvbuf,31)==0){
			printf("Server>>>:%s\n", rcvbuf);
			exit(0);
		}

		else if(strncmp("Welcome to the shop.",rcvbuf,20)==0){
			printf("This client has entered the shop\n");fflush(stdout);
			printf("%s\n", rcvbuf);fflush(stdout);
		}else{
			printf("%s\n", rcvbuf);fflush(stdout);

		}
	}
}
