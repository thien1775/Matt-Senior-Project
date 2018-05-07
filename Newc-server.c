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

#define DEBUGSocket
#define DEBUGCommunication

#ifdef DEBUGSocket
#  define SOCKET(x) x
#else
#  define SOCKET(x) 
#endif

#ifdef DEBUGCommunication
#  define TALK(x) x
#else
#  define TALK(x) 
#endif


unsigned int nodeid = 2222222222;
unsigned int dest   = 2754343401;

json_object *jsetup;
json_object *jtime;

int time_delay = 1000;


char message[100] = {0};
int makethread = 0;

unsigned int  timer[5];
unsigned int from[10];
int sizeSub = 0;



int send_setup    (unsigned int from, int type, int sock, sockaddr_in client);
int send_broadcast(unsigned int from,char * message, int sock, sockaddr_in client);
int send_time     (unsigned int from, int type, json_object * jobj, int sock, sockaddr_in client);
int BindCreatedSocket(int hSocket);
void error(const char *msg, int socket_desc, int* sock);
void SocketRestart(int socket_des, int* sock);
void printMessage(int from, char* msg);
int timeout_recv(int timeoutinseconds, char* client_message, int sock, int recv_Size);
int recive_message(sockaddr_in client, int sock, int recv_Size);

int SocketCreate(void)
{
	int socket_desc;
	SOCKET(printf("Create the socket\n"));
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
	return  ;
}

void error(const char *msg, int socket_desc, int* sock){
    //close(sock);
    perror(msg);
    //exit(0);
	SocketRestart(socket_desc, sock);
}

void SocketRestart(int socket_des, int* sock){
    int yes=1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    memset(&client,0,sizeof(client));
    clientLen = sizeof(struct sockaddr_in);
	SOCKET(printf("Socket Restart\n"));
    SOCKET(printf("shut down work %d\n", shutdown(sock,2)));
    SOCKET(printf("close work %d\n", close(sock)));
    sleep(10);

    listen(socket_desc , 3);
    
    // Add sockets to poll list
    //accept the connection
    
    socklen_t cli_addr_size = sizeof(client);
    *sock = accept(socket_desc,(struct sockaddr *) &client,&cli_addr_size);
    
    //sock = accept(hSocket, (struct sockaddr *)&client, (socklen_t*)&clientLen);
    char *ip = inet_ntoa(client.sin_addr);
    printf("%s\n", ip);
    
	return 1;
}

unsigned int  getTime(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (unsigned int )(time.tv_nsec / 1000);
}

int checkList(int jfrom){
    
int startParse(json_object * jobj) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
        if(strcmp(key,"from") == 0){
            checkList(json_object_get_int(val));
        }
        if(strcmp(key,"type") == 0){
            int temp = json_object_get_int(val);
            TALK(printf("%s :%d\n", key, temp));
            return temp;
        }
    }
    return 0;
}

int send_time(unsigned int from, int type, json_object * jobj, int sock, sockaddr_in client){
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
            timer[4]= (getTime()- startTime)+ timer[1];
            //time_delay = abs((timer[2]-timer[1])/2 + (timer[2]-timer[3]))/1000;
            time_delay =abs((timer[3]-timer[0]) - (timer[2]-timer[1]));
            TALK(printf("time delay %d\n", time_delay));
            send_setup(5);
            return;
    }
    //json_object_object_add(jmes,"type", jtype2);
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"msg", jmes);
    json_object_object_add(jsend,"type", jtype1);
    
    //printf ("print json \n");
    TALK(printf ("The json object created: %s\n",json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)));
    if( sendto(sock , json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN) , strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)), 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent \n"));
        return 1;
    }
    
}
   
int send_setup(unsigned int from, int type,  int sock, sockaddr_in client){
    json_object *jsetup = json_object_new_object();
    json_object *jdest = json_object_new_int64(from);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(type);
    json_object *jsub = json_object_new_array();
    json_object_object_add(jsetup,"dest", jdest);
    json_object_object_add(jsetup,"from", jnodeid);
    json_object_object_add(jsetup,"subs", jsub);
    json_object_object_add(jsetup,"type", jtype);
    TALK(printf ("The json object created: %s\n",json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN)));
    if( sendto(sock , json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN) ,strlen(json_object_to_json_string_ext(jsetup,JSON_C_TO_STRING_PLAIN)), 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent\n"));
        return 1;
    }
}

