//Jonathan Reynolds
// control.cpp
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

#define MAXDATASIZE 1000//size of buf

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char * argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;

	char address[INET6_ADDRSTRLEN];
	
	

	if (argc != 4) {
        	fprintf(stderr,"usage: server-hostname <TCP Port> <UDP Port>");
        	exit(1);
	}

    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
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

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }


    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 0;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            address, sizeof address);
    printf("client: connecting to %s\n", address);

    freeaddrinfo(servinfo); // all done with this structure
   
    	//send udp socket info to client
	/*char * udp_port = argv[3];
 	if(send(sockfd,udp_port,MAXDATASIZE-1,0) == -1){
        	perror("send");
      	}
	    
      	//receive from server
      	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        	perror("recv");
        	exit(1);
      	}

	//printf("'%s'\n",buf);*/

    while(1){
      //get user input
      fgets(buf,MAXDATASIZE,stdin);

      //send client input
      if(send(sockfd,buf,MAXDATASIZE-1,0) == -1){
        perror("send");
      }
	memset(&buf,0,sizeof buf);
      //receive from server
      if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
      }
      printf("'%s'\n",buf);
      memset(buf,0,sizeof buf);
    }	
    close(sockfd);	
    return 0;
}
