#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <vector>
using namespace std;
typedef struct client{
    char ID[20];
    char pass[20]; 
    int online;
    int fd;    
    char *msg[20];
    int msgs;
} Client;

const int LOGIN        = 0;
const int LOGOUT       = 1;
const int REGISTRATION = 2;
const int KNOCKING     = 3;
const int MESSAGE      = 4;
const int FILED        = 5;
const int CHAT         = 1;
const int PRIVATE      = 0;

const int  SUCCESS     = 0;
const int  WRONG_PASS  = 1;
const int  NO_SUCH_ID  = 2;
const int  ID_EXIST    = 1;
const int  ONLINE      = 0;
const int  OFFLINE     = 1;   

vector<Client> usr;
int num_of_usr = 0;

void Error(const char *msg)
{
    perror(msg);
    exit(1);
}

void Login(int fd, char buf[])
{
    int i, t;
    char ID[20], pass[20], str[256];
    
    sscanf(buf, "%d%s%s", &t, ID, pass);
    for(i = 0; i < num_of_usr; i++){
	if(strcmp(ID, usr.at(i).ID) == 0){
	    if(strcmp(pass, usr.at(i).pass) == 0){
		sprintf(str, "%d %d\n", LOGIN, SUCCESS);
		write(fd, str, 256);
		usr.at(i).online = 1;
		usr.at(i).fd = fd;
		for(int j = 0 ; j < usr.at(i).msgs ; j++){	//offline message
			write(fd, usr.at(i).msg[j], 256);
			free(usr.at(i).msg[j]);
		}
		usr.at(i).msgs = 0;
		return;
	    }
	    else{
		sprintf(str, "%d %d\n", LOGIN, WRONG_PASS);
		write(fd, str, 256);
	        return;
	    }
	}
    }
    sprintf(str, "%d %d\n", LOGIN, NO_SUCH_ID);
    write(fd, str, 256);
    return;
}

void Logout(int fd, char buf[])
{
    int i;
    char str[256];

    for(i = 0; i < num_of_usr; i++){
	if(usr.at(i).fd == fd){
		usr.at(i).online = 0;
		usr.at(i).fd = -1;
	}
    }
    sprintf(str, "%d\n", LOGOUT);
    write(fd, str, 256);
    return;
}

void Registration(int fd, char buf[])
{
    Client p;
    int i, t;
    char ID[20], pass[20], str[256];
    
    sscanf(buf, "%d%s%s", &t, ID, pass);
	if(strcmp(ID, "SYSTEM") == 0){
	    sprintf(str, "%d %d\n", REGISTRATION, ID_EXIST);
	    write(fd, str, 256);
	    return;
	}
    for(i = 0; i < num_of_usr; i++){
	if(strcmp(ID, usr.at(i).ID) == 0){
	    sprintf(str, "%d %d\n", REGISTRATION, ID_EXIST);
	    write(fd, str, 256);
	    return;
	}
    }
    strcpy(p.ID, ID);
    strcpy(p.pass, pass);
    p.online = 1;
    p.fd = fd;
    p.msgs = 0;
    usr.push_back(p);
    num_of_usr++;
    sprintf(str, "%d %d\n", REGISTRATION, SUCCESS);
    write(fd, str, 256);
    return;
}

void Knocking(int fd, char buf[])
{
    int i, t;
    char ID[20], str[256];

    sscanf(buf, "%d%s", &t, ID);

    for(i = 0; i < num_of_usr; i++){
	if(strcmp(usr.at(i).ID, ID) == 0){
	    if(usr.at(i).online == 1){
		sprintf(str, "%d %d %s\n", KNOCKING, ONLINE, ID);
		write(fd, str, 256);
		return;
	    }
	    else{
		sprintf(str, "%d %d %s\n", KNOCKING, OFFLINE, ID);
		write(fd, str, 256);
		return;
	    }
	}
    }
    sprintf(str, "%d %d %s\n", KNOCKING, NO_SUCH_ID, ID);
    write(fd, str, 256);
    return;
}

