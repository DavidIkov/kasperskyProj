#include<iostream>
#include<stdio.h>
#include<cstring>
#include"Socket.hpp"

int main(int argc, char** argv) {

    std::thread serverThread([]() {
        CServerSocket serv;
        serv.Open();
        CClientSocket client; client.Connect("127.0.0.1", serv.GetLocalPort());
        printf("dddd %i\n", (int)client.GetLocalPort());
        std::this_thread::sleep_for(std::chrono::seconds(8));
        printf("server sleeping done\n");
        });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread clientThread([]() {
        CClientSocket socket;
        {
            std::printf("type the ip to connect, for example 96.7.128.198(example.com)\n");
            std::string serverIP; serverIP = "104.19.222.79";//std::cin >> serverIP;
            std::printf("type the port you want to connect to\n");
            uint16_t serverPort; std::cin >> serverPort;
            socket.Connect(serverIP, serverPort);
        }
        printf("connected!\n");
        socket.SetReadBufferSize(128);
        const char* sendbuf = "GET / HTTP/1.1\r\nHost: whatismyipaddress.com\r\nConnection: close\r\n\r\n";
        socket.Send(sendbuf, strlen(sendbuf));
        std::this_thread::sleep_for(std::chrono::seconds(5));
        printf("client sleeping done\n");
        });
    serverThread.join();
    clientThread.join();
    printf("finish\n");
    /*
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation error\n");
        return 1;
    }
    
    std::printf("type the ip to connect, for example 96.7.128.198(example.com)\n");
    std::string serverIP; std::cin >> serverIP;
    std::printf("type the port you want to connect to\n");
    uint16_t serverPort; std::cin >> serverPort;

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIP.data(), &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        close(sock);
        return 1;
    }
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        close(sock);
        return 1;
    }

    // Send HTTP request
    const char* sendbuf = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    if (send(sock, sendbuf, strlen(sendbuf), 0) < 0) {
        std::cerr << "Send failed\n";
        close(sock);
        return 1;
    }

    // Receive response
    char buffer[1024] = {0};
    int bytesReceived;
    while ((bytesReceived = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << buffer;
    }

    if (bytesReceived < 0) {
        std::cerr << "Read failed\n";
    }

    std::cout << "\nConnection closed\n";

    // Cleanup
    close(sock);
    return 0;
    */
}