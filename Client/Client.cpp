#include"Client.hpp"
#include<cstring>

size_t CClient::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {

    //todo optimize this a little bit
    for (size_t i = bytesReserved;i < bytesReserved + newBytes;i++) {
        if (i % sizeof(unsigned) == 0) {
            unsigned num = *((unsigned*)(buf + i));
            printf("server received %u bytes\n", num);
        }
    }
    return (bytesReserved + newBytes) % sizeof(unsigned);
}