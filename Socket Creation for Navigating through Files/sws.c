/* 
Alastair Beaumont V00725310
CSC 361 p1
*/	

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h> 

#define MAXBUFLEN 256

void print_log(struct sockaddr_in cli_addr, char *request_line, char *response_line, char *filename)
{
	/* Time function from http://stackoverflow.com/questions/3673226/c-how-to-print-time-in-format-2009-08-10-181754-811 */
    struct tm* tm_info;
	time_t timer;
    char time_buffer[25];

    time(&timer);
    tm_info = localtime(&timer);
	/* Formats the time properly */
    strftime(time_buffer, 25, "%b %d %H:%M:%S", tm_info);
	
	/* Print Log properly*/
	printf("%s %s:%d %s; %s; %s\n", time_buffer, inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, request_line, response_line, filename); 
}
int bad_request(int sockfd, struct sockaddr_in cli_addr, socklen_t cli_len, char *buffer)
{
	/* Function for checking and printing out Bad Request codes to the log */
	int ret, num_bytes_read;	
	do
	{	
		ret = check_ready(sockfd);
		if(ret != 2)
		{
			return ret;
		}

		cli_len = sizeof(cli_addr);
		if ((num_bytes_read = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0, (struct sockaddr *) &cli_addr, &cli_len)) == -1) 
		{
			perror("sws: error on recvfrom()!");
			return -1;
		}
		/* Reached end of request header */
	} while(buffer[0] != '\n');

	/* Print out Bad Request */
	if (sendto(sockfd, "HTTP/1.0 400 Bad Request\n",26, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
	{
		perror("sws: error in send()");
		return -1;
	}
	if (sendto(sockfd, "\n", 2, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
	{
		perror("sws: error in send()");
		return -1;
	}
	return 0;
}
void init_sockaddr_in(struct sockaddr_in* serv_addr, unsigned short int portno)
{
	memset(serv_addr, 0, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(portno);
}

int create_socket(int portno) 
{
	struct sockaddr_in serv_addr;
	int sockfd;

	//The first step: creating a socket of type of UDP
	//error checking for every function call is necessary!
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("sws: ERROR on socket()");
		return -1;
	}

	/* Initialize socket */
	init_sockaddr_in(&serv_addr, portno); 
 
	/* bind the host address */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
    {
		close(sockfd);
        perror("sws: ERROR on bind()");
        return -1;
    }
	return sockfd;
}

int check_ready(int sockfd) {
	while (1){
        char read_buffer[MAXBUFLEN];
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        int retval = select(sockfd+1, &readfds, NULL, NULL, NULL);
        if(retval <= 0) // error or timeout
		{
			perror("sws: error on select()!");
            return retval;
		}
        else
        {
            if(FD_ISSET(STDIN_FILENO, &readfds) &&
               fgets(read_buffer, MAXBUFLEN, stdin) != NULL &&
               strchr(read_buffer, 'q') != NULL)  // 'q' pressed
			{                
				return 1;
			}
            else if(FD_ISSET(sockfd, &readfds))   // recv buffer ready
			{
                return 2;
			}
        }
    }
}

int handle_requests(int sockfd, char* directory) {
	struct sockaddr_in cli_addr;
	socklen_t cli_len;
	char buffer[MAXBUFLEN];	//data buffer
	char request_line[MAXBUFLEN];
	char response_line[MAXBUFLEN];
	int num_bytes_read, file_location_length, pointer;
	int request_count;
	FILE *requested_file;
	char* full_file_location = (char *) malloc(sizeof(char));
	char* token;
	char* path;
	char* version;

	/* Removes a / so there isn't an extra in the log */
	if (directory[strlen(directory)-1] == '/')
	{
		directory[strlen(directory)-1] = '\0';
	}
	
	/* loop that will increment request_count and then call print_log when the socket has recieved information */
	for (request_count = 0; ; request_count++) {
		if (request_count > 0) {
			print_log(cli_addr, request_line, response_line, full_file_location);
		}
		/* Adding null characters to properly terminate strings */
		request_line[0] = '\0';
		response_line[0] = '\0';
		/* Sets the file location to an empty string */
		strcpy(full_file_location, "");

		int ret = check_ready(sockfd);
		/* Makes sure that the recv buffer is ready */
        if(ret != 2)
		{
			return ret;
		}

		cli_len = sizeof(cli_addr);
		/* Error checking for recieving from the socket */
		if ((num_bytes_read = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0, (struct sockaddr *) &cli_addr, &cli_len)) == -1) 
		{
            perror("sws: error on recvfrom()!");
            return -1;
        }
		/* Adding null characters to end the strings */
		buffer[num_bytes_read] = '\0';
		/* Strip newline */
		strncpy(request_line, buffer, strlen(buffer)-1);
		request_line[strlen(buffer)-1] = '\0';

		/* Figuring out Header */
		if ((token = strtok(buffer," \t")) == NULL)	
		{
			/* Call bad_request to make sure that is no bad request*/
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{			
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");		
			continue;
		}
		/* If method is not either GET or get then return 400*/
		if (strcmp(token, "GET") != 0 && strcmp(token, "get") != 0) {
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{			
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");		
			continue;	
		}
		/* tokenize the buffer based on spaces and tabs */
		if ((token = strtok(NULL, " \t")) == NULL)	
		{			
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");			
			continue;		
		}
		/* Allocate memory for the directory based on the token */
		path = (char *) malloc(sizeof(char) * strlen(token));
		strcpy(path, token);

		/* Check to see if there is actually a / at the start of the directory */
		if(path[0] != '/')
		{
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");			
			continue;
		}
		
		if ( (token = strtok(NULL, " \t")) == NULL)	
		{			
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");			
			continue;	
		}
		version = (char *) malloc(sizeof(char) * strlen(token));
		strcpy(version, token);

		/* takes the next part of the header and makes sure that it is HTTP version 1.0 */
		if (strcmp(version, "HTTP/1.0\n") != 0 && strcmp(token, "http/1.0\n") != 0) 
		{
			if ((ret = bad_request(sockfd, cli_addr, cli_len, buffer)) != 0)
			{
				return ret;
			}
			strcpy(response_line, "HTTP/1.0 400 BAD REQUEST");			
			continue;						
		}		
		
		do
		{	
			ret = check_ready(sockfd);
			if(ret != 2)
			{
				return ret;
			}

			cli_len = sizeof(cli_addr);
			if ((num_bytes_read = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0, (struct sockaddr *) &cli_addr, &cli_len)) == -1) 
			{
				perror("sws: error on recvfrom()!");
				return -1;
			}
			/* Reached end of request header */
		} while(buffer[0] != '\n');	

		/* Check for .. */
		if(path[1] == '.')
		{
			if(path[2] == '.')
			{
				/* 404 ERROR */
				if (sendto(sockfd, "HTTP/1.0 404 Not Found\n", 24, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
				{
					perror("sws: error in send()");
					return -1;
				}
				if (sendto(sockfd, "\n", 2, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
				{
					perror("sws: error in send()");
					return -1;
				}
				strcpy(response_line, "HTTP/1.0 404 NOT FOUND");
				continue;
			}
		}
		/* Adding index if request is just a directory */
		pointer = strlen(path) - 1;
		if( '/' == path[pointer]) 
		{
			path = (char *) realloc(path, (sizeof(path)+sizeof("index.html")+1));
			strcat(path, "index.html");			
		}
		/* Allocating memory for the file location */
		file_location_length = strlen(directory) + strlen(path) + 1;
		full_file_location = (char *) malloc(sizeof(char) * file_location_length);
	
		/*Copying over the directory */
		strcpy(full_file_location, directory);
		strcat(full_file_location, path);

		/* opening the file requested by user*/
		requested_file = fopen(full_file_location, "r");

		if(requested_file == NULL)
		{
			/* 404 ERROR */
			if (sendto(sockfd, "HTTP/1.0 404 Not Found\n", 24, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
			{
				perror("sws: error in send()");
				return -1;
			}
			if (sendto(sockfd, "\n", 2, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
			{
				perror("sws: error in send()");
				return -1;
			}
			strcpy(response_line, "HTTP/1.0 404 NOT FOUND");
			continue;
		}
		/* 200 OK */
		if (sendto(sockfd, "HTTP/1.0 200 OK\n", 17, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
		{
			perror("sws: error in send()");
			return -1;
		}
		if (sendto(sockfd, "\n", 2, 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
		{
			perror("sws: error in send()");
			return -1;
		}
		strcpy(response_line, "HTTP/1.0 200 OK");
		/* Read in from the file then prints that buffer to the screen */
		while(fgets(buffer, MAXBUFLEN, requested_file) != NULL)
		{
			if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &cli_addr, cli_len) == -1)	
			{
				perror("sws: error in send()");
				return -1;
			}
		}
	}
}

/* Using code provided from the lab(sws.c file in resources) */
int main(int argc, char *argv[])
{
	//Using socket() function for communication
	//int socket(int domain, int type, int protocol);
	//domain is PF_INET, type is SOCK_DGRAM for UDP, and protocol can be set to 0 to choose the proper protocol!
	int portno, sockfd;
	char *directory;
	struct sockaddr_in serv_addr, cli_addr;	//we need these structures to store spcket info
	if (argc < 2) {
		printf( "Usage: %s <port> <directory>\n", argv[0]);
		fprintf(stderr,"ERROR, no port provided.\n");
		return -1;;
	}
	if (argc < 3) {
		printf( "Usage: %s <port> <directory>\n", argv[0]);
		fprintf(stderr,"ERROR, no directory provided.\n");
		return -1;;
	}

	setbuf(stdout, NULL);

	portno = atoi(argv[1]);
	directory = argv[2];
	sockfd = create_socket(portno);

	/* Error creating the socket */
	if (sockfd == -1)
	{
		return -1;
	}

	fprintf(stdout, "sws is running on UDP port %d and serving %s\n", portno, directory);
	fprintf(stdout, "press ‘q’ to quit ...\n");

	/* Exit gracefully */
	if (handle_requests(sockfd, directory) != 1)
	{
		return -1;
	}

	close(sockfd);
	exit(0);
}
