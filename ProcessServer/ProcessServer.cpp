#include"ProcessServer.hpp"


size_t CProcessServer::CClientSocketForProcServer::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    size_t lastZero = 0;
    for (size_t i = bytesReserved;i < bytesReserved + newBytes;i++)
        if (buf[i] == 0) {
            unsigned bytes = i - lastZero;
            unsigned errorsFound = 0;
            for (unsigned j = lastZero;j < lastZero + bytes;j++)
                if (buf[j] < 32 || buf[j]>126) {
                    buf[j] = '?';
                    errorsFound++;
                }
            Send(&bytes, sizeof(unsigned));
            
            printf("server received a message: \"%.*s\"", (int)bytes, buf + lastZero);
            errorsFound ? printf(" and found %u errors\n", errorsFound) : printf(" with no errors\n");
            
            std::string str(bytes, 0);
            //todo remove duplicate words
            

            lastZero = i + 1;
        }
    return bytesReserved + newBytes - lastZero;
}