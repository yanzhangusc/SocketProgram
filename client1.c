/*********************************************
Author: Yan Zhang
Email: zhan347@usc.edu
Date: Nov. 2013
This source code includes three functions including main, clientPhase2 and clientPhase3.
This project contains six source file: directory_server.c, file_server1.c, file_server2.c, 
file_server3.c, client1.c, client2.c, client3.c
If you have any questions, please contact me. Thank you.
**********************************************/
//rebase
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PHASE2PORT "32260" // cilent1's port in phase 1 
#define MAXBUFLEN 100
#define MAXDATASIZE 100 // max number of bytes we can get at once 


char* clientPhase2(void){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, sendToAddr_len, addr_len;
	struct sockaddr_in sendToAddr;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	static char buf[MAXBUFLEN];
	int rv, numbytes;
	struct hostent *hostinfo;
	char ipAddr[20];
	struct sockaddr_in local;  // used in getsockname() to get port number
    socklen_t local_len = sizeof(local);
	
	//rebase test3
	hostinfo=gethostbyname("nunki.usc.edu");
    inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr, ipAddr, 20); //get host IP address
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo("localhost", PHASE2PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);;
    }
    // loop through all the results and bind to the first we can. using the codes from Beej's tutorial.
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket.\n");
		exit(1);
    }
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){ // get socket port number
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	printf("Phase2: Client 1 has UDP port number %d and IP address %s.\n", ntohs(local.sin_port), ipAddr);
	
    freeaddrinfo(servinfo);
    
	sendToAddr_len = sizeof sendToAddr;
	sendToAddr.sin_family=AF_INET;
	inet_pton(sendToAddr.sin_family, ipAddr, &(sendToAddr.sin_addr)); //convert readable IP address to sin_addr
	sendToAddr.sin_port=htons(31260);
	
    if ((numbytes = sendto(sockfd, "Client1 doc1", MAXBUFLEN-1 , 0,     //send request to directory server
        (struct sockaddr *)&sendToAddr, sendToAddr_len)) == -1) {
        perror("sendto");
        exit(1);
    }
	printf("Phase2: The File request from Client 1 has been sent to the Directory Server.\n");
	
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {       // receive message from the directory server
        perror("recvfrom");
        exit(1);
	}
	
	printf("Phase2: The File requested by client 1 is presented in %.12s and the File Server's TCP port number is %s.\n",buf, buf+12);
	printf("Phase2: End of phase 2 for Client 1.\n");
    close(sockfd);
	return buf;

}

int clientPhase3(char* wantedPort){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, sendToAddr_len, addr_len;
	//struct sigaction sa;
	struct sockaddr_in sendToAddr;
	int yes=1;
	char s[INET6_ADDRSTRLEN], buf[MAXBUFLEN];
	int rv, numbytes;
	struct hostent *hostinfo;
	char ipAddr[20];
	struct sockaddr_in local;  
    socklen_t local_len = sizeof(local);
	
	hostinfo=gethostbyname("nunki.usc.edu");
    inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr, ipAddr, 20); //get host IP address
	
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("localhost", wantedPort+12, &hints, &servinfo)) != 0) {   //wantedPort+12 is the initial address of TCP port number.
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // connect to the file server through TCP socket
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){ //get socket port number 
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	   	printf("Phase3: Client 1 has dynamic TCP port number %d and IP adress %s.\n",ntohs(local.sin_port), ipAddr);
	
	freeaddrinfo(servinfo); 

	
	if (send(sockfd, "Client1 doc1", 13, 0) == -1){    //send request to file server
		perror("send");
	    exit(1);        
	}
    printf("Phase3: The File request from Client 1 has been sent to the %.12s.\n", wantedPort);
	
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {  //receive the requested doc from file server
	    perror("recv");
	    exit(1);
	}
    
	buf[numbytes] = '\0';
    printf("Phase3: Client 1 received %s from %.12s.\n",buf, wantedPort);
	printf("Phase3: End of Phase 3 for Client1.\n");

	close(sockfd);
	return 0;
}


int main(void)
{
	static char  *wantedPort;
	
	wantedPort=clientPhase2();
	clientPhase3(wantedPort);
	return 0;
}

