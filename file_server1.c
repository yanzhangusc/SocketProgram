/*********************************************
Author: Yan Zhang
Email: zhan347@usc.edu
Date: Nov. 2013
This source code includes five functions including main, filePhase1, filePhase3, sigchld_handler
and get_in_addr. This project contains six source file: directory_server.c, file_server1.c, 
file_server2.c, file_server3.c, client1.c, client2.c, client3.c
If you have any questions, please contact me. Thank you.
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PHASE1PORT "22260"   // file_server's phase 1 port
#define PHASE3PORT "41260"  // file_server's phase 3 port
#define MAXBUFLEN 100
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int filePhase1(void){

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, sendToAddr_len;
	struct sigaction sa;
	struct sockaddr_in sendToAddr;
	int yes=1;
	char s[INET6_ADDRSTRLEN], buf[MAXBUFLEN];
	int rv, numbytes;
	struct hostent *hostinfo;
	char ipAddr[20];
	struct sockaddr_in local;  
    socklen_t local_len = sizeof(local);
	
	hostinfo=gethostbyname("nunki.usc.edu");
    inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr, ipAddr, 20); //get file_server's IP
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PHASE1PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { //bind the socket with PHASE1PORT
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {                                             // if all the results are unsatisfactory, print fail to bind
        fprintf(stderr, "listener: failed to bind socket.\n");
        return 2;
    }
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	    printf("Phase1: File Server 1 has UDP port number %d and IP address %s.\n",ntohs(local.sin_port),ipAddr);
	
    freeaddrinfo(servinfo);
    	
    sendToAddr_len = sizeof sendToAddr;
	sendToAddr.sin_family=AF_INET;
	inet_pton(sendToAddr.sin_family, ipAddr, &(sendToAddr.sin_addr)); //convert readable IP address to sin_addr
	sendToAddr.sin_port=htons(21260);
	
    if ((numbytes = sendto(sockfd, "File_Server1 41260", MAXBUFLEN-1 , 0, //send it's own information to directory server
        (struct sockaddr *)&sendToAddr, sendToAddr_len)) == -1) {
        perror("sendto");
        exit(1);
    }
    printf("Phase1: The Registration request from File Server 1 has been sent to the Directory Server.\n");
    printf("Phase1: End of Phase 1 for File Server 1.\n");
    close(sockfd);
	

    return 0;
}

int filePhase3(void){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, sendToAddr_len;
	struct sigaction sa;
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
	

///////////////////////////////////////////TCP PHASE3

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PHASE3PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {       //bind the socket with PHASE3PORT
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {                                                // if all the results are unsatisfactory, print fail to bind
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){  //get the socket's port number and put it in struct "local"
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	printf("Phase3: File Server 1 has TCP port %d and IP address %s.\n",ntohs(local.sin_port),ipAddr);

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {    //deal with zombie processes
		perror("sigaction");
		exit(1);
	}

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //accept connection and creat child socket
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
        
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		
		if (!fork()) { // this is the child process. Reused from Beej's tutorial 6.1
			close(sockfd); // child doesn't need the listener
			
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {   //receive request from the client
	          perror("recv");
	          exit(1);
	        }
	        printf("Phase3: File Server 1 received the request from the %.7s for the file %s.\n", buf,buf+8);
			
			if (send(new_fd, buf+8, 13, 0) == -1)    //send back the requested document
				perror("send");
				printf("Phase3: File Server 1 has sent %s to %.7s.\n", buf+8, buf);
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}


int main(void)
{
	filePhase1();
	filePhase3();
	return 0;
}

