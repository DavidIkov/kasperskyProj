this is a assignment project for kaspersky

i used http and json parsing libraries here to make stuff easier(this wasnt contradicting any of the restrictions in document so..)
here i use linux sockets and a TCP
im building with CMake
for building each folder you can do "cmake -B build/" in each folder and then "make" in each build folder
the main folders are Client,ProcessServer,VisualServer, you need to build them

i tried to use a lot of OOP in this project

OOP explanation
base class for most stuff is a ClientSocket and a ServerSocket
they are meant to ACT like Client/Server, and can be used anywhere(i mean you can use client socket on server and etc)
Server class is a abstraction from ServerSocket that is also managing its connected sockets, aka ClientSockets
after that i have abstractions in each folder to suit each ones personal needs
like a Client in Client, ProcessServer in ProcessServer and VisualServer in VisualServer

this is programmed in C++17, and personally i used clang compiler to compile everything,
i dont really have linux but i did this in WSL, usefull thing!