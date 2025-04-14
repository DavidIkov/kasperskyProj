#include<iostream>
#include<stdio.h>
#include<cstring>
#include"Client.hpp"
#include"httpparser/include/response.h"
#include"httpparser/include/httpresponseparser.h"

int main(int argc, char** argv) {

    try {
        CClient socket(1024);
        {
            std::printf("type the ip to connect, for example 127.0.0.1\n");
            std::string serverIP; std::cin >> serverIP;
            std::printf("type the port you want to connect to\n");
            uint16_t serverPort; std::cin >> serverPort;
            socket.Connect(serverIP, serverPort);
        }
        printf("connected, type messages that you want to send to server, if you want to exit type exit\n");
        std::string input;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        while (true) {
            getline(std::cin, input);
            if (input == "exit" || !socket.GetIsConnected()) break;
            {
                std::string httpStr = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
                    std::to_string(input.size()) + "\r\n\r\n" + input;
                unsigned len = httpStr.size(); socket.Send(&len, sizeof(unsigned));
                socket.Send(httpStr.c_str(), httpStr.size());
            }
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