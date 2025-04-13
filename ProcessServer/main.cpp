#include<iostream>
#include<stdio.h>
#include<cstring>
#include"ProcessServer.hpp"

int main(int argc, char** argv) {

    CProcessServer serv;
    serv.Open();
    printf("opened server on %i port\n", (int)serv.GetLocalPort());
    std::this_thread::sleep_for(std::chrono::seconds(20));
    printf("closed\n");
}