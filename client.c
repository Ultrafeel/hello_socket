
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

//read
#include <unistd.h>
#include <signal.h>

#include "common.h"
   enum { MAXLINE = 255 };
  
sig_atomic_t  do_termination = 0;
void terminator_sig_hndlr(int sn)
{
	printf("\n client: terminator_sig_hndlr : %d.\n", sn);
	do_termination = 1;
}
int main(int argc, char *argv[])
{
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
		system_error(" opening socket");
		exit(1);
	}

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		system_error(" connecting");
		exit(1);
	}

			
	/**/struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	
	sa.sa_handler = &terminator_sig_hndlr;
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = &terminator_sig_hndlr;
	sigaction(SIGINT, &sa, NULL);
	
	
	char const quit_command[] = "quit";
	/* Now ask for a message from the user, this message
	 * will be read by server
	 */

	while (!do_termination) {
		printf("Please enter the command ('%s'; to quit):\n %% ", quit_command);
		bzero(buffer, MAXLINE + 1);
		if (!fgets(buffer, MAXLINE + 1, stdin)) {
			if (errno != EINTR)
				 system_error(" client: inputing command");
			break;
		}
		if (0 == strncmp(buffer, quit_command, sizeof(quit_command) - 1) 
			&& strlen(buffer) == sizeof(quit_command))
			break;
		/* Send message to the server */
		n = write(sockfd, buffer, strlen(buffer));

		if (n < 0) {
			system_error("ERROR writing to socket");
			exit(1);
		}

		enum {
			n_cmdBuff = MARK_SIZE
		};
		char cmdBuffer[n_cmdBuff + 1] = "";
		char * pcmdBuffer = cmdBuffer;
		int rqdone = 0;
		while (!rqdone) {
			/* Now read server response */
			bzero(buffer, MAXLINE + 1);
			int n = read(sockfd, buffer, MAXLINE);
			if (n < 0) {
				if (EINTR != errno) {	
					system_error("  reading from socket");				
					do_termination = 1;
					break;
				}
				if (do_termination)
					break;
				continue;
			} //else if ()
			buffer[n] = 0;	
			char * p_null = strchr(buffer, STOP_CHAR);
			if (p_null) {
				ssize_t nchar_mess = (p_null - buffer);
				if (nchar_mess < n) {

					int ncmdrecv = MIN(n - nchar_mess - 1, n_cmdBuff);
					strncpy(pcmdBuffer, p_null + 1, ncmdrecv);
					pcmdBuffer += ncmdrecv;
					if (ncmdrecv < n_cmdBuff) {
						n = recv(sockfd, pcmdBuffer, n_cmdBuff - ncmdrecv, MSG_WAITALL);
						if (n < 0) {
							system_error("reading from socket.");
							break;
							//exit(1);
						}

					}
					if (0 == strcmp(cmdBuffer, END_MARK)) {
						rqdone = 1;
						break;
					} else	{
						printf(" wrong server response \n");
					}
				}
			}
			fputs(buffer, stdout);
		}
		if (1 == rqdone)
			printf("\n end server response  read.\n");
	}
	return 0;
}