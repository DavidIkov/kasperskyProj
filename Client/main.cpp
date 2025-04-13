#include<iostream>
#include<stdio.h>
#include<cstring>
#include"Client.hpp"

int main(int argc, char** argv) {

    try {
        CClient socket;
        socket.SetReadBufferSize(128);
        {
            std::printf("type the ip to connect, for example 96.7.128.198(example.com)\n");
            std::string serverIP; std::cin >> serverIP;
            std::printf("type the port you want to connect to\n");
            uint16_t serverPort; std::cin >> serverPort;
            socket.Connect(serverIP, serverPort);
        }
        socket.Send("heyy", 5);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        printf("end of client main\n");
    }
    catch (SocketError& err) {
        printf("catched socket exception: %s\n", err.what());
    }
    catch (std::exception& err) {
        printf("catched stl exception: %s\n", err.what());
    }catch (...) {
        printf("catched unknown exception\n");
    }
}