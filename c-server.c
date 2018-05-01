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
#define DEBUG


#ifdef DEBUG
#  define D(x) x
#else
#  define D(x) 
#endif



unsigned int nodeid = 1604742636;
unsigned int dest = 2754343401;

json_object *jsetup;
json_object *jtime;

int time_delay = 1000;

    
int socket_desc , sock , clientLen , read_size;

int WIFI_flag , sfds[2] , afd;
struct sockaddr_in server , client;
char client_message[500]={0};
char message[100] = {0};
const char *pMessage = "test";
int makethread = 0;

unsigned int  timer[5];
int recv_Size= 150;



void send_setup(int type);
int SocketRestart(int hSocket);
int BindCreatedSocket(int hSocket);
void error(const char *msg);

short SocketCreate(void)
{
	short hSocket;
	D(printf("Create the socket\n"));
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}

int SocketRestart(int hSocket){
	D(printf("Socket Restart\n"));
    D(printf("shut down work %d", shutdown (sock,2)));
    sleep(5);
	socket_desc = SocketCreate();
	BindCreatedSocket(socket_desc);
	//Listen;
	listen(socket_desc , 3);
	
	// Add sockets to poll list
	//pfds[0].fd = socket_desc;
	//pfds[0].events = POLLIN;
    sleep(1);
	return 1;
}

void error(const char *msg){
    //close(sock);
    perror(msg);
    //exit(0);
	SocketRestart(sock);
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
            D(printf("%s :%d\n", key, temp));
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
            timer[4]= (getTime()- startTime)+ timer[1];
            //time_delay = abs((timer[2]-timer[1])/2 + (timer[2]-timer[3]))/1000;
            time_delay =abs((timer[3]-timer[0]) - (timer[2]-timer[1]));
            D(printf("time delay %d\n", time_delay));
            send_setup(5);
            return;
    }
    //json_object_object_add(jmes,"type", jtype2);
    json_object_object_add(jsend,"dest", jdest);
    json_object_object_add(jsend,"from", jnodeid);
    json_object_object_add(jsend,"msg", jmes);
    json_object_object_add(jsend,"type", jtype1);
    
    //printf ("print json \n");
    D(printf ("The json object created: %s\n",json_object_to_json_string(jsend)));
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
    D(printf ("The json object created: %s\n",json_object_to_json_string(jsetup)));
    if( sendto(sock , json_object_to_json_string(jsetup) , 200, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
    {
        error("WIFI Send failed\n");
    }else{
        D(printf("sent\n"));
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
            printf D(("The json object created: %s\n",json_object_to_json_string(jsend)));
            if( sendto(sock , json_object_to_json_string(jsend) , 200, 0, (const struct sockaddr *) &client, sizeof(client)) < 0)
            {
				printf("Send to failed.\n");
                error("WIFI Send failed\n");
            }else{
                D(printf("sent\n"));
            }
            /*sleep(5);
    }
    makethread = 0;
    pthread_exit(0);*/
}

int recive_message(){
    while(1){
        memset(client_message, '\0', sizeof client_message);
        int left = 0 ;
        int right = 0;
        int overbound = 0;
        if( recv(sock , client_message , recv_Size, 0) < 0)
        {
            printf("WIFI recv failed");
            printf("Here1:\n");
            return -1;
        }
        if (strcmp(client_message, "") == 0){
            return -1;
        }
        D(printf("Client reply : %s\n",client_message));
        for(int i = 0; i < recv_Size; i++){
            switch(client_message[i]){
                case '{':
                    printf("%c %c\n", client_message[i+1], client_message[i+2]);
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
        if (overbound == 1 && left - right != 0){
            D(printf("over bound %d %d %d \n", left, right , recv_Size));
            recv_Size = recv_Size - 12 ;
        }else if (left - right != 0){
            D(printf("to small %d %d %d \n", left, right , recv_Size));
            recv_Size = recv_Size + 12*(left - right);
        }else{
            return 1;
        }
    }
    return 1;
}

int main(int argc , char *argv[])
{
    int state = 0;
    int threads = 1;
	int pollRes;

    pthread_t p[threads];
    
    // poll Struct: Only polling one socket.
    struct pollfd pfds[1];
    
	// Connection, outter loop: Sets up the socket then starts the inner loop. Upon exiting inner
	//	the socket is closed and then restarts the outer loop.
	while(1)
	{
		//Create socket
		socket_desc = SocketCreate();
		if (socket_desc == -1)
		{
			printf("Could not create socket");
			return 1;
		}

		D(printf("Sockets created\n"));
		//Bind
		if( BindCreatedSocket(socket_desc) < 0)
		{
		//print the error message
			perror("bind failed.");
			return 1;
		}

		D(printf("bind done\n"));
		//Listen;
		listen(socket_desc , 3);
		
		// Add sockets to poll list

		
		//accept the connection
		clientLen = sizeof(struct sockaddr_in);
		sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&clientLen);
        
        pfds[0].fd = sock;
		pfds[0].events = POLLIN;
		D(printf("Connection accepted\n"));


		int type;
		json_object *jrecive;
		while(1) { // Communication, inner loop: Connection accepted, continually processes one connection at a time.

			
			// Start of polling case.
			D(printf("Start of polling loop.\n"));
			pollRes = poll(pfds, (unsigned int)MAX_CONN, TIMEOUT);
			switch(pollRes)
			{
				case POLL_EXPIRE:
					printf("Timeout has expired !\n");
					error("TimeOut\n");
					break;
				case POLL_ERR:
					error("Error on poll\n");
                    break;
				default:
					D(printf("Default.\n"));
					if(1 /*pfds[0].revents & POLLIN*/ )
					{
						D(printf("POLLIN & revents 1\n"));
						//Place connection activities here.
                        if(recive_message() < 0){
                            perror("recv\n");
                            break;
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
						printf("Client reply : %s\n",client_message);
						if(client_message[0] != '{'){
								continue;
						}
						jrecive = json_tokener_parse(client_message);
						type = get_type(jrecive);

						// Send some data
						
				//        if(makethread == 0){
				//            for (int i=0; i<threads; i++) {
				//                int index = i;	
				//                int rc = pthread_create(&p[i],NULL,send_broadcast("hello my name is thien"),(void*)index);
				//                assert(rc == 0);
				//            }
				//           makethread = 666;
				//        }

						D(printf("Hello from server, type %d\n", type));
						switch(type){
							case 4:
								D(printf("time\n"));
								send_time(jrecive, 4);
								send_broadcast("hello my name is thien");
								break;
							case 5 :
								D(printf("setup\n"));
								send_setup(6);
								send_broadcast("hello my name is thien");
								break;
							case 6 :
								D(printf("response\n"));
								send_time(jrecive, 6);
								break;
							case 8 :
								D(printf("broadcast message\n"));
								send_broadcast("hello my name is thien");
							/*default:
								printf("broadcase\n");
								send_broadcast("hello my name is thien");
								break;*/
						}
						usleep(time_delay);
					}
			}
			
			//close(sock);
		}
		for (int i=0; i<threads; i++) { 
			int rc = pthread_join(p[i],NULL);
			assert(rc == 0);
		}
		close(sock);
	}
}
