#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/time.h>
#include<poll.h>
#include<sys/types.h>
#include<json.h>
#include <time.h>
#include<pthread.h>
#include<assert.h>
#include<stdlib.h>
#include <unistd.h>
/*
#define MAX_CONN (2)
#define TIMEOUT (1000)
#define POLL_EXPIRE (0)
#define POLL_ERR (-1)
*/

unsigned int nodeid = 1604742636;
unsigned int dest = 2754343401;

json_object *jsetup;
json_object *jtime;

    
int socket_desc , sock , clientLen , read_size;

int WIFI_flag , sfds[2] , afd;
struct sockaddr_in server , client;
char client_message[200]={0};
char message[100] = {0};
const char *pMessage = "test";
int makethread = 0;

unsigned int  timer[4];



void send_setup(int type);

void error(const char *msg){
    close(sock);
    perror(msg);
    exit(0);
}

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


unsigned int  getTime(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (unsigned int )(time.tv_nsec / 1000);
}



int get_type(json_object * jobj) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
        if(json_object_get_type(val) == json_type_int && strcmp(key,"type") == 0){
            int temp = json_object_get_int(val);
            printf("%s :%d\n", key, temp);
            return temp;
        }
    }
    return 0;
}


void send_time(json_object * jobj, int message_type){
    unsigned int startTime = getTime();
    enum json_type type;
    int i = 0;
    json_object * temp;
    unsigned int from = 0;
    json_object_object_foreach(jobj, key1, val1) {
        if(strcmp(key1,"from")==0){
            from = json_object_get_int(val1);
            if(message_type != 4){
                break;
            }
        }
        if(message_type == 4 && strcmp(key1,"msg")==0){
            temp = json_object_get(val1);
            break;
        }
    }
    if(message_type == 4){
        json_object_object_foreach(temp, key2, val2) {
            if(strcmp(key2,"type")==0){
                timer[0] = json_object_get_int(val2);
                if(timer[0]== 2){
                    send_setup(5);
                    return;
                }
            }else if (strcmp(key2,"t0")==0){
                timer[1] = json_object_get_int(val2);
            }
        }
        timer[2] =  (getTime()- startTime)+ timer[1];
    }else if(message_type == 6){
        timer[0] = 0;
    }
    //{"dest":2754343401,"from":1604742636,"msg":{"t0":106451991,"t1":1164986233,"t2":1164986235,"type":2},"type":4} 
        
    json_object *jsend = json_object_new_object();
    json_object *jdest = json_object_new_int64(from);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype1 = json_object_new_int64(4);
    json_object *jmes = json_object_new_object();
    
    json_object *jtype2;
    json_object *jt0;
    json_object *jt1;
    json_object *jt2;
    
    switch(timer[0]){
        case 0:
            jtype2 = json_object_new_int64(1);
            jt0 = json_object_new_int64(getTime());
            json_object_object_add(jmes,"type", jtype2);
            json_object_object_add(jmes,"t0", jt0);
            break;
        case 1:
            jtype2 = json_object_new_int64(2);
            jt0 = json_object_new_int64(timer[1]);
            jt1 = json_object_new_int64(timer[2]);     timer[3] = (getTime()- startTime)+ timer[1];
            jt2 = json_object_new_int64(timer[3]);
            json_object_object_add(jmes,"type", jtype2);
            json_object_object_add(jmes,"t0", jt0);
            json_object_object_add(jmes,"t1", jt1);
            json_object_object_add(jmes,"t2", jt2);
            break;
        case 2:
            return;
    }
    //json_object_object_add(jmes,"type", jtype2);
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"msg", jmes);
    json_object_object_add(jsend,"type", jtype1);
    
    //printf ("print json \n");
    printf ("The json object created: %s\n",json_object_to_json_string(jsend));
    if( sendto(sock , json_object_to_json_string(jsend) , 200, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        error("WIFI Send failed\n");
    }else{
        printf("sent \n");
    }
    
}


    
void send_setup(int type){
    json_object *jsetup = json_object_new_object();
    json_object *jdest = json_object_new_int64(dest);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(type);
    json_object *jsub = json_object_new_array();
    json_object_object_add(jsetup,"dest", jdest);
    json_object_object_add(jsetup,"from", jnodeid);
    json_object_object_add(jsetup,"subs", jsub);
    json_object_object_add(jsetup,"type", jtype);
    printf ("The json object created: %s\n",json_object_to_json_string(jsetup));
    if( sendto(sock , json_object_to_json_string(jsetup) , 200, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        error("WIFI Send failed\n");
    }else{
        printf("sent\n");
    }
}

void send_broadcast(char * message){
    /*struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    unsigned int start = time.tv_sec;
    printf("%d \n", start);
    for(int i = 0; i < 5; i ++){*/

            json_object *jsend = json_object_new_object();
            json_object *jdest = json_object_new_int64(0);
            json_object *jnodeid = json_object_new_int64(nodeid);
            json_object *jtype = json_object_new_int64(8);
            json_object *jmes = json_object_new_string(message);
            json_object_object_add(jsend,"dest", jdest);
            json_object_object_add(jsend,"from", jnodeid);
            json_object_object_add(jsend,"msg", jmes);
            json_object_object_add(jsend,"type", jtype);
            printf ("The json object created: %s\n",json_object_to_json_string(jsend));
            if( sendto(sock , json_object_to_json_string(jsend) , 200, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
            {
                error("WIFI Send failed\n");
            }else{
                printf("sent\n");
            }
            /*sleep(5);

    }
    makethread = 0;
    pthread_exit(0);*/
}

int main(int argc , char *argv[])
{
    int state = 0;
    int threads = 1;

    pthread_t p[threads];
    
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
	//Listen;
	listen(socket_desc , 3);
	
	//accept the connection
	clientLen = sizeof(struct sockaddr_in);
	sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&clientLen);
	printf("Connection accepted\n");


    
	while(1) { // Continually processes one connection at a time.
        memset(client_message, '\0', sizeof client_message);
        memset(message, '\0', sizeof message);
        if( recv(sock , client_message , 150 , 0) < 0)
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
        int type = get_type(jrecive);
        
        // Send some data
        
//        if(makethread == 0){
//            for (int i=0; i<threads; i++) {
//                int index = i;	
//                int rc = pthread_create(&p[i],NULL,send_broadcast("hello my name is thien"),(void*)index);
//                assert(rc == 0);
//            }
//           makethread = 666;
//        }

        printf("Hello from server, type %d\n", type);
        switch(type){
            case 4:
                printf("time\n");
                send_time(jrecive, 4);
                send_broadcast("hello my name is thien");
                break;
            case 5 :
                printf("setup\n");
                send_setup(6);
                send_broadcast("hello my name is thien");
                break;
            case 6 :
                printf("response\n");
                send_time(jrecive, 6);
                break;
            case 8 :
                printf("broadcast message\n");
                send_broadcast("hello my name is thien");
            /*default:
                printf("broadcase\n");
                send_broadcast("hello my name is thien");
                break;*/
        }
        
        //close(sock);
	}
    for (int i=0; i<threads; i++) { 
		int rc = pthread_join(p[i],NULL);
		assert(rc == 0);
    }
    close(sock);
    
}
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
