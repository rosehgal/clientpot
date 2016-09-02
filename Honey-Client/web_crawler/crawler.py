#!/usr/bin/env python

import time     
import sys    
import socket
import os
import json
import sys
import file_handler as handler

# This is the first page from where the crawler starts crawling


#reference_seed_page = "http://www.steep.it"
#reference_seed_page = "https://www.dmoz.org"
# reference_seed_page = sys.argv[1]               #"https://www.malwaredomainlist.com" 
#reference_seed_page= "http://www.hosts-file.net" 
#reference_seed_page= "http://www.bellefonte.net"   #Defining the seed URL (should be with a prefix, netloc and scheme and without path)


#Downloading entire Web Document (Raw Page Content)
def download_page(url):
    version = (3,0)
    cur_version = sys.version_info
    if cur_version >= version:                            #If the Current Version of Python is 3.0 or above
        import urllib.request                              #urllib library for Extracting web pages
        opener = urllib.request.FancyURLopener({})
        try:
            open_url = opener.open(url)
            page = str(open_url.read()).replace('\\n', '')
            return page
        except Exception as e:
                print(str(e))
    else:                        #If the Current Version of Python is 2.x
        import urllib2
        try:
            headers = {}
            headers['User-Agent'] = "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.27 Safari/537.17"
            req = urllib2.Request(url, headers = headers)
            response = urllib2.urlopen(req)
            page = response.read()
            return page    
        except:
            return"Page Not found"


#Finding 'Next Link' on a given web page
def get_next_link(s):
    start_link = s.find("href")
    if start_link == -1:    #If no links are found then give an error!
        end_quote = 0
        link = "no_links"
        return link, end_quote
    else:
        start_quote = s.find('"', start_link)
        end_quote = s.find('"',start_quote+1)
        link = str(s[start_quote+1:end_quote])
        return link, end_quote
          

#Getting all links with the help of 'get_next_links'
def get_all_links(page):
    links = []
    while True:
        link, end_link = get_next_link(page)
        if link == "no_links":
            break
        else:
            links.append(link)      #Append all the links in the list named 'Links'
            #time.sleep(0.1)
            page = page[end_link:]
    return links 


#Check for URL extension so crawler does not crawl images and text files
def extension_scan(url):
    a = ['.png','.jpg','.jpeg','.gif','.tif','.txt']
    j = 0
    while j < (len(a)):
        if a[j] in url:
            flag2 = 1
            break
        else:
            flag2 = 0
            j = j+1
    
    return flag2


#URL parsing for incomplete or duplicate URLs

def url_parse(url):
    try:
        from urllib.parse import urlparse
    except ImportError:
        from urlparse import urlparse
    url = url.lower()                                        #Make it lower case
    s = urlparse(url)                                    #parse the given url
    seed_page = reference_seed_page.lower()               #Make it lower case
    t = urlparse(seed_page)                               #parse the seed page (reference page)
    i = 0
    while i<=7:
        if url == "/":
            url = seed_page
            flag = 0  
        elif not s.scheme:
            url = "http://" + url
            flag = 0
        elif "#" in url:
            url = url[:url.find("#")]
        elif "?" in url:
            url = url[:url.find("?")]
        elif s.netloc == "":
            url = seed_page + s.path
            flag = 0
        elif "www" not in url:
            url = "www."[:7] + url[7:]
            flag = 0
            
        elif url[len(url)-1] == "/":
            url = url[:-1]
            flag = 0
        elif s.netloc != t.netloc:
            url = url
            flag = 1
            break        
        else:
            url = url
            flag = 0
            break
        
        i = i+1
        s = urlparse(url)                                #Parse after every loop to update the values of url parameters
    return(url, flag)


     
# t0 = time.time()

#Main Crawl function that calls all the above function and crawls the entire site sequentially

def web_crawl():

    f = open("./newcrawl.dat","a")
    #sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # host=sys.argv[2]                                               #"127.0.0.1"
    # port=sys.argv[3]                                               #8081

    to_crawl = [reference_seed_page]                      #Define list name 'Seed Page'
    crawled=[]                                               #Define list name 'Seed Page'
    
    i=0;                                                         #Initiate Variable to count No. of Iterations
    while to_crawl:                                       #Continue Looping till the 'to_crawl' list is not empty
        urll = to_crawl.pop(0)                                 #If there are elements in to_crawl then pop out the first element
        urll,flag = url_parse(urll)
        flag2 = extension_scan(urll)
        
        #If flag = 1, then the URL is outside the seed domain URL

        if flag == 1 or flag2 == 1:
            pass                                                           #Do Nothing
            
        else:       
            if urll in crawled:                                              #Else check if the URL is already crawled
                pass                                                 #Do Nothing
                                  
            else:                                                        #If the URL is not already crawled, then crawl i and extract all the links from it
                
                to_crawl = to_crawl + get_all_links(download_page(urll))
                crawled.append(urll)
                f.write(urll+"\n");
                
                filename,fileext = os.path.splitext(urll)

                #sock=socket.create_connection((host,port));
                
                fileext = fileext[1:]
                handler.file_handler(urll,fileext)

                #data=json.dumps((urll,fileext))
                #data=json.dumps(("http://www.inkwelleditorial.com/pdfSample.pdf","pdf"))
                #sent = sock.send(data);

                # if sent==0:
                #     print "cannot sent";
		
		# rec = sock.recv(2);
		# rec = rec.decode('utf-8')
		# if rec == "OK" or rec == "ok" or rec == "Ok":
		# 	sock.close();
		# else:
		# 	sock.close();
		# 	print "Not received ok";
		# 	break;


                #Remove duplicated from to_crawl


                n = 1
                j = 0


                while j < (len(to_crawl)-n):
                    if to_crawl[j] in to_crawl[j+1:(len(to_crawl)-1)]:
                        to_crawl.pop(j)
                        n = n+1
                    else:
                        pass                                                                 #Do Nothing
                    j = j+1
            i=i+1

    return to_crawl, crawled






serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    serversocket.bind(("0.0.0.0", 8082))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
print 'Socket bind complete'

serversocket.listen(10);

while 1:

    (conn,addr)=serversocket.accept();

    rec = conn.recv(200)
    reference_seed_page = rec.decode('utf-8')


    crawl_data = web_crawl()
    conn.send("OK")

#t1 = time.time()
#total_time = t1-t0
#print(total_time)
