#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/time.h>
#include<poll.h>
#include<sys/types.h>

/*
#define MAX_CONN (2)
#define TIMEOUT (1000)
#define POLL_EXPIRE (0)
#define POLL_ERR (-1)
*/
short SocketCreate(void)
{
	short hSocket;
	printf("Create the socket\n");
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}
int BindCreatedSocket(int hSocket)
{
	int iRetval=-1;
	int ClientPort = 5555;
	struct sockaddr_in  remote={0};
	remote.sin_family = AF_INET; /* Internet address family */
	remote.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	remote.sin_port = htons(ClientPort); /* Local port */
	iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
	return iRetval;
}

int main(int argc , char *argv[])
{
	int socket_desc , sock , clientLen , read_size;
	int WIFI_flag , sfds[2] , afd;
	struct sockaddr_in server , client;
	char client_message[200]={0};
	char message[100] = {0};
	const char *pMessage = "test";
	//Create socket
	socket_desc = SocketCreate();
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		return 1;
	}

	printf("Sockets created\n");
	//Bind
	if( BindCreatedSocket(socket_desc) < 0)
	{
	//print the error message
		perror("bind failed.");
		return 1;
	}

	printf("bind done\n");
	//Listen
	listen(socket_desc , 3);
	
	//accept the connection
	clientLen = sizeof(struct sockaddr_in);
	sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&clientLen);
	printf("Connection accepted\n");

	for (int i = 0; i <10; i++) { // Continually processes one connection at a time.
	
	memset(client_message, '\0', sizeof client_message);
	memset(message, '\0', sizeof message);
	if( recv(sock , client_message , 200 , 0) < 0)
	{
		printf("WIFI recv failed");
        break;
	}
    if (strcmp(client_message, "") == 0){
        break;
    }
	// print reply
	printf("Client reply : %s\n",client_message);
	// Send some data
    
    strcpy(message,"Hello from server");
	
	if( sendto(sock , message , strlen(message) , 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
	{
		printf("WIFI Send failed\n");
		break;
	}else{
        printf("sent \n");
    }
    
	//close(sock);
	}
    close(sock);
}
