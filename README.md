## Client-Server APP
This application is a simple client-server chat created using Boost.Asio library.
The client and server applications are asynchronous.

### Functional
This simple application allows you to connect multiple clients in one chat. Clients can 
send messages to each other.  

### Build and Rund
To build this application you may use CMakeLists from this repo. After the project is build, one of the clients has to run the server using
```
./server -p [ --port ] PORT
```
command. You have to specify port this server will run on.

When the server is up you may connect to it and send mesages. To do it, launch your client app. You have to specify host on which the server is runnng, the port to connect and your nickname.
```
./client -H [ --host ] HOST -p [ -- port] PORT -n [ --name ] nickname
```

Congratulations! Now you can chat with your friends!
