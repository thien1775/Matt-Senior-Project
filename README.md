# Matt-Senior-Project

this will let you see all the program using socket.

netstat -np

do this and it might close the socket

kill PID
  
kill -9 PID
  
sudo lsof -i :5555

  
to install json do: sudo apt-get install libjson-c-dev 


 gcc -o server c-server.c -I/usr/include/json-c -L/usr/lib -ljson-c -lpthread

