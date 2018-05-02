# Matt-Senior-Project

this will let you see all the program using socket.

netstat -np

do this and it might close the socket

lsof -ti :5555 | xargs --no-run-if-empty kill -9
  
to install json do: sudo apt-get install libjson-c-dev 

run code 

gcc -o server 5c-server.c -I/usr/include/json-c -L/usr/lib -ljson-c -lpthread
