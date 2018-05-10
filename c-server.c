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


#include<setjmp.h>
#define TRY do { jmp_buf ex_buf__; switch( setjmp(ex_buf__) ) { case 0: while(1) {
#define CATCH(x) break; case x:
#define FINALLY break; } default: {
#define NTRY break; } } }while(0)
#define THROW(x) longjmp(ex_buf__, x)

#define NO_RESPONSE (1)


#define MAX_CONN (3)
#define TIMEOUT (10000)
#define POLL_EXPIRE (0)
#define POLL_ERR (-1)

//#define DEBUGSocket
#define DEBUGCommunication

#ifdef DEBUGSocket
#  define DSOCKET(x) x
#else
#  define DSOCKET(x) 
#endif

#ifdef DEBUGCommunication
#  define TALK(y) y
#else
#  define TALK(y) 
#endif


unsigned int nodeid = 1604742636;


int time_delay = 1000;


char message[100] = {0};
int makethread = 0;
int spacing = 10;
unsigned int  timer[5] = {0};
int64_t sub[10] = {0};
int sizeSub = 0;

int SocketCreate(void);
int BindCreatedSocket(int hSocket);
void server_node(void);
int send_setup    (int64_t jfrom, int type, json_object * jarray, int * sock, struct sockaddr_in * client);
int send_broadcast(char * message, int * sock, struct sockaddr_in * client);
int send_time     (int64_t jfrom, int type, json_object * jobj, int * sock, struct sockaddr_in * client);
void printMessage(int64_t from, const char* msg);
void error(const char *msg, int *socket_desc, int* sock, int* clientLen,struct sockaddr_in *client);
void SocketRestart(int *socket_desc, int* sock, int* clientLen,struct sockaddr_in *client);
int timeout_recv(int timeoutinseconds, char(* client_message) [1000], int * sock, int recv_Size);
int recive_message(char(* client_message) [1000], int *sock, int *recv_Size);
int checkArray(json_object *jarray);
int checkList(int64_t jfrom);
int send_message(int64_t dest, char * message, int * sock, struct sockaddr_in *client);

int SocketCreate(void)
{
	int socket_desc;
	DSOCKET(printf("Create the socket\n"));
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    int iRetval=-1;
	int ClientPort = 5555;
	struct sockaddr_in  remote={0};
	remote.sin_family = AF_INET; /* Internet address family */
	remote.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	remote.sin_port = htons(ClientPort); /* Local port */
	if( iRetval = bind(socket_desc,(struct sockaddr *)&remote,sizeof(remote))< 0){
        perror("BIND FAIL");
    }
	return socket_desc;
}

void error(const char *msg, int *socket_desc, int* sock, int* clientLen,  struct sockaddr_in *client){
    //close(sock);
    perror(msg);
    //exit(0);
	SocketRestart(socket_desc, sock, clientLen, client);
}