int send_broadcast(unsigned int from,char * message, int sock, sockaddr_in client){

    json_object *jsend = json_object_new_object();
    json_object *jdest = json_object_new_int64(0);
    json_object *jnodeid = json_object_new_int64(nodeid);
    json_object *jtype = json_object_new_int64(8);
    json_object *jmes = json_object_new_string(message);
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"msg", jmes);
    json_object_object_add(jsend,"type", jtype);
    TALK(printf("The json object created: %s\n",json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)));
    if( sendto(sock , json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN) , strlen(json_object_to_json_string_ext(jsend,JSON_C_TO_STRING_PLAIN)), 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        return -1;
    }else{
        TALK(printf("sent\n"));
        return 1;
    }
}

void printMessage(int from, char* msg){
    printf("from %d: %s",  from, msg);
}

int timeout_recv(int timeoutinseconds, char* client_message, int sock, int recv_Size){
    fd_set socks;
    struct timeval t;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    t.tv_sec = timeoutinseconds;
    t.tv_usec = 0;
    int temp = select(sock + 1, &socks, NULL, NULL, &t);
    if (temp >0){
	TALK(printf("listen \n"));
        if( recv(sock , client_message , recv_Size, 0) == 10060){
            return -1;
        }
        return 1;
    }else{
        printf("time out: FAIL \n");
        return -1;
    }
}

int recive_message(char* client_message, sockaddr_in client, int sock, int recv_Size){
    while(1){
        memset(client_message, '\0', sizeof client_message);
        int left = 0 ;
        int right = 0;
        int overbound = 0;
        if( timeout_recv(5) < 0)
        {
            printf("WIFI recv failed");
            printf("Here1:\n");
            return -1;
        }
        if (strcmp(client_message, "") == 0){
            return -1;
        }
        TALK(printf("Client reply : %s\n",client_message));
        for(int i = 0; i < recv_Size; i++){
            switch(client_message[i]){
                case '{':
                    left++;
                    break;
                case '}':
                    right++;
                    break;
            }
            if(left - right == 0 && i  != recv_Size-1){
                overbound = 1;
            }
        }
        if (overbound == 1 && left - right > 0&& recv_Size > 150){
            TALK(printf("over bound %d %d %d \n", left, right , recv_Size));
            recv_Size = recv_Size - 12 ;
        }else if (left - right != 0|| recv_Size < 74){
            TALK(printf("to small %d %d %d \n", left, right , recv_Size));
            recv_Size = recv_Size + 12*abs(left - right);
        }else{
            return 1;
        }
    }
    return 1;
}

void server_node(){
    struct sockaddr_in client;
    int socket_desc , sock , clientLen , read_size;
    char client_message[1000]={0};
    char message[100] = {0};
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
    SOCKET(printf("%s\n", ip));
    
    SOCKET(printf("Connection accepted\n"));
    
    while(1) { // Communication, inner loop: Connection accepted, continually processes one connection at a time.
        //Place connection activities here.
        json_object *jrecive;
        if(recive_message() < 0){
            error("recv\n");
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
        
        
        int type;
        unsigned int jfrom;
        json_object *jarray;
        json_object *jobj;
        char * jmsg;
        int i= 0; 
        
        json_object_object_foreach(jrecive, key, val) {
            enum json_type jtype;
            switch(i){
                case 0://dest
                    break;
                case 1://from   
                    if(strcmp(key,"from") == 0){
                        jfrom = json_object_get_int(val);
                        checkList(json_object_get_int(val));
                    }
                    break;
                case 2://type
                    if(strcmp(key,"type") == 0){
                        int type = json_object_get_int(val);
                        TALK(printf("%s :%d\n", key, temp));
                    }
                    break;
                case 3:
                    enum json_type jtype = json_object_get_type(val);
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
                            
                        
            if(strcmp(key,"type") == 0){
                int type = json_object_get_int(val);
                TALK(printf("%s :%d\n", key, temp));
            }
        }

        TALK(printf("Hello from server, type %d\n", type));
        switch(type){
            case 4:
                TALK(printf("time\n"));
                send_time(jfrom, type, jobj, sock, client);
                break;
            case 5 :
                TALK(printf("setup\n"));
                send_setup(6);
                break;
            case 6 :
                TALK(printf("response\n"));
                send_time(jrecive, 6);
                break;
            case 8 :
                TALK(printf("broadcast message\n"));
                printMessage(jrecive);
                break;
            /*default:
                printf("broadcase\n");
                send_broadcast("hello my name is thien");
                break;*/
        }
        usleep(time_delay+10000);
        send_broadcast("hello my name is thien");
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
    close(sock);
}
