#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/socket.h>
#include<arpa/inet.h>

#include <string.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>

#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#define filemonitor_id 0


using namespace std;


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

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

void print_error(string err)
{
    cerr<<"Error :{"<<errno<<"} "<<err<<endl;
    exit(EXIT_FAILURE);
}


struct thread_data{
	char path_to_monitor[128];
	int flags;
	int id;
	char *ip;
	int port;
};

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

void * thread_handler(void *data)
{
	thread_data *thread = (thread_data *)data;
	string path_to_monitor(thread->path_to_monitor);

	cout<<"Thread "<<thread->id<<" Listening on "<<path_to_monitor<<endl;
	int fromThread = thread->id;

	///////////////////////////////// monitoring ///////
    int length, i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];
	
	sockaddr_in server_addr;
	
	bzero((void*)&server_addr,sizeof server_addr);

	server_addr.sin_port = htons(thread->port);
	server_addr.sin_family = AF_INET;

	if(inet_pton(AF_INET, thread->ip, &server_addr.sin_addr) <= 0)
		print_error("Unable to Link server IP");

	fd = inotify_init();

	if (fd<0)
		print_error( "inotify_init" );


	wd = inotify_add_watch( fd, path_to_monitor.c_str(),thread->flags);
	
	if(wd<0)
		print_error("Error creating watch");
	
	for(;;)
	{
		int socket_d;

		length = read( fd, buffer, BUF_LEN );  

		if ( length < 0 ) {
			perror( "read" );
		}  

		while ( i < length ) {
			struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
			cout<<"Thread "<<fromThread;
			if ( event->len ) {
				if((socket_d=socket(AF_INET,SOCK_STREAM,IPPROTO_IP)) < 0)
					print_error("Creating Socket");
				
				communication_data msg;
				msg.proc = communication_data::filemonitor;
				msg.pid=-1;
				msg.ppid=-1;
				msg.session_id = (int)getsid(getpid());
				
				cout<<msg.session_id;
				
				string tmp = thread->path_to_monitor;
				
				string dir=" Directory : ";
				string file=" File : ";

				if(connect(socket_d,(sockaddr *)&server_addr,sizeof(server_addr)) < 0)
					print_error("Connect");
				
				if ( event->mask & IN_CREATE ) {
					if ( event->mask & IN_ISDIR ) { 
						printf( "The directory %s was created.\n", event->name ); tmp += dir + event->name;
					}
					else {
						printf( "The file %s was created.\n", event->name ); tmp += file + event->name;
						
					}
				}
				else if ( event->mask & IN_DELETE ) {
					if ( event->mask & IN_ISDIR ) {
						printf( "The directory %s was deleted.\n", event->name ); tmp += dir + event->name;      
					}
					else {
						printf( "The file %s was deleted.\n", event->name ); tmp += file + event->name;
					}
				}
				else if ( event->mask & IN_MODIFY ) {
					if ( event->mask & IN_ISDIR ) {
						printf( "The directory %s was modified.\n", event->name ); tmp += dir + event->name;
					}
					else {
						printf( "The file %s was modified.\n", event->name ); tmp += file + event->name;
					}
				}
				else if ( event->mask & IN_ACCESS ) {
					if ( event->mask & IN_ISDIR ) {
						printf( "The directory %s was Accessed.\n", event->name ); tmp += dir + event->name;
					}
					else {
						printf( "The file %s was Accessed.\n", event->name ); tmp += file + event->name;
					}
				}
				else if ( event->mask & IN_OPEN ) {
					if ( event->mask & IN_ISDIR ) {
						printf( "The directory %s was open.\n", event->name ); tmp += dir + event->name;
					}
					else {
						printf( "The file %s was open.\n", event->name ); tmp += file + event->name;
					}
				}
				else if ( event->mask & IN_ATTRIB ) {
					if ( event->mask & IN_ISDIR ) {
						printf( "The directory %s atrributes were changed.\n", event->name ); tmp += dir + event->name;
					}
					else {
						printf( "The file %s attributes were changed .\n", event->name ); tmp += file + event->name;
					}
				}
				// cout<<tmp<<endl;
				strcpy(msg.data,tmp.c_str());
				write(socket_d,(void *)&msg,sizeof(msg));
				close(socket_d);
			}
			i += EVENT_SIZE + event->len;
		}
		i=0;	
	}
	//////////////////monitoring ends ////////
	return NULL;
}



int main( int argc, char **argv ) 
{
	pthread_t *thread =  new pthread_t[argc];
    thread_data *td =  new thread_data[argc];
	
	for(int i=1;i<argc-2;++i)
	{

		vector<string> args = split(argv[i],',');

		
		if(args.size()<=1 )
		{
			cout<<argv[i]<<" donot contains the monitoring rights\nExiting";
			exit(EXIT_FAILURE);
		}
		
		strcpy(td[i-1].path_to_monitor,args[0].c_str());
		td[i-1].flags=IN_CREATE;
		td[i-1].id=i;
		td[i-1].ip = argv[argc-2];
		td[i-1].port = atoi(argv[argc-1]);

		for(vector<string>::iterator it = args.begin()+1;it<args.end();++it)
			if(*it == "create")
				continue;
			else if(*it == "modify")
				td[i-1].flags |= IN_MODIFY;
			else if(*it == "delete")
				td[i-1].flags |= IN_DELETE;
			else if(*it == "isdir")
				td[i-1].flags |= IN_ISDIR;
		        else if(*it=="access")
		                td[i-1].flags |= IN_ACCESS;
		        else if(*it=="attribute")
		                td[i-1].flags |= IN_ATTRIB;
		        else if(*it=="open")
		                td[i-1].flags |= IN_OPEN;
		        

		
		if(pthread_create(&thread[i-1],NULL,thread_handler,(void*)&td[i-1]))
			cout<<"Unable to create Thread";
	}

	
	for(int i=1;i<argc;++i)
		pthread_join(thread[i-1],NULL);
	
	return 0;
}