void SocketRestart(int *socket_desc, int* sock, int *clientLen, struct sockaddr_in *client){
    int yes=1;
    if (setsockopt(*socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    memset(client,0,sizeof(client));
    *clientLen = sizeof(struct sockaddr_in);
	DSOCKET(printf("Socket Restart\n"));
    DSOCKET(printf("shut down work %d\n", shutdown(*sock,2)));
    DSOCKET(printf("close work %d\n", close(*sock)));
    sleep(10);

    listen(*socket_desc , 3);
    
    // Add sockets to poll list
    //accept the connection
    
    socklen_t cli_addr_size = sizeof(client);
    *sock = accept(*socket_desc,(struct sockaddr *) client,&cli_addr_size);
    
    //sock = accept(hSocket, (struct sockaddr *)&client, (socklen_t*)&clientLen);
    char *ip = inet_ntoa(client->sin_addr);
    printf("%s\n", ip);
}

unsigned int  getTime(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (unsigned int )(time.tv_nsec / 1000);
}

int checkList(int64_t jfrom){
    if( jfrom == 0){
        return 1;
    }
    //printf("%" PRId64 ", %"PRId64 ", %d\n", jfrom, sub[sizeSub-1], sizeSub);
    for(int i = 0; i < sizeSub; i++){
        if(sub[i] == jfrom){
            return 1;
        }
    }
    //printf("%" PRId64"\n", jfrom);
    sub[sizeSub] = jfrom;
    //printf("%" PRId64", %d\n", sub[sizeSub],sizeSub);
    sizeSub++;
    return -1;
}

int send_time(int64_t jfrom,int type, json_object * jobj, int *sock, struct sockaddr_in *client){
    unsigned int startTime = getTime();
    
    if(type == 4){
        json_object_object_foreach(jobj, key2, val2) {
            if(strcmp(key2,"type")==0){
                timer[0] = json_object_get_int(val2);
            }else if(strcmp(key2,"t0")==0){
                timer[1] = json_object_get_int(val2);
            }else if(strcmp(key2,"t1")==0){
                timer[2] = json_object_get_int(val2);
            }else if(strcmp(key2,"t2")==0){
                timer[3] = json_object_get_int(val2);
            }
        }
        if(timer[0] == 1){
            timer[2] =  (getTime()- startTime)+ timer[1];
        }
    }else if(type == 6){
        timer[0] = 0;
    }
        
    json_object *jsend = json_object_new_object();
    json_object *jdest = json_object_new_int64(jfrom);
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
            timer[4]= (getTime()- startTime)+ timer[1];
            //time_delay = abs((timer[2]-timer[1])/2 + (timer[2]-timer[3]))/1000;
            time_delay =abs((timer[3]-timer[0]) - (timer[2]-timer[1]));
            TALK(printf("time delay %d\n", time_delay));
            //send_time(jfrom,type, jobj, sock, client);
            return 1;
            break;
    }
    //json_object_object_add(jmes,"type", jtype2);
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"type", jtype1);
    json_object_object_add(jsend,"msg", jmes);
    
    //printf ("print json \n");
    TALK(printf ("The json object created: %s\n\n\n %d",json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN),strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN))));
    char * send = malloc(strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)+spacing));
    memset(send,0,strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)+spacing));
    asprintf(&send, "%s\0", json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN));
    if( sendto(*sock , send , strlen(send), 0, (const struct sockaddr *) client, sizeof(*client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent \n"));
        return 1;
    }
    free(send);
}
   
int send_setup(int64_t jfrom, int type,  json_object *jarray, int *sock, struct sockaddr_in *client){
    checkList(jfrom);
    checkArray(jarray);
    json_object *jsetup = json_object_new_object();
    json_object *jdest = json_object_new_int64(jfrom);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(type);
    json_object *jsub = json_object_new_array();
    json_object_object_add(jsetup,"dest", jdest);
    json_object_object_add(jsetup,"from", jnodeid);
    json_object_object_add(jsetup,"type", jtype);
    json_object_object_add(jsetup,"subs", jsub);
    TALK(printf ("\n \n The json object created: %s\n\n %d",json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN),strlen(json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN)) ));
    char * send = malloc(strlen(json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN)+spacing));
    memset(send,0,strlen(json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN)+spacing));
    asprintf(&send, "%s\0", json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN));
    if( sendto(*sock , send,strlen(send)+ spacing, 0, (const struct sockaddr *) client, sizeof(*client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent\n"));
        return 1;
    }
    free(send);
}

int checkArray(json_object *jarray){
    for(int i = 0; i < json_object_array_length(jarray); i++){
        json_object *jobj = json_object_array_get_idx(jarray,i);
        json_object *temp;
        enum json_type jtype = json_object_get_type(jobj);
        switch (jtype){
            case json_type_int:// NodeId
                checkList(json_object_get_int(jobj));
                break;
            case json_type_array:
                temp = json_object_get(jobj);
                if(json_object_array_length(temp) != 0){
                    checkArray(temp);
                }
                break;
        }
    }
}

int send_broadcast(char * message, int * sock, struct sockaddr_in * client){

    /*json_object *jdest = json_object_new_int64(0);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(8);
    json_object *jmes = json_object_new_string(message);
    for( int i =0; i < sizeSub; i++){
        json_object *jsend = json_object_new_object();
        json_object *jdest = json_object_new_int64(0);
        json_object_object_add(jsend,"dest", jdest);
        json_object_object_add(jsend,"from", jnodeid);
        json_object_object_add(jsend,"type", jtype);
        json_object_object_add(jsend,"msg", jmes);
        TALK(printf("The json object created: %s\n",json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)));
        if( sendto(sock , json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN) , strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)), 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
        {
            return -1;
        }else{
            TALK(printf("sent\n"));
            return 1;
        }
    }*/
    send_message(sub[0], message, sock, client);
    usleep(time_delay+10000);
    send_message(0, message, sock, client);
    return 1;
}

int send_message(int64_t dest, char * message, int * sock, struct sockaddr_in * client){
    json_object *jdest = json_object_new_int64(dest);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(8);
    json_object *jmes = json_object_new_string(message);
    json_object *jsend = json_object_new_object();
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"type", jtype);
    json_object_object_add(jsend,"msg", jmes);
    TALK(printf("The json object created: %s\n \n\n %d",json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN),strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN))));
    char * send = malloc(strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)+spacing));
    memset(send,0,strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)+spacing));
    asprintf(&send, "%s\0", json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN));
    if( sendto(*sock , send, strlen(send), 0, (const struct sockaddr *) client, sizeof(*client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent\n"));
        return 1;
    }
    free(send);
}
    
void printMessage(int64_t jfrom, const char* msg){
    printf("from %"PRId64  ": %s",  jfrom, msg);
}

int timeout_recv(int timeoutinseconds, char(* client_message) [1000], int *sock, int recv_Size){
    fd_set socks;
    struct timeval t;
    FD_ZERO(&socks);
    FD_SET(*sock, &socks);
    t.tv_sec = timeoutinseconds;
    t.tv_usec = 0;
    int temp = select(*sock + 1, &socks, NULL, NULL, &t);
    if (temp >0){
	TALK(printf("listen \n"));
        if( recv(*sock , *client_message , recv_Size, 0) == 10060){
            return -1;
        }
        return 1;
    }else{
        printf("time out: FAIL \n");
        return -1;
    }
}

int recive_message(char(* client_message) [1000], int *sock, int *recv_Size){
    while(1){
        memset(*client_message, '\0', sizeof (*client_message));
        int left = 0 ;
        int right = 0;
        int overbound = 0;
        if( timeout_recv(10, client_message, sock, *recv_Size) < 0)
        {
            printf("WIFI recv failed");
            printf("Here1:\n");
            return -1;
        }
        if (strcmp(*client_message, "") == 0){
            return -1;
        }
        TALK(printf("Client reply : %s %d\n",*client_message, *recv_Size));
        for(int i = 0; i < *recv_Size; i++){
            switch((*client_message)[i]){
                case '{':
                    left++;
                    break;
                case '}':
                    right++;
                    break;
            }
            if(left - right == 0 && i  != *recv_Size-1&& left != 0){
                overbound = 1;
            }
        }
        if (overbound == 1 && left - right > 0&& *recv_Size > 150){
            TALK(printf("over bound %d %d %d \n", left, right , *recv_Size));
            *recv_Size = *recv_Size - 12 ;
        }else if (left - right != 0|| *recv_Size < 74){
            TALK(printf("to small %d %d %d \n", left, right , *recv_Size));
            *recv_Size = *recv_Size + 12*abs(left - right);
        }else{
            return 1;
        }
    }
    return 1;
}
void printlist(){
    printf("connections: \n");
    for(int i = 0; i < sizeSub; i++){
        printf("%" PRId64 " \n", sub[i]);
    }
}
void server_node(){
    struct sockaddr_in client;
    int socket_desc , sock , clientLen , read_size;
    char client_message[1000]={0};
    int recv_Size= 150;
    socket_desc = SocketCreate();
    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return;
    }
    listen(socket_desc , 3);
    
    //clientLen = sizeof(struct sockaddr_in);
    //sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&clientLen);
    socklen_t cli_addr_size = sizeof(client);
    sock = accept(socket_desc,(struct sockaddr *) &client,&cli_addr_size);
    char *ip = inet_ntoa(client.sin_addr);
    DSOCKET(printf("%s\n", ip));
    
    DSOCKET(printf("Connection accepted\n"));
    
    while(1) { // Communication, inner loop: Connection accepted, continually processes one connection at a time.
        //Place connection activities here.
        printlist();
        json_object *jrecive;
        if(recive_message(&client_message, &sock,&recv_Size) < 0){
            error("recv\n", &socket_desc, &sock,&clientLen, &client);
        }
        /*if( recv(sock , client_message , recv_Size, 0) < 0)
        {
            printf("WIFI recv failed");
            printf("Here1:\n");
            break;
        }
        if (strcmp(client_message, "") == 0){
            break;
        }*/
        
        // print reply
        TALK(printf("Client reply : %s\n",client_message));
        if(client_message[0] != '{'){
                continue;
        }
        jrecive = json_tokener_parse(client_message);
        
        
        int type = 0 ;
        int64_t  jfrom = 0;
        
        json_object *jarray;
        json_object *jobj;
        const char * jmsg;
        
        json_object_object_foreach(jrecive, key, val) {
            TALK(printf ("The json object created: %s\n",json_object_to_json_string_ext(val,JSON_C_TO_STRING_PLAIN)));
            if(strcmp(key,"from")==0){
                jfrom = json_object_get_int64(val);
                TALK(printf("from" "%" PRId64  "\n", jfrom));
                checkList(json_object_get_int(val));
            }else if(strcmp(key,"type")==0){
                type = json_object_get_int(val);
                TALK(printf("from %d \n", type));
            }else{
                enum json_type jtype;
                jtype = json_object_get_type(val);
                switch (jtype){
                    case json_type_array:
                        jarray = json_object_get(val);
                        break;
                    case json_type_object:
                        jobj = json_object_get(val);
                        break;
                    case json_type_string:
                        jmsg = json_object_get_string(val);
                }
            }
        }

        TALK(printf("Hello from server, from %" PRId64", type %d\n", jfrom, type));
        int pass = 1;
        switch(type){
            case 4:
                TALK(printf("time\n"));
                pass =send_time(jfrom, type, jobj, &sock, &client);
                break;
            case 5 :
                TALK(printf("setup\n"));
                pass = send_setup(jfrom,6,jarray, &sock, &client);
                sleep(5);
                pass = send_setup(0,5,jarray, &sock, &client);
                usleep(time_delay+10000);
                break;
            case 6 :
                TALK(printf("response\n"));
                pass = send_time(jfrom,type, jobj, &sock, &client);
                break;
            case 8 :
                TALK(printf("broadcast message\n"));
                printMessage(jfrom, jmsg);
                break;
            /*default:
                printf("broadcase\n");
                send_broadcast("hello my name is thien");
                break;*/
        }
        if(pass < 0){
            perror("fail send");
        }
        usleep(time_delay+10000);
        send_broadcast("hello my name is thien", &sock, &client);
        usleep(time_delay+10000);
    }
}
int main(int argc , char *argv[])
{
    int state = 0;
    int threads = 1;

    pthread_t p[threads];
    
    server_node();
            // Send some data
//        if(makethread == 0){
//            for (int i=0; i<threads; i++) {
//                int index = i;	
//                int rc = pthread_create(&p[i],NULL,send_broadcast("hello my name is thien"),(void*)index);
//                assert(rc == 0);
//            }
//           makethread = 666;
//        }
    for (int i=0; i<threads; i++) { 
        int rc = pthread_join(p[i],NULL);
        assert(rc == 0);
    }
}