
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

//read
#include <unistd.h>

#include "common.h"
   enum { MAXLINE = 255 };

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[MAXLINE + 1];

	if (argc < 2) {
		fprintf(stderr, "usage %s hostname [port]\n", argv[0]);
		exit(0);
	} else if (argc >= 3) {
		portno = atoi(argv[2]);
	} else {
		portno = TCP_PORT;
	}
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   /* Now ask for a message from the user, this message
      * will be read by server
   */
	
   printf("Please enter the command: ");
   bzero(buffer,MAXLINE + 1);
   fgets(buffer,MAXLINE + 1,stdin);
  // char response[1024] = "";
	//snprintf(response, sizeof(response),
	//	"  %s, popen error . ", );

	//n = write(sockfd, buffer, strlen(buffer));
	//if (n > 0)
   
   /* Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }

enum {
		n_cmdBuff = 3
	};
	char cmdBuffer[n_cmdBuff + 1] = "";
	char * pcmdBuffer = cmdBuffer;
	int doExit = 0;
	while (!doExit) {
		/* Now read server response */
		bzero(buffer, MAXLINE + 1);
		int n = read(sockfd, buffer, MAXLINE);
		buffer[n] = 0;
		if (n < 0) {
			perror("ERROR reading from socket");
			break;
			//exit(1);
		}
		char * p_null = strchr(buffer, '\0');
		if (p_null) {
			ssize_t nchar_mess = (p_null - buffer);
			if (nchar_mess < n) {

				int ncmdrecv = MIN(n - nchar_mess, n_cmdBuff);
				strncpy(pcmdBuffer, buffer, ncmdrecv);
				pcmdBuffer += ncmdrecv;
				if (ncmdrecv < n_cmdBuff) {
					n = recv(sockfd, pcmdBuffer, n_cmdBuff - ncmdrecv, MSG_WAITALL);
					if (n < 0) {
						perror("ERROR reading from socket");
						break;
						//exit(1);
					}

				}
				if (strcmp(cmdBuffer, END_MARK)) {
					
					doExit = 1;
					break;
				}
			}
		}
		fputs( buffer, stdout);
	}
	if (1 == doExit)
		printf("\ncommand end read,\n");
	return 0;
}