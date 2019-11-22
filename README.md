 ## StreamBase

# Use

Just start *Server.exe* first. Then you can start *Client.exe*. Follow the instructions displayed on the client console to do what you want.

# Implementation details

Server and Client use common classes found inside the **Utils** project. 

## Utils

### Streamer

**Streamer** is the name of the custom class created following the requirements of the assigment. A **Streamer** object can be sent between **Server** and **Client** with the use of *serialization*.

### Commands

**Commands** is an enumeration of all the *commands* that can be sent between Server and Client. All messages exchanged between **Server** and **Client** contain a command. A switch specific to each then determines the actions to realize by the receiver.

### MessageWrapper

**MessageWrapper** is the meat of the communication. The **MessageWrapper** class is responsible for the *serialization* and *deserialization* of the mesages. 

All **MessageWrapper** objects have a command attribute whose role is described above. Depending on the nature of the message, one such object also has either a data, streamer or a vector of one of those as an attribute. This is the content of the message when the client chooses to send or receive data/Streamer objects.

## Client

### ClientMain

**ClientMain** is in charge of display and user input. It also creates a unique client instance.

**ClientMain** displays a list of options for the user to choose. Once the user types the number corresponding to its choice a switch determines which command to send, with which content and whether it will be sent *synchronously* or *asynchronously*.

### Client

The **Client** class is in charge of multiple things.
It implements the method to connect to **Server** and disconnects from it.
It has methods to connect either *synchronously* or *asynchronously* and to send messages either *synchronously* or *asynchronously*.
It processes the response it receives from the server to display the results.

It also has methods to check whether the reading and writing processes have been completed in regard to the **NamedPipe** connection. This is useful for asynchronous communication.


## Server

### ServerMain

**ServerMain** mirrors **ClientMain**. It is just here to create a unique instance of **Server**.

### Server

Server is responsible for awaiting connections from clients as well as reading and replying to messages sent by the clients. It also displays the result of the request made by the client and the state of the connection between the two.

The *Initialize* method of Server starts a never ending loop that keeps awaiting a connection from clients.

Once a client conncets to the server it starts a new thread with the *InstanceThread* function. This function calls *GetAnswerToRequest* to process the incoming message.


# How it works

The server and the clients are connected through a NamedPipe.
Multiple clients can be connected to the same server.

If a client wants to send a message to its server it creates a new NamedPipe. If the message is to be sent or received in an asynchronous manner the *ConnectAsync* method will be called. Otherwise the *ConnectSync* method will be called.

Messages content are serialized to ensure the communication between server and clients. **Serialization** is achieved through the use of the Boost library (version 1.71.0).
When a client or the server wants to send a message it first serializes it. Obviously when it receives a message it also deserializes it to access its content. 

A new NamedPipe is created every time the client sends or makes a request to the server.

# Requirements

- **The client should be able to connect to the server through a NamedPipe**\
  The NamedPipes API has been used.

- **The client should be able to make both sync and async calls to the server**\
  This is realized through the *SendSync* and *SendAsync* methods mentioned earlier. 

- **The client should be able to send trivial data (strings, numbers) to the server**\
  You can try either option 3 or 4 of the client options list. Then just try list 7 or 8 to retrive the data from the server.

- **The client should be able to create objects on the server (based on req-7 below), retrieve them, their attributes and call methods on them**\
  As with the data the client options list will tell you what to do. Just try option 1 or 2 to send an **Streamer** object. Use option 5 or 6 to retrieve all the **Streamer** objects.

- **The server should be able to receive both sync/async connection requests from clients**\
  This is achieved by the creation of a new NamedPipe by the client everytime a request is made. A new thread takes care of it on the server side.

- **The server should be able to store data provided by the client via NamedPipe in a reasonable data structure**\
  Vectors have been used to store data. The data can be retrieved through a client by selection option 7 or 8.

- **The server should be able to register a custom class (w/ related functions, attributes) which can be used by the client (see req-4)** and **The server should be able to store the custom objects created by the client for the custom class created in req-7**\
  
  The **Streamer** class is shared by both **Client** and **Server**. A client can create and send a **Streamer** object to the server which will store it. All the objects are stored into a vector and can be retrived through options 5 and 6.

# Failure

I tried to use CMake but It's my first time using it and there was an issue I didn't manage to solve. Hopefully given enough time I'd be able to use CMake.


# Links
https://docs.microsoft.com/pt-br/windows/desktop/ipc/named-pipes
https://www.boost.org/doc/libs/1_71_0/libs/serialization/doc/index.html
https://users.ece.cmu.edu/~eno/coding/CppCodingStandard.html