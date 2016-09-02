import socket as sock
import sys
import os
import json
import lxc
import time
from urllib.parse import urlparse

global ubuntu
ubuntu = lxc.Container("ubuntu")

if not ubuntu.defined:
	os.system("lxc-copy --name ub1 --newname ubuntu")
	ubuntu = lxc.Container("ubuntu")

if not ubuntu.defined:
	raise Exception("Unable to create LXC_COPY of Container\nExiting...\n")
	exit(1);

if not ubuntu.running:
	ubuntu.start()
time.sleep(2)

output=os.popen("lxc-ls -f --filter=ubuntu | grep 10.0 | awk '{ print $5 }'").read()
server_ip = output[:-1]
server_port = int(sys.argv[1])


sock_d = sock.socket(sock.AF_INET,sock.SOCK_STREAM)
server_address = (server_ip,server_port)
print(server_address)
buffer_size = 1024

if not ubuntu.running:
	ubuntu.start()

if ubuntu.running:
	print("LXC - Machine started")



data = sys.argv[2] 
sock_d.connect(server_address)

sock_d.send(data.encode('utf-8'))

data=sock_d.recv(2)
if data.decode('utf-8') != "":
        print("Removing Machine")
        os.system("lxc-stop -n ubuntu")
        os.system("lxc-destroy -n ubuntu")
