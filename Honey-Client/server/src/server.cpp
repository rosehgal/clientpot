#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include "server.h"

#include <sys/socket.h>
#include<arpa/inet.h>

#include <semaphore.h>
#include <string.h>
#include <pthread.h>

#define BUFF_LEN 1136
#define THREAD_SHARED 0


using namespace std;

struct communication_data
{
    enum process
    {
        filemonitor,
        procmonitor
    } proc;

    int pid;
    int ppid;
    int session_id;
    char name[100];
    char data[1024];    //optional field for data

};

sem_t mutex1,mutex2;

void print_error(string err)
{
    cerr<<"Error :{"<<errno<<"} "<<err<<endl;
    exit(EXIT_FAILURE);
}

void file_handler(communication_data &msg)
{
    
    sem_wait(&mutex1);
    stringstream ss;
    ss<< LOG_LOCATION<<"file_monitor"<<msg.session_id<<".dat";
    ofstream file;
    file.open(ss.str().c_str(),ios::app);
    string data(msg.data);
    file<<data<<endl;
    file.close();
    sem_post(&mutex1);
}

void proc_handler(communication_data &msg)
{
    sem_wait(&mutex2);
    stringstream ss;
    ss<<LOG_LOCATION<<"proc_session-"<<msg.session_id<<".dat";
    ofstream file;
    file.open(ss.str().c_str(),ios::app | ios::out);
	if(!file.is_open())
		cout<<"Unable to Create file ";
    string name(msg.name);
    file<<msg.pid<<endl<<msg.ppid<<endl<<name<<endl<<"-----------------------"<<endl;
    sem_post(&mutex2);
}

void * process_request(void *data)
{
    cout<<"Received connection \n";
    int client_socket_d = *(int*)data;
    int count=1;
    communication_data msg;
    
    while(count>0)
        count=read(client_socket_d,(void*)&msg,sizeof(msg));

    if(msg.proc == communication_data::procmonitor)
        proc_handler(msg);
    else
        file_handler(msg);
    return NULL;
}

int main(int argc,char **argv)
{
    if(argc==1)
        print_error("port number not specified");

    int port = atoi(argv[1]);
    int server_socket_d,backlog=10;

    sockaddr_in server_addr,client_addr;

    if((server_socket_d=socket(AF_INET,SOCK_STREAM,IPPROTO_IP))==-1)
        print_error("Creating Socket");

    bzero((void*)&server_addr,sizeof(sockaddr_in));    
    bzero((void*)&client_addr,sizeof(sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if(bind(server_socket_d,(sockaddr *)&server_addr,sizeof(server_addr)) < 0)
        print_error("Binding the Socket to the port");
    
    if(listen(server_socket_d,backlog) < 0)
        print_error("Listening for socket");

    socklen_t client_len=sizeof(client_addr);
    
    sem_init(&mutex1,THREAD_SHARED,1);
    sem_init(&mutex2,THREAD_SHARED,1);
    while(true)
    {
        cout<<"Waiting for client to connect\n";
        int client_socket_d = accept(server_socket_d,(sockaddr *)&client_addr,(socklen_t*)&client_len);
        pthread_t processor;

        if(client_socket_d < 0)
            print_error("Unable to Accept");
        
        pthread_create(&processor,NULL,process_request,(void*)&client_socket_d);
        pthread_join(processor,NULL);
    }
}


