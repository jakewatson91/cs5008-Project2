#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

#define TRUE 1
#define FALSE 0

int main()
{
	const char *port = "65001";		/* available port */
	const char *welcome_msg = "Type 'close' to disconnect; 'shutdown' to stop\n";
	const int hostname_size = 32;
	char hostname[hostname_size];
	char buffer[BUFSIZ];
	const int backlog = 10;			/* also max connections */
	char connection[backlog][hostname_size];	/* storage for IPv4 connections */
	socklen_t address_len = sizeof(struct sockaddr);
	struct addrinfo hints, *server;
	struct sockaddr address;
	int r,max_connect,fd,done;
	fd_set main_fd,read_fd;
	int serverfd,clientfd;

	/* configure the server */
	memset( &hints, 0, sizeof(struct addrinfo));	/* use memset_s() */
	hints.ai_family = AF_INET;			/* IPv4 */
	hints.ai_socktype = SOCK_STREAM;	/* TCP */
	hints.ai_flags = AI_PASSIVE;		/* accept any */
	r = getaddrinfo( 0, port, &hints, &server );
	if( r!=0 )
	{
		perror("failed");
		exit(1);
	}

	/* create a socket */
	serverfd = socket(server->ai_family,server->ai_socktype,server->ai_protocol);
	if( serverfd==-1 )
	{
		perror("failed");
		exit(1);
	}

	/* bind to a port */
	r = bind(serverfd,server->ai_addr,server->ai_addrlen);
	if( r==-1 )
	{
		perror("failed");
		exit(1);
	}

	/* listen for a connection*/
	puts("TCP Server is listening...");
	r = listen(serverfd,backlog);
	if( r==-1 )
	{
		perror("failed");
		exit(1);
	}

	/* deal with multiple connections */
	max_connect = backlog;		/* maximum connections */
	FD_ZERO(&main_fd);			/* initialize file descriptor set */
	FD_SET(serverfd, &main_fd);	/* set the server's file descriptor */
	/* endless loop to process the connections */
	done = FALSE;
	while(!done)
	{
		/* backup the main set into a read set for processing */
		read_fd = main_fd;

		/* scan the connections for any activity */
		r = select(max_connect+1,&read_fd, NULL, NULL, 0);
		if( r==-1 )
		{
			perror("failed");
			exit(1);
		}

		/* process any connections */
		for( fd=1; fd<=max_connect; fd++)
		{
			/* filter only active or new clients */
			if( FD_ISSET(fd,&read_fd) )	/* returns true for any fd waiting */
			{
				/* filter out the server, which indicates a new connection */
				if( fd==serverfd )
				{
					/* add the new client */
					clientfd = accept(serverfd,&address,&address_len);
					if( clientfd==-1 )
					{
						perror("failed");
						exit(1);
					}
					/* connection accepted, get name */
					r = getnameinfo(&address,address_len,hostname,hostname_size,0,0,NI_NUMERICHOST);
					/* update array */
					strcpy(connection[clientfd],hostname);
					printf("New connection from %s\n",connection[clientfd]);

					/* add new client socket to the master list */
					FD_SET(clientfd, &main_fd);

					/* respond to the connection */
					strcpy(buffer,"Hello to ");
					strcat(buffer,connection[clientfd]);
					strcat(buffer,"!\n");
					strcat(buffer,welcome_msg);
					send(clientfd,buffer,strlen(buffer),0);
				} /* end if, add new client */
				/* the current fd has incoming info - deal with it */
				else
				{
					/* read the input buffer for the current fd */
					r = recv(fd,buffer,BUFSIZ,0);
					/* if nothing received, disconnect them */
					if( r<1 )
					{
						/* clear the fd and close the connection */
						FD_CLR(fd, &main_fd);	/* reset in the list */
						close(fd);				/* disconnect */
						/* update the screen */
						printf("%s closed\n",connection[fd]);
					}
					/* something has been received */
					else
					{
						buffer[r] = '\0';		/* terminate the string */
						/* if 'shutdown' sent... */
						if( strcmp(buffer,"shutdown\n")==0 )
							done = TRUE;		/* terminate the loop */
						/* otherwise, echo back the text */
						else
							send(fd,buffer,strlen(buffer),0);
					}
				} /* end else to send/recv from client(s) */
			} /* end if */
		} /* end for loop through connections */
	} /* end while */

	puts("TCP Server shutting down");
	close(serverfd);
	freeaddrinfo(server);
	return(0);
}
