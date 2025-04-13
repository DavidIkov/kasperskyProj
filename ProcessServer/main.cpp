#include<iostream>
#include<stdio.h>
#include<cstring>
#include"ProcessServer.hpp"

int main(int argc, char** argv) {

    CProcessServer serv;
    {
        printf("type the port where you want to open the processing server, if you want OS to decide just type 0\n");
        unsigned port; std::cin >> port;
        serv.Open(port);
    }
    printf("opened server on %i port\n", (int)serv.GetLocalPort());

    printf("if you want to exit type exit\n");
    std::string input;
    while (true) {
        std::cin >> input;
        if (input == "exit") break;
    }
    printf("closing server\n");
}