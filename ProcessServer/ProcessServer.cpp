#include"ProcessServer.hpp"


size_t CProcessServer::CClientSocketForProcServer::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    size_t lastZero = 0;
    for (size_t i = bytesReserved;i < bytesReserved + newBytes;i++)
        if (buf[i] == 0) {
            unsigned bytes = i - lastZero;
            printf("server received a message: %.*s\n", (int)bytes, buf);
            Send(&bytes, sizeof(unsigned));
            lastZero = i + 1;
        }
    return bytesReserved + newBytes - lastZero;
}