void Message(int fd, char buf[])
{
    int i, j;
    char c;
    char sender[20], receiver[20], msg[200]; 

    sscanf(buf, "%d%d%s%s%c%[^\n]", &i, &j, sender, receiver, &c, msg);

    if(j == PRIVATE){
	for(i = 0; i < num_of_usr; i++){
	    if(strcmp(usr.at(i).ID, receiver) == 0 && usr.at(i).online == 1){
		write(usr.at(i).fd, buf, 256);
		write(fd, buf, 256);
	    }
		else if(strcmp(usr.at(i).ID, receiver) == 0){	//offline message!
			buf[2] = '2';
			usr.at(i).msg[usr.at(i).msgs] = (char *)malloc(sizeof(char)*256);
			strcpy(usr.at(i).msg[usr.at(i).msgs], buf);
			usr.at(i).msgs++;
			write(fd, buf, 256);
		}
	}
	return;
    }
    else{
	for(i = 0; i < num_of_usr; i++){
	    if(usr.at(i).online == 1) write(usr.at(i).fd, buf, 256);
	}
	return;
    }
}

void File(int fd, char buf[])
{
    int i, j, size, num_of_buf, t, c, remain, x, b;
    char sender[20], receiver[20], file_name[150]; 

    sscanf(buf, "%d%d%s%s%s%d", &t, &c, sender, receiver, file_name, &size);
    if(size % 256 != 0){
	num_of_buf = (size/256) + 1;
	remain = size % 256;
    }
    else{
	num_of_buf = size/256;
	remain = 256;
    }
    if(c == PRIVATE){
	for(i = 0; i < num_of_usr; i++){
	    if(strcmp(usr.at(i).ID, receiver) == 0 && usr.at(i).online == 1){
		write(usr.at(i).fd, buf, 256);
		write(fd, buf, 256);
		for(j = 0; j < num_of_buf; j++){
		    if(j == num_of_buf - 1){
			read(fd, buf, remain);
			write(usr.at(i).fd, buf, remain);
		    }
		    else{
			read(fd, buf, 256);
			write(usr.at(i).fd, buf, 256);
		    }
		}
		return;
	    }
	}
    }
    else{
	for(i = 0; i < num_of_usr; i++){
	    if(usr.at(i).online == 1) write(usr.at(i).fd, buf, 256);
	}
	for(j = 0; j < num_of_buf; j++){
	    if(j == num_of_buf-1) x = remain;
	    else x = 256;
	    b = read(fd, buf, x);
	    printf("read inside FILE %d byte\n", b);
	    for(i = 0; i < num_of_usr; i++){
		if(usr.at(i).online == 1 && usr.at(i).fd != fd) write(usr.at(i).fd, buf, x);
	    }
	}
	return;
    }
}

void* thr_fn(void* arg)
{
    int* ptr = (int*) arg;
    int sockfd = *ptr;
    int i, b, t;
    char buf[256], ID[20], pass[20];
    
    while(1){
	b = read(sockfd, &buf, 256);
	if(b == 0){
	    for(i = 0; i < num_of_usr; i++){
		if(usr.at(i).fd == sockfd){
			usr.at(i).online = 0;
			usr.at(i).fd = -1;
		}
	    }
		printf("fd %d shutdown\n", sockfd);
		close(sockfd);
		pthread_exit(NULL);
	}
	sscanf(buf, "%d", &t);
	printf("read %d bytes\n", b);
	if(t == LOGIN) Login(sockfd, buf);	
	else if(t == LOGOUT){
	    Logout(sockfd, buf);
	}
	else if(t == REGISTRATION) Registration(sockfd, buf);
	else if(t == KNOCKING) Knocking(sockfd, buf);
	else if(t == MESSAGE) Message(sockfd, buf);
	else if(t == FILED) File(sockfd, buf);
	else{
	    printf("%s\n", buf);
	    Error("get message error\n");	    
	}
    }
}

int main(int argv, char *args[])
{
    pthread_t tid[100];
    int num_of_thr = 0;
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, err;
            
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(sockfd < 0) Error("ERROR opening socket");
     
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(args[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) Error("ERROR on binding");
 
    while(1){
	listen(sockfd, 128);
	newsockfd = accept(sockfd, NULL, NULL);
	if(newsockfd < 0) Error("ERROR on accept");

	err = pthread_create(&tid[num_of_thr], NULL, thr_fn, (void*) &newsockfd);
	num_of_thr++;
	sleep(1);
     }
     
     close(sockfd);     
     return 0; 
}


