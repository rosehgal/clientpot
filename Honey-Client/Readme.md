1)sudo apt-get install lxc lxc-templates
2)create a lxc guest machine name it ub1
---- 
sudo lxc-create -n ub1 -t ubuntu
Create a lxc machine for ubuntu named ub1
the name must be **ub1** as it is hardcoded in the system
make sure the ip allocated to **lxcbr0** is from the NAT **10.0.3.0/24** else we need to change the system service files
---- 




3)install necessary utils in lxc_guest
----
sudo lxc-start -n ub1 && sudo lxc-console -n ub1
**login**-ubuntu
**passwrod**-ubuntu
sudo apt-get install less gcc g++ make zip tar wget curl lynx
----




4)Copy ./server in current machine on which lxc-host runs
build the server
----
cd ./server
make
cd build
./server $PORT_NUMBER
**This is our logging Server** 
----
For setting up log location, change the value of variable LOG_LOCATION in server.h before make,*Make sure that folder exist*.
you should start logging_server with the default port 8080 else need to change the system service file a bit
**Server need to be started before the request is sent from the cralwer.**




5)Copy the servcices in the service folder to lxc_guest ub1
**filemonitor.service**
        if the defualt of port of logging server is different than 8080, change in the argument given in EXECSTART of this service
        Default watches are addded to /tmp/ and / directory, that can be added with more watches by specifying in EXECSTART : syntax <directory_name>/,<PERMISSIONS_FOR_WATCH>,[<PERMISSIONS_FOR_WATCH>]
        PERMISSIONS_FOR_WATCH includes : ACCESS CREATE DELETE MODIFY ISDIR OPEN
        If server(lxc host belongs to different NAT) change the server IP relapce it with 10.0.3.1
        
**forkmonitor.service**
        if Applicable : Apply changes to server IP and PORT in EXECSTART

**crawler.service**
        invokes crawler service that runs default on port 7070, dont change this

*make sure to give all permissions to service files after copying*
        



6)stop the lxc_guest ub1
        This will be reated as the safe copy, that will be restored after every request



7)In the Host machine. Goto lxc-scripts folder and run
sudo python3 lxc_server.py 7070 *<URL_TO_CRAWL>*
**remember 7070 is harcoded default port for web_crawler service**




8)logs will be created to the log location specified.
Analyse the logs manually.
        
        
        
        
        
        
        
        
        
        
        
        
        

