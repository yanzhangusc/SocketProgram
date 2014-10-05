/*********************************************
Author: Yan Zhang
Email: zhan347@usc.edu
Date: Nov. 2013
This source code includes four functions including main, choose_server, directoryPhase1 
and directoryPhase2. This project contains six source file: directory_server.c, file_server1.c, 
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
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define PHASE1PORT "21260"   // directory_server's phase 1 port
#define PHASE2PORT "31260"   // directory_server's phase 3 port
#define MAXBUFLEN 100

char* choose_server(char buf[])
{
	FILE *directory,*resource,*topology;
	char line[3][30];
	static char tcpport[3][30],directory_line[3][30],directory_line2[3][30];
	int resource_flag[3][2],topology_cost[2][3];
	int cmp_cost[2][3];
	static char wanted_port[30];
	int i;
	
	///////////////////////////////////////////////////read directory.txt
	if ((directory=fopen("directory.txt","r"))!=NULL){
	    fscanf(directory,"%s %s %s %s %s %s",directory_line[0],directory_line2[0],directory_line[1],directory_line2[1],directory_line[2],directory_line2[2]);
		for(i=0;i<3;i++){
		    strcat(directory_line[i],directory_line2[i]); //put the content in line i into directory_line[i]
			if(directory_line[i][11]=='1')    //detect whose information each line contains 
                strcpy(tcpport[0],directory_line[i]);
			else if(directory_line[i][11]=='2')
                strcpy(tcpport[1],directory_line[i]);
		    else if(directory_line[i][11]=='3')
                strcpy(tcpport[2],directory_line[i]);
			
		}
	}
	else 
		perror("Unable to open directory.txt!");
		
	///////////////////////////////////////////////////read resource.txt
	if ((resource=fopen("resource.txt","r"))!=NULL){  
	  for(i=0;i<3;i++){
		  if (fgets(line[i],30,resource) == NULL){
		      printf("fgets error rerere\n");
		  }
		  else{
            if(line[i][13]=='2'){                  //detect what documents each file server has and put yes(1) or no(0) in resource_flag
			resource_flag[i][0]=1;
			resource_flag[i][1]=1;
			}
			else if (line[i][13]=='0'){
				resource_flag[i][0]=0;
			    resource_flag[i][1]=0;
			}
			else if (line[i][13]=='1'){
			    if(line[i][18]=='1'){
					resource_flag[i][0]=1;
					resource_flag[i][1]=0;
				}
				else if(line[i][18]=='2'){
					resource_flag[i][0]=0;
					resource_flag[i][1]=1;
				}
			}
		}
      }
		fclose(resource);
	}
	else 
		perror("Unable to open resource.txt!");
	////////////////////////////////////////////////read topology
	if ((topology=fopen("topology.txt","r"))!=NULL){
		 fscanf(topology,"%d %d %d %d %d %d",&topology_cost[0][0],&topology_cost[0][1],&topology_cost[0][2],&topology_cost[1][0],&topology_cost[1][1],&topology_cost[1][2]);
	}
	else 
		perror("Unable to open topology.txt!");
	///////////////////////////////////////////////compare cost
	for(i=0;i<3;i++){
		cmp_cost[0][i]=resource_flag[i][0]*topology_cost[0][i];    //if a server doesn't has a file, the cost will be 0
		cmp_cost[1][i]=resource_flag[i][1]*topology_cost[1][i];
		if(cmp_cost[0][i]==0)
		cmp_cost[0][i]=cmp_cost[0][0]+cmp_cost[0][1]+cmp_cost[0][2]+1;    // set the cost bigger than the sum cost, indicating the fileserver doesn't has the doc 
		if(cmp_cost[1][i]==0)
		cmp_cost[1][i]=cmp_cost[1][0]+cmp_cost[1][1]+cmp_cost[1][2]+1;
	}

	if(buf[11]=='1'){                                                     //get the lowest cost and assign the file server name and its port number to wanted_port
		if(cmp_cost[0][0]<cmp_cost[0][1]&&cmp_cost[0][0]<cmp_cost[0][2])
			strcpy(wanted_port,tcpport[0]);
		if(cmp_cost[0][1]<cmp_cost[0][0]&&cmp_cost[0][1]<cmp_cost[0][2])
			strcpy(wanted_port,tcpport[1]);
		if(cmp_cost[0][2]<cmp_cost[0][1]&&cmp_cost[0][2]<cmp_cost[0][0])
			strcpy(wanted_port,tcpport[2]);
	}
	else if(buf[11]=='2'){
	    if(cmp_cost[1][0]<cmp_cost[1][1]&&cmp_cost[1][0]<cmp_cost[1][2])
			strcpy(wanted_port,tcpport[0]);
		if(cmp_cost[1][1]<cmp_cost[1][0]&&cmp_cost[1][1]<cmp_cost[1][2])
			strcpy(wanted_port,tcpport[1]);
		if(cmp_cost[1][2]<cmp_cost[1][1]&&cmp_cost[1][2]<cmp_cost[1][0])
			strcpy(wanted_port,tcpport[2]);
	}
	
	//printf("wanted port is %s.",wanted_port);
	return wanted_port;
	
}

int directoryPhase1(void){

    int sockfd,i;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[4][MAXBUFLEN];
	char *port;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
	FILE *directory;
	struct hostent *hostinfo;
	char ipAddr[20];
	struct sockaddr_in local;  
    socklen_t local_len = sizeof(local);
	
	hostinfo=gethostbyname("nunki.usc.edu");
    inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr, ipAddr, 20); //get host IP address
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
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
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	printf("Phase1: The directory Server has UDP port number %d and IP address %s\n",ntohs(local.sin_port),ipAddr);
	
    freeaddrinfo(servinfo);
	    
    addr_len = sizeof their_addr;
	
	for(i=1;i<4;i++){
    if ((numbytes = recvfrom(sockfd, buf[i], MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
	
    }
	
	buf[i][numbytes] = '\0';
	printf("Phase1: The Directory Server has received request from %.12s.\n", buf[i]);
	}
	
	directory=fopen("directory.txt","w");
	if(fprintf(directory,"%s\n%s\n%s\n",buf[1],buf[2],buf[3])<0){
	    perror("fprintf error");
		exit(1);
	}
	fclose(directory);
	printf("Phase1: The directory.txt file has been created. End of Phase 1 for the Directory Server.\n");
	
    close(sockfd);
	return 0;
}

int directoryPhase2(void){

    int sockfd,i;
    struct addrinfo hints, *servinfo, *p;
    int numbytes,rv;
    struct sockaddr_storage their_addr;
    char buf[4][MAXBUFLEN];
	static char *port;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
	FILE *directory;
	struct hostent *hostinfo;
	char ipAddr[20];
	struct sockaddr_in local;  
    socklen_t local_len = sizeof(local);
	
	hostinfo=gethostbyname("nunki.usc.edu");
    inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr, ipAddr, 20); //get host IP address
	
	//////////////////////////////////////////////////PHASE2
	
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PHASE2PORT, &hints, &servinfo)) != 0) {
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
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket.\n");
        return 2;
    }
	
	if(getsockname(sockfd,(struct sockaddr *)&local, &local_len)==1){
	    perror("client: getsockname");
	    exit(1);
	}
	else 
	printf("Phase2: The directoy Server has UDP port number %d and IP address %s.\n",ntohs(local.sin_port),ipAddr);
	
    freeaddrinfo(servinfo);
    
    addr_len = sizeof their_addr;
	
	for(i=1;i<3;i++){
      if ((numbytes = recvfrom(sockfd, buf[i], MAXBUFLEN-1 , 0,
          (struct sockaddr *)&their_addr, &addr_len)) == -1) {
          perror("recvfrom");
          exit(1);
	  } 
	  else{
	      port=choose_server(buf[i]);   // call function choose_server to get the TCP port number of the file server with required doc and lowest cost
		  printf("Phase2: The Directory server has received request from %.7s.\n", buf[i]);
	  }  
	  if ((numbytes = sendto(sockfd, port, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, addr_len)) == -1) {
        perror("sendto");
        exit(1);
      }  
	  else
          printf("Phase2: File server details has been sent to %.7s.\n", buf[i]);
	}
	printf("Phase2: End of Phase 2 for the Directory Server.\n");
	close(sockfd);
	return 0;
}



int main(void)
{
    directoryPhase1();
	directoryPhase2();
	return 0;
}

