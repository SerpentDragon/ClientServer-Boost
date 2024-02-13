## Client-Server APP
This application is a simple client-server chat created using Boost.Asio library.
The client and server applications are asynchronous.

### Functional
This simple application allows you to connect multiple clients in one chat. Clients can 
send messages to each other.  

### Build and Rund
To build this application you may use CMakeLists from this repo. After the project is built, one of the clients has to run the server using
```
./server -p [ --port ] PORT
```
command. You have to specify port this server will run on.

When the server is up, you may connect to it and send mesages. To do it, launch your client app. You have to specify host on which the server is running, the port to connect and your nickname.
```
./client -H [ --host ] HOST -p [ -- port] PORT -n [ --name ] nickname
```

Congratulations! Now you can chat with your friends!

### Allowed command
1) To send message just type it and press Enter;
2) To find out how many client are on-line send "list!" command (without  double-quotes);
3) To exit the chat just send # symbol.
