#include<iostream>
#include<stdio.h>
#include<cstring>
#include"Client.hpp"

int main(int argc, char** argv) {

    try {
        CClient socket(128);
        {
            std::printf("type the ip to connect, for example 96.7.128.198(example.com)\n");
            std::string serverIP; std::cin >> serverIP;
            std::printf("type the port you want to connect to\n");
            uint16_t serverPort; std::cin >> serverPort;
            socket.Connect(serverIP, serverPort);
        }
        printf("connected, type messages that you want to send to server, if you want to exit just push enter on empty line\n");
        std::string input;
        while (true) {
            std::cin >> input;
            if (input.size() == 0) break;
            socket.Send(input.c_str(), input.size() + 1);
        }
        printf("exiting reading loop\n");
    }
    catch (SocketError& err) {
        fprintf(stderr, "catched socket exception: %s\n", err.what());
    }
    catch (std::exception& err) {
        fprintf(stderr, "catched stl exception: %s\n", err.what());
    }
    catch (...) {
        fprintf(stderr, "catched unknown exception\n");
    }
}