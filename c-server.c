#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/time.h>
#include<poll.h>
#include<sys/types.h>
#include<json.h>

/*
#define MAX_CONN (2)
#define TIMEOUT (1000)
#define POLL_EXPIRE (0)
#define POLL_ERR (-1)
*/

unsigned int nodeid = 1604742636;
unsigned int dest = 2754343401;
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

int json_parse(json_object * jobj) {
    enum json_type type;
    int temp;
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);
        switch (type) {
            case json_type_int: printf("type: json_type_int, ");
            temp = json_object_get_int(val);
            printf("value: %d\n", temp);
            break;
        }
    }
    return temp;
}

int main(int argc , char *argv[])
{
    //Buffer : {"dest":2754343401,"from":1604742636,"subs":[],"type":6}
    int state = 0;
    json_object *jsetup = json_object_new_object();
    json_object *jdest = json_object_new_int64(dest);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(6);
    json_object *jsub = json_object_new_array();
    json_object_object_add(jsetup,"dest", jdest);
    json_object_object_add(jsetup,"from", jnodeid);
    json_object_object_add(jsetup,"subs", jsub);
    json_object_object_add(jsetup,"type", jtype);
    printf ("The json object created: %s\n",json_object_to_json_string(jsetup));
    //
    //{"dest":0,"from":1604742636,"msg":"{\"nodeId\":1604742636,\"topic\":\"logNode_LISTENER\"}","type":8} 
    json_object *jsend = json_object_new_object();
    json_object *jdest2 = json_object_new_int64(0);
    json_object *jnodeid2 = json_object_new_int64(nodeid);
    json_object *jtype2 = json_object_new_int64(8);
    json_object *jmes = json_object_new_string("hello");
    json_object_object_add(jsend,"dest", jdest2);
    json_object_object_add(jsend,"from", jnodeid2);
    json_object_object_add(jsend,"msg", jmes);
    json_object_object_add(jsend,"type", jtype2);
    printf ("The json object created: %s\n",json_object_to_json_string(jsend));
    
    
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
    json_object *jrecive = json_tokener_parse(client_message);
    int type = json_parse(jrecive);
    printf("type :%d\n", type);
    
	// Send some data
    
    strcpy(message,"Hello from server");
	switch(type){
        case 4:
            if( sendto(sock , json_object_to_json_string(jsetup) , 100, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
        {
            printf("WIFI Send failed\n");
            break;
        }else{
            printf("sent \n");
        }
        case 5 :
        if( sendto(sock , json_object_to_json_string(jsetup) , 100, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
        {
            printf("WIFI Send failed\n");
            break;
        }else{
            printf("sent \n");
        }
        state = 1;
        default:
        if( sendto(sock , json_object_to_json_string(jsend) , 100, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
        {
            printf("WIFI Send failed\n");
            break;
        }else{
            printf("sent \n");
        }
        state = 0;
    }
    
	//close(sock);
	}
    close(sock);
    
}
